<div align="center">
<img src="https://avatars.githubusercontent.com/u/245985800?s=200&v=4" style="width:100px;" width="100"/>
<h2>FasterEdge MCU - PY32F002A</h2>
<h3>FasterEdge framework on PY32F002A (Cortex-M0+) (Keil MDK / PlatformIO editions)</h3>
</div>

### 1. Introduction

This repository implements the **[FasterEdge](https://github.com/FasterEdge/FasterEdge)** framework on the **PY32F002Ax5 (Cortex-M0+)**. This 32-bit Cortex-M0+ device runs at up to 24MHz, provides 20KB Flash and 3KB SRAM, has no hardware EEPROM, networking or operating system. Following the [MCU-C51](../MCU-C51) no-network design, the capability set is trimmed and 3 **MCU-specific** modules (registers / GPIO / chip info) are kept.

- ✅ **keil/ (Keil MDK, Puya DFP)** + **platformio_ide/ (ARM GCC, PlatformIO)** dual editions
- ✅ Same names & commands as the main repo — peer programming for edge/cloud
- ✅ HMAC-SHA256 in pure C (zero dependencies)
- ✅ Puya CMSIS register definitions instead of CH32/WCH peripheral assumptions
- ✅ platformio_ide edition ships **Cortex-M0+ register-level drivers** (GPIO / USART1 / SysTick)
- ⚠️ PY32F002A has no hardware EEPROM; ConfigData and persistent OneKey operations explicitly report unavailable until a safe Flash journal backend is provided

### 2. Implemented Capabilities (no-network subset)

**Abilities (8)**

| Name | Type | Commands |
|------|------|----------|
| `BaseAbility` | Base | `list_data_names` / `list_ability_names` |
| `RoleAbility` | Role | `describe` / `set_role` / `get_role` |
| `TimeAbility` | Time | `sync_manual` / `sync_system` / `get_time` / `configure_run` (no NTP) |
| `OneKeyAbility` | Token | `issue_token` / `verify_token` / `revoke_all` / `list_tokens` / `status` / `rotate` (requires persistence backend) |
| `SerialAbility` | Serial | `open` / `close` / `write` / `read` / `is_open` / `set_config` / `get_config` / `list_ports` |
| `ModbusAbility` | Modbus | `set_unit_id` / `get_unit_id` / `read_holding` / `read_input` / `read_coils` / `read_discrete` / `write_holding` / `write_coil` (RTU service entry) |
| `RegAbility` | Reg (own) | `read` / `write` / `bit_set` / `bit_clear` / `info` |
| `GpioAbility` | GPIO (own) | `mode` / `write` / `read` / `info` |

**Data (3)**

| Name | Function | Commands |
|------|----------|----------|
| `BaseData` | Framework metadata | `logo` / `info` |
| `ConfigData` | Flat KV configuration (safe persistence backend required) | `get` / `set` / `delete` / `list` / `snapshot` |
| `ChipData` | Chip information (own) | `info` |

### 3. Excluded Capabilities

| Capability | Reason |
|------------|--------|
| MQTTAbility / NetMapData | No network stack on PY32F002A |
| EdgeRoleAbility | Needs network heartbeat |
| ConfigFileAbility | Redundant with ConfigData; no filesystem concept |
| KeyringData | Merged into OneKeyAbility; no hardware EEPROM is available |
| TimeAbility.sync_ntp | No network for SNTP time synchronization |

### 4. Directory Layout

```
MCU-PY32F002A/
├── keil/                       # Keil MDK-ARM / Puya DFP project
│   ├── MDK-ARM/                # FasterEdge-MCU-PY32F002A.uvproj
│   ├── Core/                   # fe.h / fe.c / fe_hmac_sha256.c
│   ├── Inc/                    # fe_ability.h / fe_data.h / fe_port.h
│   ├── Ability/                # ability_*.c (8)
│   ├── Data/                   # data_*.c (3)
│   └── User/                   # main.c / register.c / fe_port.c
├── platformio_ide/             # VS Code + PlatformIO + ARM GCC project
│   ├── platformio.ini          # genericPY32F002Ax5 + Puya LL SDK
│   ├── include/                # shared headers
│   └── src/                    # bare-metal C + Puya CMSIS port
└── gcc/                        # GCC startup and linker files
    ├── startup/                # startup_py32f002a.s
    └── ld/                     # py32f002ax5.ld
```

> Both editions keep platform-specific source trees under `keil/` and `platformio_ide/`; capabilities and commands remain isomorphic, and the shared port behavior is synchronized.

### 5. Usage

1. **keil edition**: install `Puya.PY32F0xx_DFP`, open `keil/MDK-ARM/FasterEdge-MCU-PY32F002A.uvproj` in Keil MDK (or create an ARM target with the memory settings below), build and flash through SWD
2. **platformio_ide edition**: install the **PlatformIO IDE** extension, open `platformio_ide/`, then Build / Upload / Serial Monitor
3. Verify USART1 alternate-function pins and exposed GPIO against the exact package datasheet before flashing

**Serial command examples:**

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

### 6. Platform Differences

| Aspect | ESP32/ESP8266 | PY32F002Ax5 |
|--------|---------------|-------------|
| Architecture | Xtensa 32-bit | **ARM Cortex-M0+ 32-bit** |
| RAM / Flash | KB~MB | **3KB SRAM / 20KB Flash** |
| Storage | NVS / Flash | **No hardware EEPROM; Flash emulation disabled by default** |
| Network | Yes | **No** (network items trimmed) |
| Serial | Multiple UARTs | **USART1, PA9/PA10 by default, AF1** |
| GPIO | Chip-specific | **GPIOA / GPIOB / GPIOF, package-dependent** |
| Registers | 32-bit MMIO | **32-bit peripheral space 0x40000000+** |

### 7. platformio_ide Notes (ARM GCC)

The `platformio_ide/` edition is an **ARM GCC** project using the Community Puya PlatformIO platform and `puya-py32f0-ll-sdk`. `fe_port.c` uses the official CMSIS definitions for `GPIO_TypeDef`, `RCC`, `USART1` and `SysTick`.

| Function | Implementation |
|----------|----------------|
| UART | **USART1** (default PA9 TX / PA10 RX / AF1) |
| Persistence | No hardware EEPROM; `fe_port_persistence_available()` returns FALSE by default |
| Time | **SysTick** (1ms) |
| GPIO | **GPIOA / GPIOB / GPIOF** (MODER / PUPDR / IDR / BSRR) |
| Random | UID and SysTick mixed xorshift |

```bash
cd platformio_ide
pio run            # build
pio run -t upload  # flash via CMSIS-DAP
pio device monitor # serial monitor (115200)
```

The command-line buffer is 96 bytes. Lines longer than 95 bytes are discarded as a whole and return an error; a truncated command prefix is never executed.

### 8. MCU-Specific Modules

Beyond the main-repo capabilities, 3 **MCU-specific** modules (registers / GPIO / chip info) are provided. Logical GPIO numbering is:

- `0..7` → PA0..PA7
- `8..15` → PB0..PB7
- `16..17` → PF0..PF1

Actual availability depends on the package.

| Module | Type | Commands | Description |
|--------|------|----------|-------------|
| RegAbility | Ability | `read` / `write` / `bit_set` / `bit_clear` / `info` | Direct ARM 32-bit MMIO access |
| GpioAbility | Ability | `mode` / `write` / `read` / `info` | PY32F002A GPIO, logical pins 0..17 |
| ChipData | Data | `info` | PY32F002Ax5 model / RAM / Flash / frequency |

**Examples:**

```
ability_RegAbility read 0x40000000
ability_RegAbility bit_set 0x40000000,3
ability_GpioAbility mode 0,output
ability_GpioAbility write 0,1
ability_GpioAbility read 0
data_ChipData info
```

> ⚠️ `RegAbility` accesses hardware MMIO directly. A wrong address or write value may trigger HardFault, disable peripheral clocks or affect the debug interface. Use for low-level debugging only.

### 9. Correspondence with the Main Repo

- Commands match the main repo exactly and follow the same structure as MCU-C51 / MCU-ESP32
- `Atom` model: singleton global Atom with `data_` / `ability_` prefix routing
- HMAC-SHA256 is pure C with no mbedTLS dependency
- Modbus register tables live in RAM; `modbus_slave_service()` provides the complete RTU ADU service entry
- Because PY32F002A has no hardware EEPROM, configuration and token registration do not pretend to be persistent

### 10. Sibling Projects

- **[FasterEdge MCU - PY32F003](https://github.com/FasterEdge/MCU-PY32F003)**: same-family Cortex-M0+ platform
- **[FasterEdge MCU - ESP32](https://github.com/FasterEdge/MCU-ESP32)**: dual-core, WiFi/BLE and more peripherals
- **[FasterEdge MCU - ESP8266](https://github.com/FasterEdge/MCU-ESP8266)**: WiFi and low power
- **[FasterEdge MCU - C51](https://github.com/FasterEdge/MCU-C51)**: 8-bit 8051, most minimal
- **[FasterEdge MCU - Arduino Uno R3](https://github.com/FasterEdge/MCU-Arduino-Uno-R3)**: 8-bit AVR (ATmega328P)
- **[FasterEdge MCU - Arduino Uno R4](https://github.com/FasterEdge/MCU-Arduino-Uno-R4)**: 32-bit Cortex-M4F (RA4M1)
- **[FasterEdge](https://github.com/FasterEdge/FasterEdge)**: framework main repository

### 11. Hardware and Toolchain Notes

- Keil requires `Puya.PY32F0xx_DFP` and device `PY32F002Ax5`
- IROM1: `0x08000000`, size `0x5000`
- IRAM1: `0x20000000`, size `0x0C00`
- Flash page size is 128 bytes; sector erase size is 4KB
- Do not use SWD pins or package-unexposed GPIOs as application pins
- Flash persistence requires a reserved sector plus CRC, generation and power-fail recovery journal

### 12. References

- [Puya PY32F0 GCC SDK/template](https://github.com/IOsetting/py32f0-template)
- [Community PlatformIO PY32F0 platform](https://github.com/Community-PIO-PY32F0/platform-puya32f0)
- [PY32 documents mirror](https://github.com/decaday/PY32_Docs/tree/main/PY32F030_003_002A)
