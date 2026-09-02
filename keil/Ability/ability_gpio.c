// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// ability_gpio.c — PY32F002A GpioAbility（MCU 专有）
// 逻辑引脚映射由 fe_port.c 定义，并受实际封装引脚数量限制。
//   mode <pin>,<input|output|input_pullup>  设置引脚模式
//   write <pin>,<0|1>                       输出电平
//   read <pin>                              读电平
//   info                                    说明
// pin 0..17；默认映射 PA0..7 / PB0..7 / PF0..1。
#include "fe_ability.h"
#include "fe_port.h"
#include <string.h>
#include <stdlib.h>

fe_output_t ability_gpio_dispatch(void *inst, const char *act, const char *args) {
    char tmp[32];
    char *end;
    u8 pin;

    (void)inst;
    fe_snprintf(tmp, sizeof(tmp), "%s", args ? args : "");

    if (strcmp(act, "mode") == 0) {
        char *comma = strchr(tmp, ',');
        if (!comma) return fe_err(act, "bad format, expect pin,mode");
        *comma = 0;
        pin = (u8)strtoul(tmp, &end, 0);
        if (pin > 17) return fe_err(act, "pin must be 0-17");
        if (fe_port_gpio_set_mode(pin, comma + 1) != 0)
            return fe_err(act, "mode must be input/output/input_pullup");
        {
            char out[32];
            fe_snprintf(out, sizeof(out), "{\"pin\":%u,\"mode\":\"%s\"}",
                        (unsigned)pin, comma + 1);
            return fe_ok(act, out);
        }
    }
    if (strcmp(act, "write") == 0) {
        char *comma = strchr(tmp, ',');
        if (!comma) return fe_err(act, "bad format, expect pin,value");
        *comma = 0;
        pin = (u8)strtoul(tmp, &end, 0);
        if (pin > 17) return fe_err(act, "pin must be 0-17");
        {
            u32 val = strtoul(comma + 1, &end, 0);
            if (val > 1) return fe_err(act, "value must be 0/1");
            if (fe_port_gpio_write(pin, (u8)val) != 0) return fe_err(act, "write failed");
            char out[32];
            fe_snprintf(out, sizeof(out), "{\"pin\":%u,\"value\":%u}",
                        (unsigned)pin, (unsigned)val);
            return fe_ok(act, out);
        }
    }
    if (strcmp(act, "read") == 0) {
        int v;
        pin = (u8)strtoul(tmp, &end, 0);
        if (pin > 17) return fe_err(act, "pin must be 0-17");
        v = fe_port_gpio_read(pin);
        if (v < 0) return fe_err(act, "read failed");
        {
            char out[32];
            fe_snprintf(out, sizeof(out), "{\"pin\":%u,\"value\":%d}",
                        (unsigned)pin, v);
            return fe_ok(act, out);
        }
    }
    if (strcmp(act, "info") == 0) {
        return fe_ok(act,
            "{\"ability\":\"GpioAbility\",\"desc\":\"PY32F002A GPIO\","
            "\"pins\":\"0-17(package dependent)\",\"width\":1}");
    }
    return fe_err(act, "unsupported command");
}
