# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(ptp_clock)

zephyr_library_sources_ifdef(CONFIG_PTP_CLOCK ptp_clock.c)
