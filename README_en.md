# FasterEdge MCU — PY32F002A

Bare-metal, no-network FasterEdge implementation for Puya **PY32F002Ax5 / Cortex-M0+**.

## Device

- Cortex-M0+, up to 24 MHz
- 20 KB Flash (`0x08000000..0x08004FFF`)
- 3 KB SRAM (`0x20000000..0x20000BFF`)
- USART1, SPI1, I2C1, TIM1, TIM16, LPTIM1, ADC, comparators, CRC and SWD
- GPIOA/GPIOB/GPIOF; exposed pins depend on package
- No hardware EEPROM, DMA, RTC, USART2 or networking

## Platform-separated layout

```text
MCU-PY32F002A/
├── keil/             # Keil MDK-ARM source groups
├── platformio_ide/   # PlatformIO, Arm GCC and Puya LL SDK
└── gcc/              # PY32F002A startup and linker script
```

Abilities: Base, Role, Time, OneKey, Serial, Modbus, register access and GPIO. Data modules: BaseData, ConfigData and ChipData. Network-dependent modules are intentionally excluded.

## PlatformIO

```bash
cd platformio_ide
pio run
pio run -t upload
pio device monitor
```

The project uses the community Puya platform, `genericPY32F002Ax5`, and `puya-py32f0-ll-sdk`. The port uses Puya CMSIS register definitions.

USART1 defaults to PA9 TX / PA10 RX / AF1. Verify alternate-function and package availability before flashing. Logical GPIO numbering is PA0..7 = 0..7, PB0..7 = 8..15 and PF0..1 = 16..17; not every package exposes every pin.

## Keil MDK

Install `Puya.PY32F0xx_DFP`, select `PY32F002Ax5`, define `PY32F002Ax5`, configure IROM1 as `0x08000000/0x5000` and IRAM1 as `0x20000000/0x0C00`, and use the DFP startup and 20 KB Flash algorithm. Add the provided Core, Inc, Ability, Data and User groups.

## Persistence

The MCU has no EEPROM. `fe_port_eeprom_*` and `fe_port_persistence_available()` safely report unavailable by default. Consequently ConfigData and the persistent OneKey token registry return `persistent storage unavailable` instead of falsely reporting successful saves or issuing reboot-volatile credentials. Flash persistence must reserve a complete 4 KB sector in the linker script and use a power-fail-safe CRC/generation journal; one sector consumes 20% of this device's Flash.

## References

- [Puya GCC SDK/template](https://github.com/IOsetting/py32f0-template)
- [Community PlatformIO platform](https://github.com/Community-PIO-PY32F0/platform-puya32f0)
- [PY32 documentation mirror](https://github.com/decaday/PY32_Docs/tree/main/PY32F030_003_002A)
