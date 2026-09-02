// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// fe_ability.h — FasterEdge MCU Ability 模块声明（PY32F002A (Cortex-M0+) 版）
// 资源受限无网络子集：Base / Role / Time / OneKey / Serial / Modbus
// 排除：MQTT（无网络）、EdgeRole（无网络心跳）、ConfigFile（与 Data 重复）
#ifndef FE_ABILITY_H
#define FE_ABILITY_H

#include "fe.h"

// ============================================================
// BaseAbility —— list_data_names / list_ability_names
// ============================================================
fe_output_t ability_base_dispatch(void *inst, const char *act, const char *args);

// ============================================================
// RoleAbility —— describe / set_role / get_role
// ============================================================
typedef struct {
    char role[12];   // edge / cloud / standalone
} role_ability_t;
fe_output_t ability_role_dispatch(void *inst, const char *act, const char *args);

// ============================================================
// TimeAbility —— sync_manual / sync_system / get_time / configure_run
// 无网络：configure_run 记录可由主循环消费的周期调度状态。
// ============================================================
typedef struct {
    u32 manual_epoch;
    u32 run_interval;
    u32 next_run;
    u8  run_enabled;
} time_ability_t;
fe_output_t ability_time_dispatch(void *inst, const char *act, const char *args);

// ============================================================
// OneKeyAbility —— issue_token / verify_token / revoke_all /
//                  list_tokens / status / rotate
// （HMAC-SHA256；只有配置安全持久化后端后才启用）
// ============================================================
typedef struct {
    char secret[33];  // HMAC 密钥（持久化后端，32 字节 + NUL）
    u32  seq;         // 令牌序列
} onekey_ability_t;
fe_output_t ability_onekey_dispatch(void *inst, const char *act, const char *args);

// ============================================================
// SerialAbility —— open / close / write / read / is_open /
//                  set_config / get_config / list_ports
// ============================================================
typedef struct {
    u8  open;
    u32 baud;
    u8  port;
} serial_ability_t;
fe_output_t ability_serial_dispatch(void *inst, const char *act, const char *args);

// ============================================================
// ModbusAbility —— set_unit_id / get_unit_id / read_holding /
//                  read_input / read_coils / read_discrete /
//                  write_holding / write_coil
// （RTU 从站寄存器表存 RAM）
// ============================================================
typedef struct {
    u8    unit_id;
    u16   holding_regs[32];
    u16   input_regs[32];
    u8    coils[32];
    u8    discrete_inputs[32];
} modbus_ability_t;
fe_output_t ability_modbus_dispatch(void *inst, const char *act, const char *args);
// 处理一个完整 Modbus RTU ADU，并通过 fe_port_uart_write(0, ...) 回写响应。
void modbus_slave_service(modbus_ability_t *self, const u8 *req, u16 len);

// ============================================================
// RegAbility —— ARM 32 位 MMIO：read / write / bit_set / bit_clear / info
// ============================================================
fe_output_t ability_reg_dispatch(void *inst, const char *act, const char *args);

// ============================================================
// GpioAbility —— MCU 专有·端口 GPIO 控制：mode / write / read / info
// 逻辑 GPIO 0..17，实际可用性取决于封装
// ============================================================
fe_output_t ability_gpio_dispatch(void *inst, const char *act, const char *args);

// ============================================================
// 注册全部 Ability（register.c 调用）
// ============================================================
void fe_register_all_abilities(fe_atom_t *atom);

#endif // FE_ABILITY_H
