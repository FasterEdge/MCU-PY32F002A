// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// fe.h — FasterEdge MCU 核心框架（PY32F002A (Cortex-M0+) 版）
// 面向 ARM GCC / ARMClang 的裸机实现：无操作系统、无动态内存。
// 固定宽度整数确保 Cortex-M0+ 上的协议与加密运算宽度明确。
// 对应 FasterEdge 主仓库的 Atom / Ability / Data / Command 模型。
// 平台相关操作通过 fe_port.h 抽象（UART / EEPROM / 定时器）。
#ifndef FE_H
#define FE_H

#include <stdint.h>

// ============================================================
// 固定宽度基础类型
// ============================================================
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;

// 轻量布尔常量，避免公共 ABI 依赖 C 标准版本。
#define TRUE  1
#define FALSE 0

// 常用数学
#define FE_ABS(x) ((x) < 0 ? -(x) : (x))

// ============================================================
// 基础类型
// ============================================================

// 命令输出（对应 FasterEdge 的 CommandOutput）
// 缓冲按 3 KB SRAM 目标控制大小；所有写入必须经过有界格式化。
typedef struct {
    char name[16];      // 命令名
    char value[96];     // 返回值（文本 / JSON）
    char err[32];       // 错误信息（空 = 成功）
    u8   ok;
} fe_output_t;

// 命令执行回调：inst 为各模块实例，act 命令名，args 参数字符串
typedef fe_output_t (*fe_cmd_handler_t)(void *inst, const char *act, const char *args);

// 命令表项
typedef struct {
    const char *name;
    fe_cmd_handler_t handler;
} fe_cmd_t;

// 模块（Data / Ability 通用描述）
typedef struct {
    const char *name;       // 如 "BaseData" / "BaseAbility"
    const char *desc;
    const fe_cmd_t *cmds;
    u8 cmd_count;
    void *instance;
    fe_cmd_handler_t dispatch;
} fe_module_t;

// ============================================================
// Atom：注册表 + 命令路由
// ============================================================
#define FE_MAX_MODULES 16

typedef struct {
    fe_module_t data[FE_MAX_MODULES];
    u8 data_count;
    fe_module_t ability[FE_MAX_MODULES];
    u8 ability_count;
} fe_atom_t;

// 注册
void fe_register_data(fe_atom_t *atom, const fe_module_t *mod);
void fe_register_ability(fe_atom_t *atom, const fe_module_t *mod);

// 查询名称列表（写入逗号分隔字符串）
void fe_list_data_names(const fe_atom_t *atom, char *out, u16 outlen);
void fe_list_ability_names(const fe_atom_t *atom, char *out, u16 outlen);

// 执行命令：target 形如 "data_BaseData" / "ability_BaseAbility"
fe_output_t fe_execute(fe_atom_t *atom, const char *target, const char *act, const char *args);

// 构建输出工具
fe_output_t fe_ok(const char *name, const char *value);
fe_output_t fe_err(const char *name, const char *err);

// ============================================================
// 全局 Atom + 初始化（由 register.c 实现）
// ============================================================
fe_atom_t *fe_global_atom(void);
void fe_init_all(void);

#endif // FE_H
