# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(elkhart_lake)
zephyr_library_include_directories(${ZEPHYR_BASE}/drivers)

zephyr_cc_option(-march=goldmont)

zephyr_library_sources(cpu.c)
