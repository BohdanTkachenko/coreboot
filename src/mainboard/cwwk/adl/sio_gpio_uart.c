/* SPDX-License-Identifier: GPL-2.0-or-later */

/*
 * Optional transmit-only software UART on one signal pin of the 3.3 V
 * IT8613E GPIO header. The board has no COM header. Bits are timed against
 * absolute TSC deadlines, which keeps 38400 baud accurate.
 */

#include <arch/io.h>
#include <assert.h>
#include <boot/coreboot_tables.h>
#include <console/uart.h>
#include <cpu/x86/tsc.h>
#include <delay.h>
#include <device/pnp_ops.h>
#include <superio/ite/common/ite.h>
#include <superio/ite/common/ite_gpio.h>
#include <superio/ite/it8613e/it8613e.h>
#include <types.h>

#define GPIO_DEV		PNP_DEV(0x2e, IT8613E_GPIO)
#define SIMPLE_IO_BASE		0x0a00	/* decoded by gen1_dec */

/* GPIO header signal pins 1-8 and their multi-function pin select register */
static const struct {
	u8 gpio;
	u8 mux_reg;
} header_pins[] = {
	{ 10, 0x25 }, { 22, 0x26 }, { 31, 0x27 }, { 32, 0x27 },
	{ 33, 0x27 }, { 41, 0x28 }, { 45, 0x28 }, { 63, 0x2d },
};

static_assert(ARRAY_SIZE(header_pins) == 8); /* Kconfig range 1-8 */

#define TX_GPIO		(header_pins[CONFIG_CWWK_SIO_GPIO_UART_TX_PIN - 1].gpio)
#define TX_MUX_REG	(header_pins[CONFIG_CWWK_SIO_GPIO_UART_TX_PIN - 1].mux_reg)
#define TX_PORT		(SIMPLE_IO_BASE + TX_GPIO / 10 - 1)
#define TX_MASK		(1 << (TX_GPIO % 10))

static uint64_t ticks_per_bit;

unsigned int get_uart_baudrate(void)
{
	return CONFIG_CWWK_SIO_GPIO_UART_BAUD;
}

uintptr_t uart_platform_base(unsigned int idx)
{
	return TX_PORT;
}

static void set_tx(int level, bool drive)
{
	/* Read back so that other header pins keep their current level. */
	const u8 value = inb(TX_PORT);

	if (drive)
		outb(level ? value | TX_MASK : value & ~TX_MASK, TX_PORT);
}

#if ENV_BOOTBLOCK
static void setup_tx_pin(void)
{
	pnp_enter_conf_state(GPIO_DEV);
	pnp_set_logical_device(GPIO_DEV);
	pnp_set_iobase(GPIO_DEV, PNP_IDX_IO1, SIMPLE_IO_BASE);
	pnp_unset_and_set_config(GPIO_DEV, TX_MUX_REG, 0, TX_MASK);
	pnp_exit_conf_state(GPIO_DEV);

	/* Drive idle-high from the moment the output is enabled. */
	ite_gpio_setup(GPIO_DEV, TX_GPIO, ITE_GPIO_INPUT, ITE_GPIO_SIMPLE_IO_MODE,
		       ITE_GPIO_CONTROL_DEFAULT);
	set_tx(1, true);
	ite_gpio_setup(GPIO_DEV, TX_GPIO, ITE_GPIO_OUTPUT, ITE_GPIO_SIMPLE_IO_MODE,
		       ITE_GPIO_CONTROL_DEFAULT);
}
#endif

void uart_init(unsigned int idx)
{
	/* At runtime the OS owns the Super I/O; SMM must not touch it. */
	if (ENV_SMM)
		return;

	ticks_per_bit = tsc_freq_mhz() * 1000000ULL / get_uart_baudrate();

#if ENV_BOOTBLOCK
	/* The Super I/O keeps the pin setup until power loss; bootblock runs on every boot. */
	setup_tx_pin();
	mdelay(1);
#endif
}

static void wait_until(uint64_t deadline)
{
	/* The inb() keeps the poll loop on the LPC bus instead of spinning. */
	while (rdtscll() < deadline)
		inb(TX_PORT);
}

static void __attribute__((noinline)) send_frame(unsigned char data, bool drive)
{
	const uint64_t tpb = drive ? ticks_per_bit : 0;
	const uint64_t t0 = rdtscll();
	int i;

	set_tx(0, drive);
	for (i = 0; i < 8; i++) {
		wait_until(t0 + (i + 1) * tpb);
		set_tx(data & (1 << i), drive);
	}
	wait_until(t0 + 9 * tpb);
	set_tx(1, drive);
	wait_until(t0 + 10 * tpb);
}

void uart_tx_byte(unsigned int idx, unsigned char data)
{
	if (ENV_SMM)
		return;

	/*
	 * In bootblock the code runs from flash, and a cache miss in the middle
	 * of a frame stretches a bit. Run the frame once without driving the pin
	 * so that its code is cached.
	 */
	send_frame(data, false);
	send_frame(data, true);
}

void uart_tx_flush(unsigned int idx)
{
}

unsigned char uart_rx_byte(unsigned int idx)
{
	return 0;
}

enum cb_err fill_lb_serial(struct lb_serial *serial)
{
	/* A payload cannot reuse a bit-banged transmitter. */
	return CB_ERR;
}
