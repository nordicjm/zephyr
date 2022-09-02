# SPDX-License-Identifier: Apache-2.0

zephyr_linker_sources(
  RAM_SECTIONS
  SORT_KEY 0
  intvect_reserved.ld
)
zephyr_library_named(da1469x)
zephyr_library_sources(soc.c)
