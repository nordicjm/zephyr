  ExternalZephyrProject_Add(
    APPLICATION mcuboot
    SOURCE_DIR ${ZEPHYR_MCUBOOT_MODULE_DIR}/boot/zephyr/
SYSBUILD_MODULE_DIRS ${SYSBUILD_MODULE_DIRS}
SYSBUILD_CMAKE_DIRS ${SYSBUILD_CMAKE_DIRS}
  )
  # MCUBoot default configuration is to perform a full chip erase.
  # Placing MCUBoot first in list to ensure it is flashed before other images.
  set(IMAGES "mcuboot" ${IMAGES})
