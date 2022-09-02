#
# Copyright (c) 2021 metraTec GmbH
#
# SPDX-License-Identifier: Apache-2.0
#
zephyr_library_named(lpc51u68)

zephyr_library_sources(soc.c)

zephyr_library_include_directories(
  ${ZEPHYR_BASE}/kernel/include
  ${ZEPHYR_BASE}/arch/${ARCH}/include
  )
