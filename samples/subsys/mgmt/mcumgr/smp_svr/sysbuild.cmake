#if(SB_CONFIG_BOOTLOADER_MCUBOOT)
  ExternalZephyrProject_Add(
    APPLICATION hello_world
    SOURCE_DIR /tmp/aa/zephyr/samples/hello_world
    BOARD nrf5340dk_nrf5340_cpuapp_ns
  )
  # MCUBoot default configuration is to perform a full chip erase.
  # Placing MCUBoot first in list to ensure it is flashed before other images.
  set(IMAGES ${IMAGES} "hello_world")


  ExternalZephyrProject_Add(
    APPLICATION hello_world_net
    SOURCE_DIR /tmp/aa/zephyr/samples/hello_world
    BOARD nrf5340dk_nrf5340_cpunet
  )
  # MCUBoot default configuration is to perform a full chip erase.
  # Placing MCUBoot first in list to ensure it is flashed before other images.
  set(IMAGES ${IMAGES} "hello_world_net")

  ExternalZephyrProject_Add(
    APPLICATION mcuboot_other
    SOURCE_DIR ${ZEPHYR_MCUBOOT_MODULE_DIR}/boot/zephyr/
    BOARD nrf5340dk_nrf5340_cpuapp_ns
  )
  # MCUBoot default configuration is to perform a full chip erase.
  # Placing MCUBoot first in list to ensure it is flashed before other images.
  set(IMAGES "mcuboot_other" ${IMAGES})
#endif()
