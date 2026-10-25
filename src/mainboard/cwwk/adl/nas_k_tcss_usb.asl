/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * ADL-N TCSS xHCI wired to Type-A on CW-NAS-ADLN-K. No USB4 DMA/PCIe
 * topology or D3cold policy is present on this board. Keep the upstream
 * TCSS xHCI PCI PME methods, CPU wake bookkeeping and IOM display-off bit.
 * USB port _UPC/_PLD are supplied by drivers/usb/acpi in the generated SSDT.
 */
#include <soc/iomap.h>
#if CONFIG(D3COLD_SUPPORT)
#error This Type-A-only TCSS description does not implement D3cold
#endif
#define MCHBAR_TCSS_DEVEN_OFFSET 0x7090
Scope (\_SB)
{
	#include <soc/intel/alderlake/acpi/tcss_cpu_wake.asl>
}
Scope (\_SB.PCI0)
{
	OperationRegion (TDEN, SystemMemory, (GMHB() + MCHBAR_TCSS_DEVEN_OFFSET), 0x4)
	Field (TDEN, ByteAcc, NoLock, Preserve)
	{
		,     8,
		THCE, 1,  /* XHCI_EN */
		,     23
	}

	#include <soc/intel/alderlake/acpi/tcss_iom.asl>

	Device (TXHC)
	{
		Name (_ADR, 0x000D0000)
		Name (_DDN, "North XHCI controller")
		Name (_STR, Unicode ("North XHCI controller"))
		Name (DCPM, 4)
		Method (_STA, 0, NotSerialized)
		{
			If (THCE == 1) { Return (0x0F) }
			Return (0)
		}
		#include <soc/intel/alderlake/acpi/tcss_xhci_methods.asl>

		Device (RHUB)
		{
			Name (_ADR, 0)
			Device (HS01) { Name (_ADR, 1) }
			Device (SS01) { Name (_ADR, 2) }
			Device (SS02) { Name (_ADR, 3) }
		}
	}
}
