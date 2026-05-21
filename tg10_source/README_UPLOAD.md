# TG10 Program Source

This folder contains the cleaned source snapshot for the TG10 embedded project.

## Included

- `applications/`: application logic, drivers, Lua API adapters, runtime diagnostics, and main program code.
- `board/`: board support code, CubeMX source/config, FAL/Flash/filesystem ports, linker and startup files.
- Root project files: RT-Thread config, SCons files, Keil project file, Kconfig, and helper scripts.
- Debug/demo helpers: PPP/MQTT bridge script and serial web simulator.

## Not included

Large generated files and third-party SDK trees are intentionally excluded from this Git snapshot:

- `build/`, `Debug/`, `DebugConfig/`, Keil intermediate outputs, firmware images, map/hex/bin/axf files.
- Full `rt-thread/`, `libraries/`, `packages/`, and large toolchain/vendor directories.
- Local backup/cache folders.

The original full local project path was:

```text
D:\国汉实习\tg10程序5_19\tg10程序\tg10程序
```

To rebuild the project, restore the required RT-Thread, STM32H7 HAL/CMSIS, LuatOS/packages, and other vendor libraries according to the original project environment, then use the included project/config files as the application snapshot.
