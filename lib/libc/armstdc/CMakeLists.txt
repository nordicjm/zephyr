# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(armstdc)
zephyr_library_sources(src/errno.c)
zephyr_library_sources(src/string.c)
zephyr_library_sources(src/libc-hooks.c)
zephyr_library_sources_ifndef(CONFIG_MULTITHREADING src/threading_weak.c)

zephyr_system_include_directories(include)
