# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(lldp)
zephyr_library_include_directories(. ${ZEPHYR_BASE}/subsys/net/ip)

zephyr_library_sources(lldp.c)
