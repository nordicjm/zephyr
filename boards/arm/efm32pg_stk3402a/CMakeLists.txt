# SPDX-License-Identifier: Apache-2.0

if(CONFIG_UART_GECKO)
  zephyr_library_named(efm32pg_stk3402a)
  zephyr_library_sources(board.c)
endif()
