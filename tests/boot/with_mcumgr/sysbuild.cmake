# Add the mcuboot key file to the secondary swapped app
# This must be done here to ensure that the same key file is used for signing
# both the primary and secondary apps
set(smp_svr_CONFIG_MCUBOOT_SIGNATURE_KEY_FILE
    \"${SB_CONFIG_BOOT_SIGNATURE_KEY_FILE}\" CACHE STRING
    "Signature key file for signing" FORCE)

# Add the swapped app to the build
ExternalZephyrProject_Add(
  APPLICATION smp_svr
  SOURCE_DIR ${ZEPHYR_BASE}/samples/subsys/mgmt/mcumgr/smp_svr
)

# Do not add the swapped app to the list of images to flash,
# just invoke the CMake configure step to build the secondary app
ExternalZephyrProject_Cmake(APPLICATION "smp_svr")
