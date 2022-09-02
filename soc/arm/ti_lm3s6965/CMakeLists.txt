# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(ti_lm3s6965)

zephyr_library_sources(
  soc.c
  soc_config.c
  reboot.S
  sys_arch_reboot.c
  )

zephyr_library_include_directories(
  ${ZEPHYR_BASE}/kernel/include
  ${ZEPHYR_BASE}/arch/arm/include
  )
