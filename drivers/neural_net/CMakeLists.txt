# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(neural_net)

zephyr_library_sources_ifdef(CONFIG_INTEL_GNA		intel_gna.c)
