# SPDX-License-Identifier: Apache-2.0

if(CONFIG_UART_GECKO)
  zephyr_library_named(efr32_radio)
  zephyr_library_sources(board.c)
endif()
