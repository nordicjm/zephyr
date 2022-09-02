# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(.)

zephyr_library_named(conn_mgr)
zephyr_library_sources(
	conn_mgr.c
	events_handler.c
	)
