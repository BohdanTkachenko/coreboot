/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "tcss_xhci_methods.asl"

/*
 * xHCI Root Hub Device
 */
Device (RHUB)
{
	Name (_ADR, 0)

	/* High Speed Ports */
	Device (HS01)
	{
		Name (_ADR, 0x01)
	}

	/* Super Speed Ports */
	Device (SS01)
	{
		Name (_ADR, 0x02)
		Method (_DSD, 0, NotSerialized)
		{
			Return( Package ()
			{
				ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
				Package ()
				{
					Package (2) { "usb4-host-interface", \_SB.PCI0.TDM0 },
					Package (2) { "usb4-port-number", 0 }
				}
			})
		}
	}

	Device (SS02)
	{
		Name (_ADR, 0x03)
		Method (_DSD, 0, NotSerialized)
		{
			Return( Package ()
			{
				ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
				Package ()
				{
					Package (2) { "usb4-host-interface", \_SB.PCI0.TDM0 },
					Package (2) { "usb4-port-number", 1 }
				}
			})
		}
	}

	Device (SS03)
	{
		Name (_ADR, 0x04)
		Method (_DSD, 0, NotSerialized)
		{
			Return( Package ()
			{
				ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
				Package ()
				{
					Package (2) { "usb4-host-interface", \_SB.PCI0.TDM1 },
					Package (2) { "usb4-port-number", 0 }
				}
			})
		}
	}

	Device (SS04)
	{
		Name (_ADR, 0x05)
		Method (_DSD, 0, NotSerialized)
		{
			Return( Package ()
			{
				ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
				Package ()
				{
					Package (2) { "usb4-host-interface", \_SB.PCI0.TDM1 },
					Package (2) { "usb4-port-number", 1 }
				}
			})
		}
	}
}
