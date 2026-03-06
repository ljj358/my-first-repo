# AGENTS.md

## Project Overview

This is an embedded systems firmware project written in C for a **Loongson 1B (LS1B) MIPS-based SoC** running the **RT-Thread RTOS**. The target hardware is a **BKRC (°Ù¿ÆÈÙ´´) IoT education/competition development board**. The firmware provides a touchscreen GUI via LVGL 7.0.1 and integrates 20+ hardware sensor/actuator modules (ESP8266 WiFi, DHT11, BH1750, RC522 RFID, MAX30102, PAJ7620U2, AS608, stepper motors, etc.).

All source code resides in `2024.8.30_1/src/`.

## Cursor Cloud specific instructions

### Important Constraints

- **No build system in repo**: There is no Makefile, CMakeLists.txt, SConscript, or IDE project file. The original developers likely use RT-Thread Studio or a custom BSP-provided build environment.
- **Missing BSP/SDK**: Headers like `bsp.h`, `main.h`, `ls1b_gpio.h`, and the LVGL 7.0.1 library are referenced but **not included** in this repo. Full compilation requires the Loongson 1B RT-Thread BSP.
- **Cannot link or run natively**: This firmware targets MIPS hardware. It cannot be fully compiled or executed on x86 without the complete BSP, RT-Thread kernel, and LVGL sources.

### Available Development Tools

- **`cppcheck`** ¡ª Static analysis / lint for C code. Run:
  ```
  cppcheck --enable=all --suppress=missingInclude --suppress=missingIncludeSystem --language=c --std=c11 2024.8.30_1/src/
  ```
- **`mipsel-linux-gnu-gcc`** ¡ª MIPS (little-endian) cross-compiler. Useful for syntax checking individual files (though linking fails without BSP):
  ```
  mipsel-linux-gnu-gcc -fsyntax-only -I2024.8.30_1/src <file.c>
  ```
- **`qemu-mipsel-static`** ¡ª QEMU user-mode emulator for running statically-linked MIPS binaries (useful for testing standalone MIPS programs).
- **`gcc`** ¡ª Native x86 GCC, available for general C development tasks.

### Linting

Use cppcheck as the primary lint tool. The `--suppress=missingInclude` flags are necessary because BSP headers are not in the repo.

### Testing

There are no automated tests in this repository. Testing requires the physical BKRC hardware board. For code quality checks, rely on `cppcheck` static analysis.

### Building

Full builds require the RT-Thread BSP for Loongson 1B, which is not in this repo. Individual `.c` files can be syntax-checked with `mipsel-linux-gnu-gcc -fsyntax-only` but will fail on missing BSP headers.
