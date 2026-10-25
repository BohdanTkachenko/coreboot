/* SPDX-License-Identifier: GPL-2.0-or-later */

CPPFLAGS_common += -I$(src)/mainboard/$(MAINBOARDDIR)/include

bootblock-y += bootblock.c

romstage-y += variants/$(VARIANT_DIR)/romstage.c

ramstage-y += ramstage.c
ramstage-y += variants/$(VARIANT_DIR)/gpio.c
ramstage-$(CONFIG_BOARD_CWWK_CW_NAS_ADLN_K) += nas_k_fans.c
ramstage-$(CONFIG_BOARD_CWWK_CW_NAS_ADLN_K) += nas_k_pcie_slot.c

all-$(CONFIG_CWWK_SIO_GPIO_UART) += sio_gpio_uart.c
# Provide inert UART entry points for DEBUG_SMI; the OS owns these GPIOs.
smm-$(CONFIG_CWWK_SIO_GPIO_UART) += sio_gpio_uart.c
