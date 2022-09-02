# Copyright (c) 2021 Casper Meijn <casper@meijn.net
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_GPIO)
  zephyr_library_named(pinetime_devkit0)
  zephyr_library_sources(key_out.c)
endif()
