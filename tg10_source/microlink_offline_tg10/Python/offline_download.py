import PikaStdLib
import time
import cmd
import load

# TG10 MKLINK V3/V4 offline download script.
# Execute path on MICROKEEN disk: Python/offline_download.py
#
# Required files on MICROKEEN disk:
#   Python/offline_download.py
#   bootloader.bin
#   rt-thread_full.bin
#   FLM/STM32H7B0_Flash.FLM
#   FLM/ART-Pi_W25Q64.FLM

AUTO_DOWNLOAD_COUNT = 1
WAIT_IDCODE_TIMEOUT = 10000
SWD_CLOCK_HZ = 1000000
SKIP_BOOT_TEST_APP_ONLY = 1

# Internal flash bootloader
BOOT_FLM_FILE_PATH = "FLM/STM32H7B0_Flash.FLM"
BOOT_FLM_FLASH_BASE = 0x08000000
BOOT_FLM_RAM_BASE = 0x20000000
BOOT_BIN_FILE_PATH = "bootloader_pad32.bin"
BOOT_BIN_FILE_ADD = 0x08000000

# External OSPI W25Q64 app
APP_FLM_FILE_PATH = "FLM/ART-Pi_W25Q64.FLM"
APP_FLM_FLASH_BASE = 0x90000000
APP_FLM_RAM_BASE = 0x20000000
APP_BIN_FILE_PATH = "rt-thread_full.bin"
APP_BIN_FILE_ADD = 0x90000000

cmd.set_swd_clock(SWD_CLOCK_HZ)

abort = False
for i in range(AUTO_DOWNLOAD_COUNT):
    if abort:
        break

    print("=== TG10 Auto Download Round:", i + 1, "===")

    elapsed = 0
    while True:
        idcode = cmd.get_idcode()
        if idcode not in (0, 0xFFFFFFFF):
            break
        if elapsed >= WAIT_IDCODE_TIMEOUT:
            print("wait idcode online timeout")
            abort = True
            break
        print("=== waited_ms :", elapsed, "===")
        time.sleep_ms(500)
        elapsed += 500

    if abort:
        break

    print("IDCODE: 0x%08X" % idcode)

    if SKIP_BOOT_TEST_APP_ONLY == 0:
        print("load boot flm")
        if load.flm(BOOT_FLM_FILE_PATH, BOOT_FLM_FLASH_BASE, BOOT_FLM_RAM_BASE) != 0:
            print("load boot flm failed")
            abort = True
            break

        print("erase boot flash")
        if cmd.erase_chip_flash(BOOT_FLM_FLASH_BASE) != 0:
            print("erase boot flash failed")
            abort = True
            break

        print("load boot bin")
        if load.bin(BOOT_BIN_FILE_PATH, BOOT_BIN_FILE_ADD) != 0:
            print("load boot bin failed")
            abort = True
            break
    else:
        print("skip boot flash, test app only")

    print("load app flm")
    if load.flm(APP_FLM_FILE_PATH, APP_FLM_FLASH_BASE, APP_FLM_RAM_BASE) != 0:
        print("load app flm failed")
        abort = True
        break

    print("erase app flash")
    if cmd.erase_chip_flash(APP_FLM_FLASH_BASE) != 0:
        print("erase app flash failed")
        abort = True
        break

    print("load app bin")
    if load.bin(APP_BIN_FILE_PATH, APP_BIN_FILE_ADD) != 0:
        print("load app bin failed")
        abort = True
        break

    if AUTO_DOWNLOAD_COUNT == 1:
        break

    elapsed = 0
    while True:
        idcode = cmd.get_idcode()
        if idcode in (0, 0xFFFFFFFF):
            break
        if elapsed >= WAIT_IDCODE_TIMEOUT:
            print("wait idcode offline timeout")
            abort = True
            break
        time.sleep_ms(500)
        elapsed += 500

if not abort:
    print("tg10 auto download finished")
else:
    print("tg10 auto download aborted")
