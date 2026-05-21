#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
PPP 串口桥 + 极简 MQTT（TCP 1883）——用于 PC 侧模拟“4G 模块”，让单片机经 lwIP PPP 连上本地 MQTT。

与 ppp_4g_web_serial_sim.html 控制面行为对齐思路：AT→CONNECT→LCP/IPCP，之后转发 IPv4，
对目的地址 网关:1883 做一连接、处理 MQTT CONNECT / PINGREQ / SUBSCRIBE。

单片机侧请将 MQTT 服务器设为 --gw-ip（默认 10.10.0.1），端口 1883。

依赖：  pip install pyserial

用法示例：
  python ppp_mqtt_serial_bridge.py COM5
  python ppp_mqtt_serial_bridge.py /dev/ttyUSB0 --baud 115200
"""

from __future__ import annotations

import argparse
import os
import random
import struct
import sys
import time

try:
    import serial
except ImportError:
    print("请先安装: pip install pyserial", file=sys.stderr)
    raise SystemExit(1)


def ppp_fcs16(data: bytes) -> int:
    fcs = 0xFFFF
    for b in data:
        x = (fcs ^ b) & 0xFF
        for _ in range(8):
            x = ((x >> 1) ^ 0x8408) if (x & 1) else (x >> 1)
        fcs = ((fcs >> 8) ^ x) & 0xFFFF
    return (~fcs) & 0xFFFF


def ppp_escape(payload: bytes) -> bytes:
    out = bytearray([0x7E])
    for b in payload:
        if b in (0x7E, 0x7D) or b < 0x20:
            out.extend([0x7D, b ^ 0x20])
        else:
            out.append(b)
    out.append(0x7E)
    return bytes(out)


def build_ppp_frame(protocol: int, info: bytes, use_acfc: bool) -> bytes:
    if use_acfc:
        pl = struct.pack(">H", protocol) + info
    else:
        pl = b"\xff\x03" + struct.pack(">H", protocol) + info
    fcs = ppp_fcs16(pl)
    pl += struct.pack("<H", fcs)
    return ppp_escape(pl)


def unescape_ppp(buf: bytes) -> bytes:
    out = bytearray()
    i = 0
    while i < len(buf):
        if buf[i] == 0x7D and i + 1 < len(buf):
            out.append(buf[i + 1] ^ 0x20)
            i += 2
        else:
            out.append(buf[i])
            i += 1
    return bytes(out)


def parse_ppp_body(body: bytes) -> tuple[int, bytes, bool]:
    """返回 (protocol, info, fcs_ok)。"""
    if len(body) < 4:
        return 0, b"", False
    if body[0] == 0xFF and body[1] == 0x03:
        proto = struct.unpack(">H", body[2:4])[0]
        pload = body[4:-2]
    else:
        proto = struct.unpack(">H", body[0:2])[0]
        pload = body[2:-2]
    recv = struct.unpack("<H", body[-2:])[0]
    calc = ppp_fcs16(body[:-2])
    return proto, pload, recv == calc


def lcp_has_acfc(conf_req_options: bytes) -> bool:
    i = 0
    while i + 2 <= len(conf_req_options):
        t, ln = conf_req_options[i], conf_req_options[i + 1]
        if ln < 2:
            break
        if t == 8 and ln == 2:
            return True
        i += ln
    return False


def ipcp_fix_zero_ip_or_ack(info: bytes, assign: bytes) -> tuple[bytes | None, str]:
    """
    处理 IPCP Configure-Request。
    若含 IP 选项且为 0.0.0.0，返回 Configure-Nak 二进制；否则返回 Configure-Ack（逐字节 echo）。
    """
    if len(info) < 4:
        return None, "short"
    code, ident, ln = info[0], info[1], struct.unpack(">H", info[2:4])[0]
    if code != 1 or len(info) < ln:
        return None, "bad-req"
    opts = info[4:ln]
    new_opts = bytearray(opts)
    changed = False
    i = 0
    while i + 2 <= len(new_opts):
        ot, oln = new_opts[i], new_opts[i + 1]
        if oln < 2 or i + oln > len(new_opts):
            break
        if ot == 3 and oln == 6:
            ipb = bytes(new_opts[i + 2 : i + 6])
            if ipb == b"\x00\x00\x00\x00":
                new_opts[i + 2 : i + 6] = assign
                changed = True
        i += oln
    if changed:
        nak_pl = bytes([3, ident]) + struct.pack(">H", 4 + len(new_opts)) + bytes(new_opts)
        return nak_pl, "nak"
    ack_pl = bytes([2, ident]) + struct.pack(">H", ln) + opts
    return ack_pl, "ack"


def ip_checksum(hdr: bytes) -> int:
    if len(hdr) != 20:
        return 0
    h = bytearray(hdr)
    h[10:12] = b"\x00\x00"
    s = 0
    for j in range(0, 20, 2):
        s += struct.unpack(">H", h[j : j + 2])[0]
    while s >> 16:
        s = (s & 0xFFFF) + (s >> 16)
    return (~s) & 0xFFFF


def tcp_checksum(src: bytes, dst: bytes, pseudo_proto: int, segment: bytes) -> int:
    plen = len(segment)
    ps = struct.pack("!4s4sHH", src, dst, pseudo_proto, plen) + segment
    s = 0
    for i in range(0, len(ps), 2):
        if i + 1 < len(ps):
            s += ps[i] << 8 | ps[i + 1]
        else:
            s += ps[i] << 8
    while s >> 16:
        s = (s & 0xFFFF) + (s >> 16)
    return (~s) & 0xFFFF


def build_ipv4_tcp(
    src_ip: bytes,
    dst_ip: bytes,
    sport: int,
    dport: int,
    seq: int,
    ackn: int,
    flags: int,
    payload: bytes,
    ip_id: int,
) -> bytes:
    ihl = 5
    ver = 4
    tlen = ihl * 4 + 20 + len(payload)
    ih = struct.pack(
        "!BBHHHBBH",
        (ver << 4) | ihl,
        0,
        tlen,
        ip_id,
        0,
        64,
        6,
        0,
    ) + src_ip + dst_ip
    cs = ip_checksum(ih)
    ih = ih[:10] + struct.pack(">H", cs) + ih[12:]

    off_res = 0x5000
    th = struct.pack(
        "!HHIIBBHHH",
        sport,
        dport,
        seq & 0xFFFFFFFF,
        ackn & 0xFFFFFFFF,
        off_res >> 8,
        flags,
        8192,
        0,
    )
    tcs = tcp_checksum(src_ip, dst_ip, 6, th + payload)
    th = th[:16] + struct.pack(">H", tcs)

    return ih + th + payload


# --- MQTT 极简 ---

def mqtt_parse_packets(stream: bytearray) -> list[bytes]:
    """从流中取出完整 MQTT 帧（固定头 + Remaining length）。"""
    out = []
    while len(stream) >= 2:
        p = 1
        mult = 1
        rl = 0
        while p < len(stream):
            digit = stream[p]
            rl += (digit & 0x7F) * mult
            mult *= 128
            p += 1
            if (digit & 0x80) == 0:
                break
        else:
            break
        end = p + rl
        if end > len(stream):
            break
        out.append(bytes(stream[:end]))
        del stream[:end]
    return out


def mqtt_skip_remaining_len(pkt: bytes, start: int = 1) -> int:
    p = start
    while p < len(pkt):
        d = pkt[p]
        p += 1
        if (d & 0x80) == 0:
            return p
    return len(pkt)


def mqtt_packet_id(pkt: bytes) -> bytes:
    """MQTT 3.1.1 SUBSCRIBE/UNSUB 等：跳过 Remaining length 后 2 字节报文标识。"""
    p = mqtt_skip_remaining_len(pkt, 1)
    if p + 2 <= len(pkt):
        return pkt[p : p + 2]
    return b"\x00\x00"


def mqtt_handle_frame(pkt: bytes, log) -> bytes | None:
    if not pkt:
        return None
    cmd = pkt[0] >> 4
    if cmd == 1:  # CONNECT
        log("MQTT CONNECT")
        return bytes([0x20, 0x02, 0x00, 0x00])  # CONNACK
    if cmd == 12:  # PINGREQ
        log("MQTT PINGREQ")
        return bytes([0xD0, 0x00])
    if cmd == 8:  # SUBSCRIBE
        mid = mqtt_packet_id(pkt)
        log("MQTT SUBSCRIBE")
        return bytes([0x90, 0x03]) + mid + bytes([0x00])
    if cmd == 3:  # PUBLISH
        log("MQTT PUBLISH (未回显)")
        return None
    log(f"MQTT 其它 cmd={cmd} len={len(pkt)}")
    return None


class TcpMqttSession:
    def __init__(self, log_fn):
        self.state = "CLOSED"
        self.iss = random.randint(1, 0x7FFFFFFF)
        self.irs = 0
        self.snd_nxt = self.iss + 1
        self.rcv_nxt = 0
        self.peer_mss = 0
        self.rx_stream = bytearray()
        self.log = log_fn
        self.waiting_ack_after_synack = False

    def on_packet(self, src_ip: bytes, dst_ip: bytes, seg: bytes, ip_id: list, send_ppp) -> None:
        if len(seg) < 20:
            return
        sport, dport, seq, ack = struct.unpack("!HHII", seg[0:12])
        off = (seg[12] >> 4) * 4
        flg = seg[13]
        payload = seg[off:]

        # MQTT 仅监听 1883
        if dport != 1883:
            return

        syn = bool(flg & 0x02)
        ack_flag = bool(flg & 0x10)
        fin = bool(flg & 0x01)
        rst = bool(flg & 0x04)
        psh = bool(flg & 0x08)

        if rst:
            self.state = "CLOSED"
            self.rx_stream.clear()
            return

        if syn and not ack_flag and self.state == "CLOSED":
            self.state = "SYN_RECV"
            self.irs = seq
            self.rcv_nxt = seq + 1
            self.snd_nxt = self.iss + 1
            ip_id[0] = (ip_id[0] + 1) & 0xFFFF
            synack = build_ipv4_tcp(
                dst_ip,
                src_ip,
                1883,
                sport,
                self.iss,
                self.rcv_nxt,
                0x12,
                b"",
                ip_id[0],
            )
            send_ppp(synack)
            self.waiting_ack_after_synack = True
            self.log(f"TCP SYN -> SYN-ACK (sport={sport})")
            return

        if self.state in ("SYN_RECV", "ESTABLISHED") and ack_flag:
            if self.waiting_ack_after_synack and seq >= self.irs + 1:
                self.waiting_ack_after_synack = False
                self.state = "ESTABLISHED"
                self.log("TCP ESTABLISHED")

        if self.state != "ESTABLISHED":
            return

        if len(payload) > 0 or psh:
            if len(payload) > 0:
                self.rcv_nxt = seq + len(payload)
            self.rx_stream.extend(payload)
            sent_psh = False
            for m in mqtt_parse_packets(self.rx_stream):
                rep = mqtt_handle_frame(m, self.log)
                if rep:
                    ip_id[0] = (ip_id[0] + 1) & 0xFFFF
                    pkt = build_ipv4_tcp(
                        dst_ip,
                        src_ip,
                        1883,
                        sport,
                        self.snd_nxt,
                        self.rcv_nxt,
                        0x18,
                        rep,
                        ip_id[0],
                    )
                    self.snd_nxt += len(rep)
                    send_ppp(pkt)
                    sent_psh = True
            if (len(payload) > 0 or psh) and not sent_psh:
                ip_id[0] = (ip_id[0] + 1) & 0xFFFF
                ack_only = build_ipv4_tcp(
                    dst_ip,
                    src_ip,
                    1883,
                    sport,
                    self.snd_nxt,
                    self.rcv_nxt,
                    0x10,
                    b"",
                    ip_id[0],
                )
                send_ppp(ack_only)

        if fin:
            self.rcv_nxt = seq + 1
            ip_id[0] = (ip_id[0] + 1) & 0xFFFF
            fa = build_ipv4_tcp(
                dst_ip, src_ip, 1883, sport, self.snd_nxt, self.rcv_nxt, 0x11, b"", ip_id[0]
            )
            send_ppp(fa)
            self.state = "CLOSED"
            self.rx_stream.clear()


class Bridge:
    def __init__(self, ser: serial.Serial, gw: str, mcu: str, log=print):
        self.ser = ser
        self.log = log
        self.gw_ip = socket_inet_aton(gw)
        self.mcu_ip = socket_inet_aton(mcu)
        self.ppp_buf = bytearray()
        self.at_line = bytearray()
        self.ppp_mode = False
        self.use_acfc = False
        self.lcp_magic = os.urandom(4)
        self.csq_count = 0
        self.at_rdy = False
        self.ip_id = [random.randint(1, 0xFFFF)]
        self.tcp = TcpMqttSession(lambda m: self.log(f" [MQTT] {m}"))
        self._gw_ipcp_sent = False
        self.ipcp_local_id = 0

    def send_raw(self, b: bytes) -> None:
        self.ser.write(b)

    def send_ppp_ip(self, ip_pkt: bytes) -> None:
        self.send_raw(build_ppp_frame(0x0021, ip_pkt, self.use_acfc))

    def handle_at_line(self, line: str) -> None:
        u = line.strip().upper()
        if not u:
            return
        self.log(f"RX AT: {line!r}")
        if u == "AT":
            if not self.at_rdy:
                self.at_rdy = True
                self.send_raw(b"\r\nRDY\r\n\r\nOK\r\n")
            else:
                self.send_raw(b"\r\nOK\r\n")
            return
        if u == "ATE0":
            self.send_raw(b"\r\nOK\r\n")
            return
        if u.startswith("AT+CSQ"):
            self.csq_count += 1
            if self.csq_count <= 3:
                csq = "99,99"
            else:
                csq = f"{[25, 26, 31][(self.csq_count - 4) % 3]},99"
            self.send_raw(f"\r\n+CSQ: {csq}\r\n\r\nOK\r\n".encode())
            return
        if u in ("AT+CGMR",) or u.startswith("AT+CGDCONT") or u in (
            "AT+CREG?",
            "AT+CPIN?",
            "AT+CESQ",
        ):
            self.send_raw(b"\r\nOK\r\n")
            return
        if u == "ATD*99#":
            self.lcp_magic = os.urandom(4)
            self.ppp_mode = True
            self.use_acfc = False
            self._gw_ipcp_sent = False
            self.ipcp_local_id = 0
            self.tcp = TcpMqttSession(lambda m: self.log(f" [MQTT] {m}"))
            self.send_raw(b"\r\nCONNECT\r\n")
            self.log(f"PPP 会话 Magic={self.lcp_magic.hex(' ')}")
            return
        self.send_raw(b"\r\nOK\r\n")

    def handle_lcp(self, info: bytes) -> bytes | None:
        if len(info) < 1:
            return None
        code = info[0]
        if code == 1:  # Conf-Req
            if len(info) < 4:
                return None
            ln = struct.unpack(">H", info[2:4])[0]
            if len(info) < ln:
                return None
            opts = info[4:ln]
            self.use_acfc = lcp_has_acfc(opts)
            pl = bytes([2, info[1]]) + struct.pack(">H", ln) + opts
            return build_ppp_frame(0xC021, pl, self.use_acfc)
        if code == 9:
            if len(info) < 8:
                return None
            ident, ln = info[1], struct.unpack(">H", info[2:4])[0]
            peer_m = info[4:8]
            body = bytes([0x0A, ident, 0, 12]) + self.lcp_magic + peer_m
            return build_ppp_frame(0xC021, body, self.use_acfc)
        if code == 5:
            ident, ln = info[1], struct.unpack(">H", info[2:4])[0]
            rest = info[4:ln] if len(info) >= ln else b""
            al = 4 + len(rest)
            pl = bytes([6, ident]) + struct.pack(">H", al) + rest
            return build_ppp_frame(0xC021, pl, self.use_acfc)
        return None

    def handle_ppp_info(self, proto: int, info: bytes) -> None:
        if proto == 0xC021:
            r = self.handle_lcp(info)
            if r:
                self.send_raw(r)
            return
        if proto == 0x8021:
            if len(info) < 1:
                return
            code = info[0]
            if code == 2:
                self.log("IPCP Conf-Ack")
                return
            if code == 1:
                pl, kind = ipcp_fix_zero_ip_or_ack(info, self.mcu_ip)
                if pl:
                    if kind == "nak":
                        self.log("IPCP Configure-Nak（分配 MCU IP）")
                    self.send_raw(build_ppp_frame(0x8021, pl, self.use_acfc))
                    if kind == "ack" and not self._gw_ipcp_sent:
                        self.send_ipcp_offer_gw()
                return
        if proto == 0xC023 and len(info) > 0 and info[0] == 1:
            ident = info[1]
            msg = b"Login OK"
            ln = 5 + len(msg)
            pl = bytes([2, ident]) + struct.pack(">H", ln) + bytes([len(msg)]) + msg
            self.send_raw(build_ppp_frame(0xC023, pl, self.use_acfc))
            return
        if proto == 0x0021:
            self.handle_ipv4(info)
            return
        self.log(f"忽略 PPP proto=0x{proto:04x} len={len(info)}")

    def send_ipcp_offer_gw(self) -> None:
        """通告网侧 IP，便于 lwIP 建立指向 PC 的 peer/默认路由。"""
        self._gw_ipcp_sent = True
        self.ipcp_local_id = (self.ipcp_local_id % 255) + 1
        opt = bytes([3, 6]) + self.gw_ip
        tln = 4 + len(opt)
        pl = bytes([1, self.ipcp_local_id]) + struct.pack(">H", tln) + opt
        self.send_raw(build_ppp_frame(0x8021, pl, self.use_acfc))
        self.log("IPCP Configure-Request（提议网侧 " + socket_inet_ntoa(self.gw_ip) + "）")

    def handle_ipv4(self, ip_pkt: bytes) -> None:
        if len(ip_pkt) < 20:
            return
        ver_ihl = ip_pkt[0]
        ihl = (ver_ihl & 0xF) * 4
        if len(ip_pkt) < ihl:
            return
        proto = ip_pkt[9]
        src = ip_pkt[12:16]
        dst = ip_pkt[16:20]
        if proto != 6:
            return
        if dst != self.gw_ip:
            return
        self.tcp.on_packet(
            src,
            dst,
            ip_pkt[ihl:],
            self.ip_id,
            self.send_ppp_ip,
        )

    def feed_byte(self, b: int) -> None:
        if not self.ppp_mode:
            if b in (0x0D, 0x0A):
                if self.at_line:
                    try:
                        self.handle_at_line(self.at_line.decode("utf-8", errors="replace"))
                    except Exception as e:
                        self.log(f"AT 错误: {e}")
                    self.at_line.clear()
            else:
                if len(self.at_line) < 512:
                    self.at_line.append(b)
            return

        if b == 0x7E:
            if len(self.ppp_buf) > 1:
                raw = unescape_ppp(bytes(self.ppp_buf))
                self.ppp_buf.clear()
                if len(raw) < 4:
                    return
                proto, info, ok = parse_ppp_body(raw)
                if ok:
                    self.handle_ppp_info(proto, info)
                else:
                    self.log("PPP FCS 错误，丢弃")
            else:
                self.ppp_buf.clear()
        else:
            self.ppp_buf.append(b)
            if len(self.ppp_buf) > 6000:
                self.log("PPP 缓冲溢出")
                self.ppp_buf.clear()

    def run(self) -> None:
        self.log("桥接运行中，Ctrl+C 结束。MQTT 请连 %s:1883" % socket_inet_ntoa(self.gw_ip))
        while True:
            chunk = self.ser.read(4096)
            if chunk:
                for x in chunk:
                    self.feed_byte(x)
            else:
                time.sleep(0.002)


def socket_inet_aton(ip: str) -> bytes:
    return bytes(int(x) for x in ip.split("."))


def socket_inet_ntoa(raw: bytes) -> str:
    return ".".join(str(x) for x in raw)


def main() -> None:
    ap = argparse.ArgumentParser(description="PPP + MQTT(1883) 串口桥")
    ap.add_argument("port", help="串口名，如 COM3 或 /dev/ttyUSB0")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--gw-ip", default="10.10.0.1", help="模拟网关，也是 MQTT 地址")
    ap.add_argument("--mcu-ip", default="10.10.0.2", help="协商给单片机的地址")
    args = ap.parse_args()

    ser = serial.Serial(args.port, args.baud, timeout=0.05)
    br = Bridge(ser, args.gw_ip, args.mcu_ip, log=lambda *a, **k: print(f"[{time.strftime('%H:%M:%S')}]", *a, **k))
    try:
        br.run()
    except KeyboardInterrupt:
        print("退出")
    finally:
        ser.close()


if __name__ == "__main__":
    main()
