/* FasterEdge 开源项目
 * GitHub: https://github.com/FasterEdge
 * Gitee:  https://gitee.com/FasterEdge
 */
// FasterEdge hardware port for Puya PY32F002Ax5 (Cortex-M0+).
#include "fe_port.h"
#include "py32f0xx.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifndef FOSC
#define FOSC 24000000UL
#endif

static fe_port_uart_rx_cb_t s_rx_cb;
static void *s_rx_user;
static volatile u32 s_tick_ms;
static volatile u32 s_epoch_base;

int fe_snprintf(char *buf, u16 size, const char *fmt, ...) {
    va_list ap;
    int n;
    if (!buf || size == 0) return 0;
    va_start(ap, fmt);
    n = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    if (n < 0) { buf[0] = 0; return 0; }
    buf[size - 1] = 0;
    return n;
}

static GPIO_TypeDef *gpio_port(u8 pin, u8 *bit, u32 *clock_bit) {
    if (pin < 8) { *bit = pin; *clock_bit = RCC_IOPENR_GPIOAEN; return GPIOA; }
    if (pin < 16) { *bit = (u8)(pin - 8); *clock_bit = RCC_IOPENR_GPIOBEN; return GPIOB; }
    if (pin < 18) { *bit = (u8)(pin - 16); *clock_bit = RCC_IOPENR_GPIOFEN; return GPIOF; }
    return NULL;
}

void fe_port_uart_init(u8 port, u32 baud, fe_port_uart_rx_cb_t cb, void *user) {
    u32 pos;
    (void)port;
    s_rx_cb = cb;
    s_rx_user = user;
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    RCC->APBENR2 |= RCC_APBENR2_USART1EN;
    // USART1 default mapping: PA9 TX / PA10 RX, AF1. Check package pinout.
    GPIOA->MODER = (GPIOA->MODER & ~((3u << 18) | (3u << 20))) |
                   (2u << 18) | (2u << 20);
    GPIOA->OTYPER &= ~(1u << 9);
    GPIOA->OSPEEDR |= (3u << 18) | (3u << 20);
    GPIOA->PUPDR = (GPIOA->PUPDR & ~((3u << 18) | (3u << 20))) |
                   (1u << 18) | (1u << 20);
    pos = 4u;
    GPIOA->AFR[1] = (GPIOA->AFR[1] & ~(0xFu << pos)) | (1u << pos);
    pos = 8u;
    GPIOA->AFR[1] = (GPIOA->AFR[1] & ~(0xFu << pos)) | (1u << pos);
    USART1->CR1 = 0;
    USART1->BRR = (FOSC + baud / 2u) / baud;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

u16 fe_port_uart_write(u8 port, const u8 *data, u16 len) {
    u16 i;
    (void)port;
    for (i = 0; i < len; ++i) {
        while (!(USART1->SR & USART_SR_TXE));
        USART1->DR = data[i];
    }
    while (!(USART1->SR & USART_SR_TC));
    return len;
}

u8 fe_port_uart_available(u8 port) {
    (void)port;
    return (USART1->SR & USART_SR_RXNE) ? TRUE : FALSE;
}

int fe_port_uart_read(u8 port) {
    u8 value;
    (void)port;
    if (!(USART1->SR & USART_SR_RXNE)) return -1;
    value = (u8)USART1->DR;
    if (s_rx_cb) s_rx_cb(value, s_rx_user);
    return value;
}

void fe_port_uart_close(u8 port) {
    (void)port;
    USART1->CR1 &= ~(USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);
}

// No hardware EEPROM. Return unavailable until a linker-reserved, power-fail-
// safe Flash journal is configured; this avoids erasing application code.
u8 fe_port_eeprom_get_str(u16 addr, char *out, u16 outlen) {
    (void)addr;
    if (out && outlen) out[0] = 0;
    return FALSE;
}
u8 fe_port_eeprom_set_str(u16 addr, const char *value) {
    (void)addr; (void)value; return FALSE;
}
u8 fe_port_eeprom_get_u32(u16 addr, u32 *out) {
    (void)addr;
    if (out) *out = 0;
    return FALSE;
}
u8 fe_port_eeprom_set_u32(u16 addr, u32 value) {
    (void)addr; (void)value; return FALSE;
}
u8 fe_port_persistence_available(void) { return FALSE; }

void SysTick_Handler(void) { ++s_tick_ms; }
void fe_port_timer0_init(void) {
    s_tick_ms = 0;
    s_epoch_base = 0;
    (void)SysTick_Config(FOSC / 1000u);
}
u32 fe_port_time_now(void) { return s_epoch_base + s_tick_ms / 1000u; }
void fe_port_time_set(u32 epoch) { s_epoch_base = epoch - s_tick_ms / 1000u; }

void fe_port_random_fill(u8 *buf, u16 len) {
    static u32 state = 0x32F002A5u;
    u16 i;
    state ^= SysTick->VAL ^ *(volatile u32 *)UID_BASE;
    for (i = 0; i < len; ++i) {
        state ^= state << 13; state ^= state >> 17; state ^= state << 5;
        buf[i] = (u8)(state >> 24);
    }
}

int fe_port_gpio_set_mode(u8 pin, const char *mode) {
    GPIO_TypeDef *gpio;
    u8 bit;
    u32 clock_bit, shift, moder, pupdr;
    gpio = gpio_port(pin, &bit, &clock_bit);
    if (!gpio || !mode) return -1;
    RCC->IOPENR |= clock_bit;
    shift = (u32)bit * 2u;
    moder = gpio->MODER & ~(3u << shift);
    pupdr = gpio->PUPDR & ~(3u << shift);
    if (strcmp(mode, "output") == 0) moder |= 1u << shift;
    else if (strcmp(mode, "input_pullup") == 0) pupdr |= 1u << shift;
    else if (strcmp(mode, "input") != 0) return -1;
    gpio->MODER = moder;
    gpio->PUPDR = pupdr;
    return 0;
}

int fe_port_gpio_write(u8 pin, u8 level) {
    GPIO_TypeDef *gpio;
    u8 bit;
    u32 clock_bit;
    gpio = gpio_port(pin, &bit, &clock_bit);
    if (!gpio) return -1;
    RCC->IOPENR |= clock_bit;
    gpio->BSRR = level ? (1u << bit) : (1u << (bit + 16u));
    return 0;
}

int fe_port_gpio_read(u8 pin) {
    GPIO_TypeDef *gpio;
    u8 bit;
    u32 clock_bit;
    gpio = gpio_port(pin, &bit, &clock_bit);
    if (!gpio) return -1;
    RCC->IOPENR |= clock_bit;
    return (gpio->IDR & (1u << bit)) ? 1 : 0;
}

void fe_port_chip_info(char *out, u16 outlen) {
    fe_snprintf(out, outlen,
        "{\"chip\":\"PY32F002A\",\"arch\":\"Cortex-M0+\","
        "\"ramBytes\":3072,\"flashBytes\":20480,\"eepromBytes\":0,\"freqMHz\":24}");
}

void fe_port_delay_ms(u32 ms) {
    u32 start = s_tick_ms;
    while ((u32)(s_tick_ms - start) < ms) { __NOP(); }
}
