#ifndef APP_BRINGUP_DIAG_H
#define APP_BRINGUP_DIAG_H

#ifndef APP_BRINGUP_DIAG
#define APP_BRINGUP_DIAG 0
#endif

/*
 * Bring-up defaults:
 * - Run the original C startup path instead of the LED-only diagnostic loop.
 * - Keep risky outputs and auto traffic disabled until each subsystem is tested.
 */
#ifndef APP_BRINGUP_ENABLE_WATCHDOG
#define APP_BRINGUP_ENABLE_WATCHDOG 1
#endif

#ifndef APP_BRINGUP_ENABLE_BEEP_RELAY
#define APP_BRINGUP_ENABLE_BEEP_RELAY 0
#endif

#ifndef APP_BRINGUP_ENABLE_LUATOS
#define APP_BRINGUP_ENABLE_LUATOS 1
#endif

#ifndef APP_BRINGUP_RUN_SDCARD_LUA
#define APP_BRINGUP_RUN_SDCARD_LUA 1
#endif

#ifndef APP_BRINGUP_AUTO_POWERBUS
#define APP_BRINGUP_AUTO_POWERBUS 0
#endif

#ifndef APP_BRINGUP_ENABLE_AUTO_REBOOT
#define APP_BRINGUP_ENABLE_AUTO_REBOOT 0
#endif

/*
 * Minimal mesh bring-up mode.
 * Keep USB/VCOM shell, LuatOS, SD-card access, mesh.lua and LoRa mesh alive.
 * Skip unrelated GPS, PPP, web terminal, Ethernet and delayed business scripts
 * while debugging z_light_param -> mesh -> LoRa lamp control.
 */
#ifndef APP_BRINGUP_MIN_MESH_MODE
#define APP_BRINGUP_MIN_MESH_MODE 1
#endif

/*
 * Add W5500 Ethernet plus the minimal UDP light-control Lua scripts while
 * keeping GPS, PPP, MQTT and the rest of the business scripts disabled.
 */
#ifndef APP_BRINGUP_ENABLE_ETH_UDP
#define APP_BRINGUP_ENABLE_ETH_UDP 1
#endif

/*
 * Use USART10 (PE2/PE3) as an external light-control command port.
 * This occupies the UART that was originally used by the 4G PPP module.
 */
#ifndef APP_ENABLE_UART10_LIGHT_CMD
#define APP_ENABLE_UART10_LIGHT_CMD 1
#endif

/*
 * Temporary maintenance mode for updating SD-card scripts over the console.
 * It keeps SD and MSH alive, but skips noisy business services that can
 * corrupt or block YMODEM transfers on the same UART.
 */
#ifndef APP_SCRIPT_UPDATE_MODE
#define APP_SCRIPT_UPDATE_MODE 0
#endif

#if APP_SCRIPT_UPDATE_MODE
#undef APP_BRINGUP_ENABLE_LUATOS
#define APP_BRINGUP_ENABLE_LUATOS 0

#undef APP_BRINGUP_RUN_SDCARD_LUA
#define APP_BRINGUP_RUN_SDCARD_LUA 0

#undef APP_BRINGUP_AUTO_POWERBUS
#define APP_BRINGUP_AUTO_POWERBUS 0

#undef APP_BRINGUP_ENABLE_AUTO_REBOOT
#define APP_BRINGUP_ENABLE_AUTO_REBOOT 0
#endif

#endif
