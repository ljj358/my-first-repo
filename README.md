# STM32 / RT-Thread Embedded Projects

本仓库收录 3 个嵌入式项目，覆盖 **STM32 外设交互、智能车基础控制、RT-Thread 工业主机应用** 三类场景。仓库重点保留源码、工程配置和关键实现，已排除编译产物、固件文件、本地缓存和无关备份。

## 仓库结构

```text
.
├── 2024.8.30_1/       # RT-Thread + LVGL 多功能外设交互平台
├── 循迹基本代码/       # STM32F1 双路红外循迹小车基础控制
├── tg10_source/        # STM32H7 + RT-Thread 路侧主机核心源码快照
└── README.md
```

## 1. RT-Thread + LVGL 多功能外设交互平台

目录：`2024.8.30_1/`

该工程是一个基于 **RT-Thread + LVGL** 的多任务外设交互平台，使用图形界面对多个外设功能进行统一入口管理。工程中通过 RT-Thread 线程、邮箱和互斥锁组织后台任务与 UI 更新，适合展示外设驱动集成、图形界面交互和多任务协作能力。

### 主要功能

- **LVGL 多页面交互**：每个功能任务创建独立页面，支持 Esc 返回主菜单。
- **RT-Thread 任务调度**：使用线程、邮箱 `rt_mb_send_wait` 和互斥锁 `rt_mutex_take` 管理任务执行与界面资源。
- **PAJ7620 手势识别**：识别上、下、左、右、前、后、顺时针、逆时针、挥动等手势，并联动语音播报和界面显示。
- **AS608 指纹识别**：实现指纹搜索匹配、ID 输入、指纹删除、指纹录入、特征生成、模板保存，并联动步进电机动作。
- **RC522 RFID 读写**：提供 LVGL 输入界面，支持密钥输入、扇区/块选择、读卡、写卡和参数校验。
- **HX711 电子秤**：实现去皮、重量采集、实时显示和校准参数调节。
- **BH1750 光照曲线**：周期采集光照数据，维护 40 点历史数据，并用 LVGL chart 绘制实时曲线。
- **DS1302 实时时钟**：读取年月日时分秒和星期，并在界面中周期刷新。
- **密码锁/舵机控制**：通过 LVGL 数字键盘输入密码，密码正确后控制舵机开锁并延时复位。
- **蜂鸣器/RGB LED 演示**：实现音阶播放、旋律演示和 RGB 灯状态切换。

### 关键代码

```text
2024.8.30_1/src/task/          # RT-Thread + LVGL 任务页面
2024.8.30_1/src/Quest/         # RFID、电子秤、光照曲线、密码锁等功能界面
2024.8.30_1/src/AS608/         # 指纹模块驱动
2024.8.30_1/src/rc522/         # RFID 驱动
2024.8.30_1/src/HX711/         # 称重模块驱动
2024.8.30_1/src/bh1750/        # 光照传感器驱动
2024.8.30_1/src/paj7620u2/     # 手势识别驱动
```

### 技能点

`RT-Thread`、`LVGL`、`线程/邮箱/互斥锁`、`UART/SPI/I2C/GPIO/PWM`、`指纹识别`、`RFID 读写`、`传感器采集`、`图表显示`、`外设联动控制`

## 2. STM32F1 双路红外循迹小车

目录：`循迹基本代码/`

该工程是一个基于 **STM32F1** 的智能车基础控制项目，完成按键启动、双路红外循迹检测、四路 PWM 电机控制和运动动作封装。主程序根据左右红外传感器状态实时执行前进、左右修正和停止，实现基础循迹闭环。

### 主要功能

- **按键启动**：上电后等待按键触发，避免小车立即运行。
- **双路红外检测**：使用 `GPIOB Pin12 / Pin13` 读取左右循迹状态。
- **四路 PWM 控制**：通过 `TIM4 CH1-CH4` 输出 PWM，分别控制左右电机正反转通道。
- **运动动作封装**：封装前进、左修正、右修正、刹车、后退、原地左旋和原地右旋。
- **循迹判断逻辑**：根据左右传感器组合判断直行、左转修正、右转修正或停止。

### 控制逻辑

```text
左=0 右=0 -> 前进
左=1 右=0 -> 左转修正
左=0 右=1 -> 右转修正
左=1 右=1 -> 停止
```

### 关键代码

```text
循迹基本代码/User/main.c              # 主控制流程
循迹基本代码/Hardware/Irtracking.c    # 红外循迹输入
循迹基本代码/Hardware/robot.c         # 小车运动动作封装
循迹基本代码/Hardware/PWM.c           # TIM4 PWM 输出
循迹基本代码/Hardware/Key.c           # 按键启动
```

### 技能点

`STM32F1`、`GPIO 输入`、`TIM4 PWM`、`电机控制`、`传感器状态判断`、`基础闭环控制`、`Keil 工程`

## 3. STM32H7 + RT-Thread 路侧主机核心源码

目录：`tg10_source/`

该工程是一个基于 **STM32H7 + RT-Thread** 的路侧主机应用源码快照，保留应用层、板级适配、Lua 接口、通信框架和关键工程配置。完整 SDK、编译产物和本地备份未上传，便于聚焦核心业务代码。

### 主要功能

- **RT-Thread 应用框架**：包含主程序、运行诊断、启动状态标记和 Shell 调试相关代码。
- **多链路通信**：封装 UART、RS485、PowerBus 等通信处理逻辑，支持接收、解析、发送和异常保护。
- **Lua 接口封装**：对接 LuatOS/Lua，封装 AIO、DIO、GNSS、LoRa 等业务接口。
- **LoRa Mesh 灯控**：实现 Mesh 灯控相关 API，支持灯具模式、频率、亮度等控制链路。
- **存储与文件系统**：包含 FAL、Flash、文件系统和参数存储相关板级适配代码。
- **联网与调试辅助**：包含 PPP/MQTT 串口桥接脚本、Web 串口模拟页面等调试工具。
- **工程配置**：保留 `.config`、`rtconfig.h`、`SConstruct`、`project.uvprojx` 等关键配置文件。

### 关键代码

```text
tg10_source/applications/main.c              # 主程序入口
tg10_source/applications/drv/                # 应用驱动与参数/串口相关代码
tg10_source/applications/lua/                # Lua 侧硬件接口封装
tg10_source/applications/mesh_lua.h          # Mesh/Lua 接口相关声明
tg10_source/board/port/fal_cfg.h             # FAL 分区配置
tg10_source/board/port/fal_flash_ospi_mem_port.c
tg10_source/board/port/filesystem.c          # 文件系统挂载与初始化
tg10_source/rtconfig.h                       # RT-Thread 配置
tg10_source/.config                          # menuconfig 配置快照
```

### 技能点

`STM32H7`、`RT-Thread`、`UART/RS485/PowerBus`、`LuatOS/Lua`、`LoRa Mesh`、`FAL`、`Flash/文件系统`、`PPP/MQTT 调试`、`Keil + SCons 工程配置`

## 说明

- 仓库以源码展示为主，不包含完整 SDK、工具链和编译输出。
- 已排除 `build/`、`Debug/`、`.axf/.bin/.hex/.map`、Keil 中间文件、本地备份和缓存目录。
- `tg10_source/` 是从完整 TG10 工程中整理出的核心源码快照，若需重新编译，需要恢复对应 RT-Thread、STM32H7 HAL/CMSIS、LuatOS/packages 等依赖环境。
