# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(socks)

zephyr_library_sources_ifdef(CONFIG_SOCKS socks.c)
