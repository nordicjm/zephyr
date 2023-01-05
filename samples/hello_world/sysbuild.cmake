  ExternalZephyrProject_Add(
    APPLICATION blinky
    SOURCE_DIR /tmp/aa/zephyr/samples/basic/blinky
    BOARD nrf5340dk_nrf5340_cpunet
  )
  # MCUBoot default configuration is to perform a full chip erase.
  # Placing MCUBoot first in list to ensure it is flashed before other images.
  set(IMAGES "blinky" ${IMAGES})

  ExternalZephyrProject_Add(
    APPLICATION mcuboot2
    SOURCE_DIR ${ZEPHYR_MCUBOOT_MODULE_DIR}/boot/zephyr/
    BOARD nrf5340dk_nrf5340_cpunet
  )
  # MCUBoot default configuration is to perform a full chip erase.
  # Placing MCUBoot first in list to ensure it is flashed before other images.
  set(IMAGES "mcuboot2" ${IMAGES})
