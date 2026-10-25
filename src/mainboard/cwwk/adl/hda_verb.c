/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <device/azalia_device.h>

#if CONFIG(BOARD_CWWK_CW_NAS_ADLN_K)

/*
 * The NAS-K has one rear headphone/microphone combo jack and no audio header.
 * Its two signal paths were verified on ALC897 pins 0x14 and 0x18. Retain the
 * vendor's NO_PRESENCE policy for these pins, but describe their rear location.
 * Disable the reset-default connections that are absent on this board. Leave
 * the already-disconnected pins and Realtek's SKU pin 0x1d unchanged.
 */
static const u32 alc897_verbs[] = {
	AZALIA_PIN_CFG(0, 0x14, 0x01214110), /* Rear headphones */
	AZALIA_PIN_CFG(0, 0x15, AZALIA_PIN_CFG_NC(0)),
	AZALIA_PIN_CFG(0, 0x16, AZALIA_PIN_CFG_NC(0)),
	AZALIA_PIN_CFG(0, 0x17, AZALIA_PIN_CFG_NC(0)),
	AZALIA_PIN_CFG(0, 0x18, 0x01a19130), /* Rear microphone */
	AZALIA_PIN_CFG(0, 0x19, AZALIA_PIN_CFG_NC(0)),
	AZALIA_PIN_CFG(0, 0x1a, AZALIA_PIN_CFG_NC(0)),
	AZALIA_PIN_CFG(0, 0x1b, AZALIA_PIN_CFG_NC(0)),
	AZALIA_PIN_CFG(0, 0x1e, AZALIA_PIN_CFG_NC(0)),
};

const u32 pc_beep_verbs[] = {};

static struct azalia_codec mainboard_azalia_codecs[] = {
	{
		.name = "Realtek ALC897 (rear combo jack)",
		.vendor_id = 0x10ec0897,
		.subsystem_id = 0x10ec0897,
		.address = 0,
		.verbs = alc897_verbs,
		.verb_count = ARRAY_SIZE(alc897_verbs),
	},
};

AZALIA_ARRAY_SIZES;

#endif
