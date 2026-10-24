/* SPDX-License-Identifier: GPL-2.0-or-later */

/* Included in Scope (\_SB.PCI0); also used by Type-A-only TCSS wiring. */
/*
 * Operation region defined to access the IOM REGBAR. Get the MCHBAR in offset
 * 0x48 in B0:D0:F0. REGBAR Base address is in offset 0x7110 of MCHBAR.
 */
OperationRegion (MBAR, SystemMemory, (GMHB() + 0x7100), 0x1000)
Field (MBAR, ByteAcc, NoLock, Preserve)
{
	Offset(0x10),
	RBAR, 64        /* RegBar, offset 0x7110 in MCHBAR */
}

/*
 * IOM REG BAR Base address is in offset 0x7110 in MCHBAR.
 */
Method (IOMA, 0)
{
	Return (^RBAR & ~0x1)
}

/*
 * From RegBar Base, IOM_TypeC_SW_configuration_1 is in offset 0xC10040, where
 * 0x40 is the register offset.
 */
OperationRegion (IOMR, SystemMemory, (IOMA() + 0xC10000), 0x100)
Field (IOMR, DWordAcc, NoLock, Preserve)
{
	Offset(0x40),
	,     15,
	TD3C, 1,          /* [15:15] Type C D3 cold bit */
	TACK, 1,          /* [16:16] IOM Acknowledge bit */
	DPOF, 1,          /* [17:17] Set 1 to indicate IOM, all the */
			  /* display is OFF, clear otherwise */
	Offset(0x70),     /* Physical addr is offset 0x70. */
	IMCD, 32,         /* R_SA_IOM_BIOS_MAIL_BOX_CMD */
	IMDA, 32          /* R_SA_IOM_BIOS_MAIL_BOX_DATA */
}
