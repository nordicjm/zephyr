# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(net_virtual)
zephyr_library_include_directories(. ${ZEPHYR_BASE}/subsys/net/ip)

zephyr_library_sources_ifdef(CONFIG_NET_L2_VIRTUAL virtual.c)
zephyr_library_sources_ifdef(CONFIG_NET_L2_VIRTUAL_MGMT virtual_mgmt.c)

if(CONFIG_NET_L2_IPIP)
  add_subdirectory(ipip)
endif()
