# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(tcpc)

zephyr_library_sources_ifdef(CONFIG_USBC_TCPC_STM32 ucpd_stm32.c)
