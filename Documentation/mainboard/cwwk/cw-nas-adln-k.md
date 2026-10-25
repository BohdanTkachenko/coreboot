# CWWK CW-NAS-ADLN-K

The CW-NAS-ADLN-K is a mini-ITX NAS motherboard with a soldered Intel Alder
Lake-N processor (N100 or i3-N305), one DDR5 SO-DIMM slot, two M.2 NVMe
sockets, an ASM1166 six-port SATA controller, two Intel I226-V 2.5 GbE
controllers, a PCIe slot, HDMI, DisplayPort and an ITE IT8613E Super I/O.

## Building

Select `CWWK` / `CW-NAS-ADLN-K` and add the flash descriptor and Intel ME
firmware from the vendor image (`HAVE_IFD_BIN`, `HAVE_ME_BIN`). The FSP and
VBT are the ones shared with the other CWWK Alder Lake-N boards; graphics are
initialized by the FSP GOP.

## Flashing

The 16 MiB Winbond W25Q128JV SPI flash (SOIC-8) is not socketed. It can be
written internally with flashrom while coreboot runs, or externally with a
programmer and a SOIC-8 clip; do not power the board while the clip is
attached.

The vendor firmware's runtime variable driver can rewrite the beginning of
the BIOS region after an update made from the vendor firmware. The flash
layout (`nas_k_ami_safe.fmd`) therefore leaves the first 384 KiB of the BIOS
region unused. A 512 KiB `SMMSTORE` region follows the FMAP and holds the
UEFI variables of payloads such as edk2 when `SMMSTORE` is enabled. When
updating, preserve the descriptor and ME regions and the `RW_MRC_CACHE` and
`SMMSTORE` areas, for example with `flashrom --fmap -i COREBOOT`.

## Hardware

| Feature | Connection |
| --- | --- |
| M.2 NVMe 1 / 2 | PCH root ports 1 and 2 |
| SATA | ASM1166 on root port 3 |
| Ethernet | I226-V on root ports 4 and 7 |
| PCIe slot | Root port 9, up to x4 |
| USB 3 | Rear blue port and one internal header channel via TCSS ports 0/1 converted to Type-A |
| Audio | Realtek ALC897 rear headphone/microphone combo jack, HDMI and DP audio |
| Super I/O | IT8613E: fan control, GPIO header; no COM header |
| TPM | Intel PTT (CRB) |

PCIe function swapping is disabled so that each root port keeps its native
PCI function. FSP's early link probe can miss the second NVMe drive and a card
in the PCIe slot; root ports 2 and 9 therefore have hot-plug enabled, and the
slot's link is released after FSP-S if FSP left it blocked in Detect.

The second channel of the internal USB 3 header is USB 2 only, as on the
vendor firmware.

## Fans

Before memory training CPU_FAN and SYS_FAN run at a fixed 63 % duty, since
the IT8613E keeps its fan settings across warm resets. In ramstage the fans
follow the vendor's automatic curves: CPU_FAN on the CPU temperature read over
PECI, SYS_FAN on TMPIN2. The vendor's TMPIN2 source and offset are kept; the
sensor's location and calibration are unknown. If the temperatures do not
read valid, both fans run at full speed.

SYS_FAN varies the voltage on its +12 V pin; its PWM pin stays at 5 V. Fans
powered from the header slow down, but 4-pin fans on a hub with its own power
run at full speed. SYS_FAN2 supplies a constant 12 V and leaves its PWM pin
unconnected, so its fans always run at full speed.

## GPIO header

The 2.00 mm GPIO header has eight 3.3 V signals connected to the IT8613E
GPIOs, accessible in Simple I/O mode at I/O port 0xa00. Linux exposes them
with `gpio-it87` when the kernel enables `GPIO_IT87`.

| Pin | Signal | Pin | Signal |
| --- | --- | --- | --- |
| 1 | GP10 | 2 | GP22 |
| 3 | GP31 | 4 | GP32 |
| 5 | GP33 | 6 | GP41 |
| 7 | GP45 | 8 | GP63 |
| 9 | GND | 10 | 5 V |

## Status

Working:

- Both M.2 NVMe sockets, all six SATA ports, both Ethernet ports, the PCIe slot
- USB 2 and USB 3 ports, SD card reader
- HDMI and DisplayPort including audio, rear audio jack playback and recording
- Fan control, GPIO header, front-panel buttons and LEDs
- ACPI S3, wake from RTC, USB keyboard, power button and LAN
- Intel PTT and measured boot

Unknown or not working:

- The TCO watchdog's second-stage timeout does not reset the board.
- eMMC: an unpopulated footprint may be for eMMC; the controller is disabled.
