<div align="center">
<img src="https://avatars.githubusercontent.com/u/245985800?s=200&v=4" style="width:100px;" width="100"/>
<h2>FasterEdge MCU - PY32F002A</h2>
<h3>FasterEdge 框架的 PY32F002A（Cortex-M0+）平台实现（Keil MDK / PlatformIO 版）</h3>
</div>

### 一、简介

本项目是 **[FasterEdge](https://github.com/FasterEdge/FasterEdge)** 框架在 **PY32F002Ax5（Cortex-M0+）** 平台上的实现。PY32F002Ax5 为 32 位 Cortex-M0+ 内核：最高 24MHz、20KB Flash、3KB SRAM、无硬件 EEPROM，无网络、无操作系统，因此按 [MCU-C51](../MCU-C51) 的无网络精简思路裁剪能力子集，并保留 **寄存器 / GPIO / 芯片信息** 三个 MCU 专有模块。

- ✅ **keil/（Keil MDK，Puya DFP）** + **platformio_ide/（ARM GCC，PlatformIO）** 双版本
- ✅ 与主仓库**同名同命令**，云边协同对等编程
- ✅ HMAC-SHA256 纯 C 零依赖
- ✅ 使用 Puya CMSIS 寄存器定义，避免复用 CH32/WCH 外设布局
- ✅ platformio_ide 版为 **Cortex-M0+ 寄存器级实现**（GPIO / USART1 / SysTick）
- ⚠️ PY32F002A 无硬件 EEPROM；未配置安全 Flash journal 时，ConfigData 和 OneKey 持久化操作明确返回不可用

### 二、已实现能力（无网络合理子集）

**Ability（8 个）**

| 名称 | 类别 | 命令 |
|------|------|------|
| `BaseAbility` | 基础 | `list_data_names` / `list_ability_names` |
| `RoleAbility` | 角色 | `describe` / `set_role` / `get_role` |
| `TimeAbility` | 时间 | `sync_manual` / `sync_system` / `get_time` / `configure_run`（无 NTP）|
| `OneKeyAbility` | 令牌 | `issue_token` / `verify_token` / `revoke_all` / `list_tokens` / `status` / `rotate`（需持久化后端）|
| `SerialAbility` | 串口 | `open` / `close` / `write` / `read` / `is_open` / `set_config` / `get_config` / `list_ports` |
| `ModbusAbility` | Modbus | `set_unit_id` / `get_unit_id` / `read_holding` / `read_input` / `read_coils` / `read_discrete` / `write_holding` / `write_coil`（RTU 从站入口）|
| `RegAbility` | 寄存器(专有) | `read` / `write` / `bit_set` / `bit_clear` / `info` |
| `GpioAbility` | GPIO(专有) | `mode` / `write` / `read` / `info` |

**Data（3 个）**

| 名称 | 功能 | 命令 |
|------|------|------|
| `BaseData` | 框架元信息 | `logo` / `info` |
| `ConfigData` | KV 配置（需安全持久化后端） | `get` / `set` / `delete` / `list` / `snapshot` |
| `ChipData` | 芯片信息(专有) | `info` |

### 三、排除项与理由

| 能力 | 排除原因 |
|------|---------|
| MQTTAbility / NetMapData | PY32F002A 无网络协议栈 |
| EdgeRoleAbility | 依赖网络心跳上报 |
| ConfigFileAbility | 与 ConfigData 重复，且无文件系统概念 |
| KeyringData | 与 OneKeyAbility 合并；当前无硬件 EEPROM |
| TimeAbility.sync_ntp | 无网络无法 SNTP 校时 |

### 四、目录结构

```
MCU-PY32F002A/
├── keil/                       # Keil MDK-ARM / Puya DFP 工程
│   ├── MDK-ARM/                # FasterEdge-MCU-PY32F002A.uvproj
│   ├── Core/                   # fe.h / fe.c / fe_hmac_sha256.c
│   ├── Inc/                    # fe_ability.h / fe_data.h / fe_port.h
│   ├── Ability/                # ability_*.c（8 个）
│   ├── Data/                   # data_*.c（3 个）
│   └── User/                   # main.c / register.c / fe_port.c
├── platformio_ide/             # VS Code + PlatformIO + ARM GCC
│   ├── platformio.ini          # genericPY32F002Ax5 + Puya LL SDK
│   ├── include/                # 公共头文件
│   └── src/                    # 裸机 C + Puya CMSIS 端口
└── gcc/                        # GCC 启动文件与 linker script
    ├── startup/                # startup_py32f002a.s
    └── ld/                     # py32f002ax5.ld
```

> 两套应用源码分别保存在 `keil/` 与 `platformio_ide/`，能力与命令保持同构；公共端口实现已同步，便于在不同开发平台间切换。

### 五、使用说明

1. **keil 版**：安装 `Puya.PY32F0xx_DFP`，用 Keil MDK 打开 `keil/MDK-ARM/FasterEdge-MCU-PY32F002A.uvproj`，或按下方内存参数新建 ARM 工程，编译后 SWD 烧录
2. **platformio_ide 版**：安装 **PlatformIO IDE** 插件，打开 `platformio_ide/` 目录，Build / Upload / Serial Monitor
3. 烧录前按实际封装数据手册确认 USART1 引脚和 GPIO 是否引出

**串口命令示例：**

```
help
ability_BaseAbility list_ability_names
ability_RoleAbility set_role edge
ability_TimeAbility sync_manual 1700000000
ability_OneKeyAbility issue_token sensor01
ability_ModbusAbility set_unit_id 3
ability_ModbusAbility write_holding 0,42
ability_ModbusAbility read_holding 0,4
ability_SerialAbility set_config 0,9600
ability_SerialAbility write hello
data_ConfigData set wifi.ssid=MyNet
data_ConfigData get wifi.ssid
data_ChipData info
```

### 六、平台适配要点

| 差异点 | ESP32/ESP8266 | PY32F002Ax5 |
|--------|---------------|-------------|
| 架构 | Xtensa 32 位 | **ARM Cortex-M0+ 32 位** |
| RAM / Flash | KB~MB | **3KB SRAM / 20KB Flash** |
| 存储 | NVS / Flash | **无硬件 EEPROM；默认不启用 Flash 模拟** |
| 网络 | 有 | **无**（能力子集剔除网络项）|
| 串口 | 多串口 | **USART1，默认 PA9/PA10，AF1** |
| GPIO | 芯片相关 | **GPIOA / GPIOB / GPIOF，封装相关** |
| 寄存器 | 32 位内存映射 | **32 位外设空间 0x40000000+** |

### 七、platformio_ide 版实现说明（ARM GCC）

`platformio_ide/` 为 **ARM GCC 版**工程，使用 Community Puya PlatformIO 平台和 `puya-py32f0-ll-sdk`。`fe_port.c` 使用官方 CMSIS 头文件中的 `GPIO_TypeDef`、`RCC`、`USART1` 与 `SysTick` 定义。

| 功能 | 实现 |
|------|------|
| UART | **USART1**（默认 PA9 TX / PA10 RX / AF1）|
| 持久化 | 无硬件 EEPROM；`fe_port_persistence_available()` 默认返回 FALSE |
| 时间 | **SysTick**（1ms）|
| GPIO | **GPIOA / GPIOB / GPIOF**（MODER / PUPDR / IDR / BSRR）|
| 随机数 | UID 与 SysTick 混合的 xorshift |

```bash
cd platformio_ide
pio run            # 编译
pio run -t upload  # CMSIS-DAP 烧录
pio device monitor # 串口监视（115200）
```

串口命令缓冲区为 96 字节。输入超过 95 字节时整行丢弃并返回错误，不会执行被截断的命令前缀。

### 八、MCU 专有模块

除主仓库对应能力外，本仓库提供 3 个 **MCU 专有**模块。逻辑 GPIO 编号如下，实际可用性取决于封装：

- `0..7` → PA0..PA7
- `8..15` → PB0..PB7
- `16..17` → PF0..PF1

| 模块 | 类型 | 命令 | 说明 |
|------|------|------|------|
| RegAbility | Ability | `read` / `write` / `bit_set` / `bit_clear` / `info` | ARM 32 位 MMIO，直接访问寄存器 |
| GpioAbility | Ability | `mode` / `write` / `read` / `info` | PY32F002A GPIO，逻辑 pin 0..17 |
| ChipData | Data | `info` | PY32F002Ax5 型号 / RAM / Flash / 频率 |

**示例：**

```
ability_RegAbility read 0x40000000
ability_RegAbility bit_set 0x40000000,3
ability_GpioAbility mode 0,output
ability_GpioAbility write 0,1
ability_GpioAbility read 0
data_ChipData info
```

> ⚠️ `RegAbility` 直接访问硬件 MMIO。错误地址或错误写值可能触发 HardFault、关闭外设时钟或影响调试接口，仅供底层调试使用。

### 九、与 FasterEdge 主仓库的对应关系

- 命令名与主仓库**完全一致**，与 MCU-C51 / MCU-ESP32 实现同构
- `Atom` 模型：单例全局 Atom，`data_` / `ability_` 前缀路由
- HMAC-SHA256 使用纯 C 实现，无 mbedTLS 依赖
- Modbus 寄存器表存于 RAM，`modbus_slave_service()` 提供完整 RTU ADU 服务入口
- PY32F002A 无硬件 EEPROM，因此配置和令牌登记默认不会伪装成已持久化

### 十、姊妹项目

- **[FasterEdge MCU - PY32F003](https://github.com/FasterEdge/MCU-PY32F003)**：同系列 Cortex-M0+ 平台
- **[FasterEdge MCU - ESP32](https://github.com/FasterEdge/MCU-ESP32)**：双核、WiFi/BLE、更多外设
- **[FasterEdge MCU - ESP8266](https://github.com/FasterEdge/MCU-ESP8266)**：WiFi、低功耗
- **[FasterEdge MCU - C51](https://github.com/FasterEdge/MCU-C51)**：8 位 8051，最精简
- **[FasterEdge MCU - Arduino Uno R3](https://github.com/FasterEdge/MCU-Arduino-Uno-R3)**：8 位 AVR（ATmega328P）
- **[FasterEdge MCU - Arduino Uno R4](https://github.com/FasterEdge/MCU-Arduino-Uno-R4)**：32 位 Cortex-M4F（RA4M1）
- **[FasterEdge](https://github.com/FasterEdge/FasterEdge)**：框架主仓库

### 十一、硬件与工具链注意事项

- Keil 需要安装 `Puya.PY32F0xx_DFP`，并选择 `PY32F002Ax5`
- IROM1：`0x08000000`，大小 `0x5000`
- IRAM1：`0x20000000`，大小 `0x0C00`
- Flash page 为 128 字节，sector 擦除大小为 4KB
- 不要把 SWD 引脚或未由当前封装引出的 GPIO 配置为业务引脚
- 若要增加 Flash 持久化，必须预留完整 sector，并实现 CRC、generation 和掉电恢复 journal

### 十二、参考资料

- [Puya PY32F0 GCC SDK/template](https://github.com/IOsetting/py32f0-template)
- [Community PlatformIO PY32F0 platform](https://github.com/Community-PIO-PY32F0/platform-puya32f0)
- [PY32 documents mirror](https://github.com/decaday/PY32_Docs/tree/main/PY32F030_003_002A)
