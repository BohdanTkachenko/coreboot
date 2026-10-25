/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <device/pnp_ops.h>
#include <fsp/api.h>
#include <soc/meminit.h>
#include <soc/romstage.h>
#include <superio/hwm5_conf.h>
#include <superio/ite/common/ite.h>
#include <superio/ite/it8613e/it8613e.h>

#define EC_DEV		PNP_DEV(0x2e, IT8613E_EC)
#define EC_BASE		0x0a30	/* Same as the devicetree */

/*
 * The IT8613E keeps its fan settings across warm resets, which can mean
 * stopped fans, or automatic mode working from invalid readings while the
 * CPU is in reset. Run both fans at a fixed duty through memory training;
 * ramstage programs the fan curves.
 */
static void early_fans_safe(void)
{
	pnp_enter_conf_state(EC_DEV);
	pnp_set_logical_device(EC_DEV);
	pnp_set_iobase(EC_DEV, PNP_IDX_IO0, EC_BASE);
	pnp_set_enable(EC_DEV, 1);
	pnp_exit_conf_state(EC_DEV);

	pnp_unset_and_set_hwm5_index(EC_BASE, 0x06, 0x60, 0);	/* bank 0 */
	pnp_write_hwm5_index(EC_BASE, 0x6b, 0xa0);		/* FAN2 (CPU_FAN) duty 160/255 */
	pnp_write_hwm5_index(EC_BASE, 0x73, 0xa0);		/* FAN3 (SYS_FAN) duty 160/255 */
	pnp_write_hwm5_index(EC_BASE, 0x16, 0x00);		/* software mode */
	pnp_write_hwm5_index(EC_BASE, 0x17, 0x00);
}

void mainboard_memory_init_params(FSPM_UPD *memupd)
{
	early_fans_safe();

	static const struct mb_cfg ddr5_mem_config = {
		.type = MEM_TYPE_DDR5,

		.ect = true, /* Early Command Training */

		.UserBd = BOARD_TYPE_MOBILE,

		.LpDdrDqDqsReTraining = 1,
	};

	const struct mem_spd dimm_module_spd_info = {
		.topo = MEM_TOPO_DIMM_MODULE,
		.smbus = {
			[0] = { .addr_dimm[0] = 0x50, },
		},
	};

	const bool half_populated = false;
	memcfg_init(memupd, &ddr5_mem_config, &dimm_module_spd_info, half_populated);
}
