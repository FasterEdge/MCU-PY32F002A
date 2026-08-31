# FasterEdge MCU — PY32F002A

FasterEdge 在 Puya **PY32F002Ax5 / Cortex-M0+** 上的无网络裸机实现。

## 芯片与资源

- Cortex-M0+，最高 24 MHz
- 20 KB Flash（`0x08000000..0x08004FFF`）
- 3 KB SRAM（`0x20000000..0x20000BFF`）
- USART1、SPI1、I2C1、TIM1、TIM16、LPTIM1、ADC、COMP、CRC、SWD
- GPIOA / GPIOB / GPIOF；不同 SOP/QFN/TSSOP 封装引脚数量不同
- 无硬件 EEPROM、DMA、RTC、USART2 和网络

## 开发平台目录

```text
MCU-PY32F002A/
├── keil/                       # Keil MDK-ARM / Puya DFP 工程源码
│   ├── MDK-ARM/
│   ├── Core/
│   ├── Inc/
│   ├── Ability/
│   ├── Data/
│   └── User/
├── platformio_ide/             # PlatformIO + ARM GCC + Puya LL SDK
│   ├── platformio.ini
│   ├── include/
│   └── src/
└── gcc/                        # 官方兼容 GCC 启动文件与 linker script
    ├── startup/
    └── ld/
```

## 已实现模块

Ability：`BaseAbility`、`RoleAbility`、`TimeAbility`、`OneKeyAbility`、`SerialAbility`、`ModbusAbility`、`RegAbility`、`GpioAbility`。

Data：`BaseData`、`ConfigData`、`ChipData`。

网络能力被裁剪。`ModbusAbility` 当前提供本地寄存器表和 CLI 命令；RTU 完整帧服务入口仍作为移植扩展点。

## PlatformIO

```bash
cd platformio_ide
pio run
pio run -t upload
pio device monitor
```

配置使用：

- `platform = https://github.com/Community-PIO-PY32F0/platform-puya32f0.git`
- `board = genericPY32F002Ax5`
- `framework = puya-py32f0-ll-sdk`
- CMSIS-DAP / SWD

硬件端口位于 `platformio_ide/src/fe_port.c`，使用 Puya CMSIS 寄存器结构而不是 CH32/WCH 寄存器布局。

默认 USART1：PA9 TX、PA10 RX、AF1。烧录前必须按实际封装的数据手册确认这两个引脚确实引出；低引脚数封装可能需要改用合法重映射。

逻辑 GPIO 编号：

- 0..7 → PA0..PA7
- 8..15 → PB0..PB7
- 16..17 → PF0..PF1

这只是统一 API 映射，不代表每种封装都引出了所有引脚。SWD 引脚不应被业务 GPIO 占用。

串口命令接收器会在输入超过 95 字节时丢弃整行并返回 `ERR input: line too long`，不会执行被截断的命令前缀。框架名称列表和命令路由也包含空指针及输出边界保护。

## Keil MDK

安装 `Puya.PY32F0xx_DFP`，选择 `PY32F002Ax5`：

- IROM1：`0x08000000`，大小 `0x5000`
- IRAM1：`0x20000000`，大小 `0x0C00`
- Define：`PY32F002Ax5`
- Startup：DFP 中的 `startup_py32f002ax5.s`
- Flash Algorithm：Puya 20 KB Flash 算法

`keil/` 已按 Core / Inc / Ability / Data / User 分组提供源码。若本机 DFP 版本生成的 uVision XML 不同，请用 DFP 新建目标并把这些分组加入，而不要使用 8051/C51 工具链。

## 持久化说明

PY32F002A 没有硬件 EEPROM。当前 `fe_port_eeprom_*` 和 `fe_port_persistence_available()` 明确返回不可用，避免错误擦除应用程序。因此 `ConfigData` 和依赖持久化令牌登记表的 `OneKeyAbility` 会明确返回 `persistent storage unavailable`，不会虚假报告“saved”或签发重启后失效的令牌。需要持久化时应：

1. 在 linker script 中保留完整 4 KB Flash sector；
2. 使用带 CRC 和 generation 的追加式 journal；
3. 处理擦写期间掉电和中断；
4. 将可用应用 Flash 相应减小。

20 KB 芯片保留一个 4 KB sector 已占 20%，因此默认不擅自启用。

## 串口命令示例

```text
help
ability_BaseAbility list_ability_names
ability_RoleAbility set_role edge
ability_TimeAbility sync_manual 1700000000
ability_OneKeyAbility issue_token sensor01
ability_ModbusAbility write_holding 0,42
ability_ModbusAbility read_holding 0,4
ability_GpioAbility mode 0,output
ability_GpioAbility write 0,1
data_ChipData info
```

> `RegAbility` 直接读写 MMIO。错误地址或错误写值可能触发 HardFault、关闭时钟或锁死调试接口，仅用于底层调试。

## 参考

- [Puya PY32F0 GCC SDK/template](https://github.com/IOsetting/py32f0-template)
- [Community PlatformIO PY32F0 platform](https://github.com/Community-PIO-PY32F0/platform-puya32f0)
- [PY32 documents mirror](https://github.com/decaday/PY32_Docs/tree/main/PY32F030_003_002A)
