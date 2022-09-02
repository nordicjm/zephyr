# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(mbox)

zephyr_library_sources_ifdef(CONFIG_USERSPACE   mbox_handlers.c)
zephyr_library_sources_ifdef(CONFIG_MBOX_NRFX_IPC   mbox_nrfx_ipc.c)
