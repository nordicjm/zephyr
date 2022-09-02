# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(emul)

zephyr_library_sources_ifdef(CONFIG_EMUL		emul.c)

zephyr_include_directories_ifdef(CONFIG_EMUL_BMI160 ${ZEPHYR_BASE}/drivers/sensor/bmi160)
zephyr_library_sources_ifdef(CONFIG_EMUL_BMI160		emul_bmi160.c)

add_subdirectory(i2c)
add_subdirectory(espi)
