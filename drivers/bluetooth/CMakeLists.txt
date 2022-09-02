# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(bluetooth)
zephyr_library_include_directories(${ZEPHYR_BASE}/subsys/bluetooth)
add_subdirectory(hci)
