# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(ipip)
zephyr_library_include_directories(. ${ZEPHYR_BASE}/subsys/net/ip)

zephyr_library_sources_ifdef(CONFIG_NET_L2_IPIP ipip.c)
