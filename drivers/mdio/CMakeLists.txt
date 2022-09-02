# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(mdio)

zephyr_library_sources_ifdef(CONFIG_MDIO_SHELL		mdio_shell.c)
zephyr_library_sources_ifdef(CONFIG_MDIO_ATMEL_SAM	mdio_sam.c)
