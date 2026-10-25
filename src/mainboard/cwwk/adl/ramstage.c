/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <mainboard/gpio.h>
#include <soc/ramstage.h>

static void mainboard_init(void *chip_info)
{
	mainboard_configure_gpios();
}

/*
 * CW-NAS-ADLN-K: keep the PCH PCIe root ports at their native function
 * numbers. With function swapping (FSP default) enabled, adding RP2 made
 * FSP cull RP1 and RP2 together (both NVMe ports vanished, along with the
 * SATA/NIC links). Disabling the swap keeps 1c.0=RP1 (NVMe0) and
 * 1c.1=RP2 (NVMe1) so both NVMe controllers enumerate.
 */
void mainboard_silicon_init_params(FSP_S_CONFIG *params)
{
	if (CONFIG(BOARD_CWWK_CW_NAS_ADLN_K))
		params->PcieRpFunctionSwap = 0;
}

struct chip_operations mainboard_ops = {
	.init = mainboard_init,
};
