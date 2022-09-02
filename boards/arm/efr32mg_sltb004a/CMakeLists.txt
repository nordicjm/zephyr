# Copyright (c) 2020 Christian Taedcke
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_CCS811)
  zephyr_library_named(efr32mg_sltb004a)
  zephyr_library_sources(board.c)
endif()
