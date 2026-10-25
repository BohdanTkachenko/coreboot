/* SPDX-License-Identifier: GPL-2.0-or-later */

/*
 * CW-NAS-ADLN-K PCIe x1 slot = PCH root port 9 (00:1d.0). On some boots FSP-S probes the slot
 * before the card answers, gives up and leaves PCIEALC.BLKDQDA set: the LTSSM is then held in
 * Detect for good, although hot-plug keeps the port visible. Release it once FSP is done and give
 * the card a moment to link, so that coreboot and the OS enumerate it.
 */

#include <bootstate.h>
#include <console/console.h>
#include <delay.h>
#include <device/pci.h>
#include <device/pci_def.h>
#include <device/pci_ops.h>
#include <timer.h>

#define PCIEALC			0x338
#define  PCIEALC_BLKDQDA	(1 << 26)	/* block Detect.Quiet -> Detect.Active */
#define PCI_EXP_LNKCTL2		0x30
#define SLOT_LINK_WAIT_MS	500

static void nas_k_slot_release(void *unused)
{
	const struct device *dev = pcidev_on_root(0x1d, 0);
	struct stopwatch sw;
	u32 alc;
	u16 cap;

	if (!is_dev_enabled(dev))
		return;
	alc = pci_read_config32(dev, PCIEALC);
	cap = pci_find_capability(dev, PCI_CAP_ID_PCIE);
	if (!(alc & PCIEALC_BLKDQDA) || !cap)
		return;

	/* FSP also drops the target speed of a port it found empty; restore the port maximum. */
	pci_update_config16(dev, cap + PCI_EXP_LNKCTL2, ~0xf,
			    pci_read_config32(dev, cap + PCI_EXP_LNKCAP) & PCI_EXP_LNKCAP_MLS);
	pci_write_config32(dev, PCIEALC, alc & ~PCIEALC_BLKDQDA);

	stopwatch_init_msecs_expire(&sw, SLOT_LINK_WAIT_MS);
	while (!(pci_read_config16(dev, cap + PCI_EXP_LNKSTA) & PCI_EXP_LNKSTA_DLLLA)) {
		if (stopwatch_expired(&sw)) {
			printk(BIOS_INFO, "CW-NAS-ADLN-K: PCIe slot: no link %u ms after "
			       "releasing Detect (slot empty?)\n", SLOT_LINK_WAIT_MS);
			return;
		}
		mdelay(1);
	}
	printk(BIOS_INFO, "CW-NAS-ADLN-K: PCIe slot: FSP left Detect blocked; linked %ld ms "
	       "after release\n", (long)stopwatch_duration_msecs(&sw));
}

/* After FSP-S (chip init), before the PCI scan. */
BOOT_STATE_INIT_ENTRY(BS_DEV_ENUMERATE, BS_ON_ENTRY, nas_k_slot_release, NULL);
