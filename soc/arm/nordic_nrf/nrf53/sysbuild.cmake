set(NRF53_SOC_INCLUDE_BLUETOOTH 0)

# Check if any images use Bluetooth
foreach(image ${IMAGES})
  set(CONFIG_BT_RPMSG)
  sysbuild_get(CONFIG_BT_RPMSG IMAGE ${image} KCONFIG)
  if(CONFIG_BT_RPMSG STREQUAL "y")
    set(NRF53_SOC_INCLUDE_BLUETOOTH 1)
  endif()
endforeach()

# Include hci_rpmsg for Bluetooth functionality, if used
if(NRF53_SOC_INCLUDE_BLUETOOTH EQUAL 1)
  ExternalZephyrProject_Add(
    APPLICATION hci_rpmsg
    SOURCE_DIR /tmp/aa/zephyr/samples/bluetooth/hci_rpmsg
    CORE radio
  )
  set(IMAGES ${IMAGES} "hci_rpmsg")
endif()
