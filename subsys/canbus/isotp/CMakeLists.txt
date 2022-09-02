# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(canbus_isotp)
zephyr_library_sources_ifdef(CONFIG_ISOTP isotp.c)
