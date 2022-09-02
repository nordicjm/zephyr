# SPDX-License-Identifier: Apache-2.0

if(CONFIG_UART_GECKO)
  zephyr_library_named(efm32wg_stk3800)
  zephyr_library_sources(board.c)
endif()
