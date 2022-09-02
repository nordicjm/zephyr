# SPDX-License-Identifier: Apache-2.0

if(CONFIG_BT_CTLR_DEBUG_PINS)
  zephyr_library_named(rv32m1_vega)
  zephyr_library_sources(board.c)
endif()
