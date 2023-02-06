set(NRF53_SOC_INCLUDE_BLUETOOTH 0)

# Check if any images use Bluetooth
foreach(image ${IMAGES})
  set(CONFIG_BT)
  sysbuild_get(CONFIG_BT IMAGE ${image} KCONFIG)
  if(CONFIG_BT STREQUAL "y")
    set(NRF53_SOC_INCLUDE_BLUETOOTH 1)
  endif()
endforeach()

# Include hci_rpmsg for Bluetooth functionality, if used
# TODO: better for v2 would be to use same board name for cpuapp, cpuapp_ns and cpunet and somehow specify which one to use via another way
if(NRF53_SOC_INCLUDE_BLUETOOTH EQUAL 1)
  ExternalZephyrProject_Add(
    APPLICATION hci_rpmsg
    SOURCE_DIR /tmp/aa/zephyr/samples/bluetooth/hci_rpmsg
BOARD nrf5340dk_nrf5340_cpunet
  )
  set(IMAGES ${IMAGES} "hci_rpmsg")
endif()
