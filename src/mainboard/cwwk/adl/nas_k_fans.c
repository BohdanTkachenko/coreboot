/* SPDX-License-Identifier: GPL-2.0-or-later */

/*
 * The fan curves come from the devicetree (superio/ite/common/env_ctrl.c).
 * This adds what the generic driver does not do: route TMPIN2 to the source
 * the vendor firmware uses, start the smoothed automatic output at the
 * curve value, and run both fans at full speed if the temperature readings
 * never become valid, since the curves would then leave the fans at their
 * start duty.
 */

#include <bootstate.h>
#include <console/console.h>
#include <delay.h>
#include <device/device.h>
#include <device/pnp.h>
#include <superio/hwm5_conf.h>
#include <superio/ite/common/env_ctrl.h>
#include <superio/ite/it8613e/it8613e.h>

#define TMPIN_READING(x)	(0x28 + (x))
#define TMPIN2_VENDOR_SOURCE	1

static bool temp_valid(u8 t)
{
	/* 0x80 means no reading; anything outside 1-126 C is implausible here. */
	return t > 0 && t < 127;
}

/*
 * With smoothing, the automatic PWM output only ramps slowly towards the curve.
 * After S3 it starts near zero and CPU_FAN stays stalled for minutes. Without
 * smoothing the output jumps to the curve value, so briefly turn it off.
 */
static void start_ramps(u16 base)
{
	unsigned int fan;
	u8 reg;

	for (fan = 2; fan <= 3; fan++) {
		reg = pnp_read_hwm5_index(base, ITE_EC_FAN_CTL_PWM_AUTO(fan));
		if (!(reg & ITE_EC_FAN_CTL_AUTO_SMOOTHING_EN))
			continue;
		pnp_write_hwm5_index(base, ITE_EC_FAN_CTL_PWM_AUTO(fan),
				     reg & ~ITE_EC_FAN_CTL_AUTO_SMOOTHING_EN);
		mdelay(20);
		pnp_write_hwm5_index(base, ITE_EC_FAN_CTL_PWM_AUTO(fan), reg);
	}
}

static void nas_k_fans_init(void *unused)
{
	const struct device *dev = dev_find_slot_pnp(0x2e, IT8613E_EC);
	const struct resource *res;
	u8 t1 = 0x80, t2 = 0x80;
	unsigned int i;
	u16 base;

	if (!is_dev_enabled(dev))
		return;
	res = probe_resource(dev, PNP_IDX_IO0);
	if (!res)
		return;
	base = res->base;

	pnp_unset_and_set_hwm5_index(base, ITE_EC_BANK_SELECT, ITE_EC_BANK_SELECT_MASK,
				     ITE_EC_BANK_SELECT_BANK(2));
	pnp_unset_and_set_hwm5_index(base, ITE_EC_TMPIN_SRC_SEL(2),
				     ITE_EC_TMPIN_SRC_SEL_MASK << ITE_EC_TMPIN_SRC_SEL_SHIFT(2),
				     TMPIN2_VENDOR_SOURCE << ITE_EC_TMPIN_SRC_SEL_SHIFT(2));
	pnp_unset_and_set_hwm5_index(base, ITE_EC_BANK_SELECT, ITE_EC_BANK_SELECT_MASK,
				     ITE_EC_BANK_SELECT_BANK(0));

	for (i = 0; i < 100; i++) {
		t1 = pnp_read_hwm5_index(base, TMPIN_READING(1));
		t2 = pnp_read_hwm5_index(base, TMPIN_READING(2));
		if (temp_valid(t1) && temp_valid(t2)) {
			start_ramps(base);
			printk(BIOS_INFO, "CW-NAS-ADLN-K: fans automatic (CPU %u C, TMPIN2 %u C)\n",
			       t1, t2);
			return;
		}
		mdelay(10);
	}

	for (i = 2; i <= 3; i++) {
		pnp_write_hwm5_index(base, ITE_EC_FAN_CTL_PWM_START(i), ITE_EC_FAN_MAX_PWM);
		pnp_write_hwm5_index(base, ITE_EC_FAN_CTL_PWM_CONTROL(i),
				     ITE_EC_FAN_CTL_PWM_MODE_SOFTWARE);
	}
	printk(BIOS_WARNING, "CW-NAS-ADLN-K: no valid temperature (TMPIN1 %#x, TMPIN2 %#x), "
	       "fans at full speed\n", t1, t2);
}

/* After the Super I/O device init has programmed the fan curves. */
BOOT_STATE_INIT_ENTRY(BS_DEV_INIT, BS_ON_EXIT, nas_k_fans_init, NULL);
