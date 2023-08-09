# Copyright (c) 2023 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

# List of default signing and encryption keys that MCUboot provides.
set(mcuboot_default_keys "enc-ec256-priv.pem;root-ec-p256.pem;enc-ec256-pub.pem;root-ec-p256-pkcs8.pem;enc-rsa2048-priv.pem;root-ed25519.pem;enc-rsa2048-pub.pem;root-rsa-2048.pem;enc-x25519-priv.pem;root-rsa-3072.pem;enc-x25519-pub.pem")

if(DEFINED CONFIG_SB)
  set(mcuboot_signature_key_file ${})
  set(mcuboot_encryption_key_file ${})
elseif(DEFINED CONFIG_MCUBOOT)
  set(mcuboot_signature_key_file ${CONFIG_BOOT_SIGNATURE_KEY_FILE})
  set(mcuboot_encryption_key_file ${})
else()
  set(mcuboot_signature_key_file ${CONFIG_MCUBOOT_SIGNATURE_KEY_FILE})
  set(mcuboot_encryption_key_file ${CONFIG_BOOT_ENCRYPTION_KEY_FILE})
endif()

message(WARNING "${CONFIG_BOOT_SIGNATURE_KEY_FILE}")

if(DEFINED mcuboot_signature_key_file)
  # Extract signing key path and filename from Kconfig.
  cmake_path(GET mcuboot_signature_key_file PARENT_PATH signing_key_dir)
  cmake_path(GET mcuboot_signature_key_file FILENAME signing_key_name)

  # Check if the path matches the MCUboot module path and if so, the filename is one of the
  # default. This is not foolproof but prevents an error being shown if someone has forked MCUboot
  # and created their own key in their forked MCUboot repository.
  if("${ZEPHYR_MCUBOOT_MODULE_DIR}" STREQUAL "${signing_key_dir}")
    list(FIND mcuboot_default_keys "${signing_key_name}" signing_key_default)
    if(NOT "${signing_key_default}" EQUAL "-1")
      message(WARNING
              "You are using a default MCUboot signing key, this is not a secure configuration. "
              "Please see the MCUboot documentation page on imgtool for details on creating a "
              "unique and secure signing key: https://docs.mcuboot.com/imgtool.html"
      )
    endif()
  endif()

  # Clean up
  set(signing_key_dir)
  set(signing_key_name)
endif()

if(DEFINED mcuboot_encryption_key_file)
  # Extract encryption key path and filename from Kconfig.
  cmake_path(GET mcuboot_encryption_key_file PARENT_PATH encryption_key_dir)
  cmake_path(GET mcuboot_encryption_key_file FILENAME encryption_key_name)

  # Check if the path matches the MCUboot module path and if so, the filename is one of the
  # default. This is not foolproof but prevents an error being shown if someone has forked MCUboot
  # and created their own key in their forked MCUboot repository.
  if("${ZEPHYR_MCUBOOT_MODULE_DIR}" STREQUAL "${encryption_key_dir}")
    list(FIND mcuboot_default_keys "${encryption_key_name}" encryption_key_default)
    if(NOT "${encryption_key_default}" EQUAL "-1")
      message(WARNING
              "You are using a default MCUboot encryption key, this is not a secure configuration. "
              "Please see the MCUboot documentation page on imgtool for details on creating a "
              "unique and secure encryption key: https://docs.mcuboot.com/imgtool.html"
      )
    endif()
  endif()

  # Clean up
  set(encryption_key_dir)
  set(encryption_key_name)
endif()

# Clean up
set(mcuboot_signature_key_file)
set(mcuboot_encryption_key_file)
