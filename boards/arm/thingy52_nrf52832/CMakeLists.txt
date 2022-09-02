# SPDX-License-Identifier: Apache-2.0

if(CONFIG_CCS811)
zephyr_library_named(thingy52_nrf52832)
zephyr_library_sources(board.c)
endif()
