# Copyright (c) 2021 Nordic Semiconductor
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED APP_DIR)
  message(FATAL_ERROR "No main application specified")
endif()

# This will update the APP_DIR cache variable to PATH type and apply a comment.
# If APP_DIR is a relative path, then CMake will adjust to absolute path based
# on current working dir.
set(APP_DIR ${APP_DIR} CACHE PATH "Main Application Source Directory")

# Add sysbuild/cmake/modules to CMAKE_MODULE_PATH which allows us to integrate
# sysbuild CMake modules with general Zephyr CMake modules.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/modules)
# List of Zephyr and sysbuild CMake modules we need for sysbuild.
# Note: sysbuild_kconfig will internally load kconfig CMake module.
set(zephyr_modules extensions sysbuild_extensions python west root zephyr_module boards shields sysbuild_kconfig)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE} COMPONENTS ${zephyr_modules})

project(sysbuild LANGUAGES)

# Global list of images enabled in this multi image build system.
set(IMAGES)

get_filename_component(APP_DIR  ${APP_DIR} ABSOLUTE)
get_filename_component(app_name ${APP_DIR} NAME)




# Call sysbuild modules for each target image
  # Include zephyr modules generated CMake file.
set(SYSBUILD_MODULE_DIRS "")
set(SYSBUILD_CMAKE_DIRS "")
  foreach(module_name ${ZEPHYR_SYSBUILD_MODULE_NAMES})
    # Note the second, binary_dir parameter requires the added
    # subdirectory to have its own, local cmake target(s). If not then
    # this binary_dir is created but stays empty. Object files land in
    # the main binary dir instead.
    # https://cmake.org/pipermail/cmake/2019-June/069547.html
    zephyr_string(SANITIZE TOUPPER MODULE_NAME_UPPER ${module_name})
    if(NOT ${ZEPHYR_SYSBUILD_${MODULE_NAME_UPPER}_CMAKE_DIR} STREQUAL "")
list(APPEND SYSBUILD_MODULE_DIRS ${ZEPHYR_SYSBUILD_${MODULE_NAME_UPPER}_MODULE_DIR})
list(APPEND SYSBUILD_CMAKE_DIRS ${ZEPHYR_SYSBUILD_${MODULE_NAME_UPPER}_CMAKE_DIR})
#      set(ZEPHYR_CURRENT_MODULE_DIR ${ZEPHYR_SYSBUILD_${MODULE_NAME_UPPER}_MODULE_DIR})
#      set(ZEPHYR_CURRENT_CMAKE_DIR ${ZEPHYR_SYSBUILD_${MODULE_NAME_UPPER}_CMAKE_DIR})
#      add_subdirectory(${ZEPHYR_CURRENT_CMAKE_DIR} ${CMAKE_BINARY_DIR}/${image}/sysbuild_modules/${module_name})
    endif()
  endforeach()

message(WARNING "it is ${SYSBUILD_MODULE_DIRS} und ${SYSBUILD_CMAKE_DIRS}")


# Propagate bootloader and signing settings from this system to the MCUboot and
# application image build systems.
if(SB_CONFIG_BOOTLOADER_MCUBOOT)
  set(${app_name}_CONFIG_BOOTLOADER_MCUBOOT y CACHE STRING
      "MCUBOOT is enabled as bootloader" FORCE
  )
  set(${app_name}_CONFIG_MCUBOOT_SIGNATURE_KEY_FILE
      \"${SB_CONFIG_BOOT_SIGNATURE_KEY_FILE}\" CACHE STRING
      "Signature key file for signing" FORCE
  )

  # Set corresponding values in mcuboot
  set(mcuboot_CONFIG_BOOT_SIGNATURE_TYPE_${SB_CONFIG_SIGNATURE_TYPE} y CACHE STRING
      "MCUBOOT signature type" FORCE
  )
  set(mcuboot_CONFIG_BOOT_SIGNATURE_KEY_FILE
      \"${SB_CONFIG_BOOT_SIGNATURE_KEY_FILE}\" CACHE STRING
      "Signature key file for signing" FORCE
  )
else()
  set(${app_name}_CONFIG_BOOTLOADER_MCUBOOT n CACHE STRING
      "MCUBOOT is disabled as bootloader" FORCE
  )
endif()

# This adds the primary application to the build.
ExternalZephyrProject_Add(
  APPLICATION ${app_name}
  SOURCE_DIR ${APP_DIR}
  MAIN_APP
SYSBUILD_MODULE_DIRS ${SYSBUILD_MODULE_DIRS}
SYSBUILD_CMAKE_DIRS ${SYSBUILD_CMAKE_DIRS}
)
list(APPEND IMAGES "${app_name}")
set(DEFAULT_IMAGE "${app_name}")

add_subdirectory(bootloader)

# This allows for board and app specific images to be included.
include(${BOARD_DIR}/sysbuild.cmake OPTIONAL)
include(${APP_DIR}/sysbuild.cmake OPTIONAL)

# TODO: deal with sysbuild Kconfig and sysbuild cmake
set_property(GLOBAL PROPERTY letpmrun "yes")

get_property(tmpvar GLOBAL PROPERTY somevar)
message(WARNING "finally it's ${tmpvar}")

message(WARNING "apps: ${IMAGES}")

#message(WARNING "list: ${ZEPHYR_SYSBUILD_MODULE_NAMES}")

include(cmake/domains.cmake)

message(WARNING "teh apps: ${IMAGES}")

foreach(image ${IMAGES})
message(WARNING "image: ${image}")

endforeach()


#if(ZEPHYR_NRF_MODULE_DIR)
#set(ZEPHYR_BINARY_DIR ${CMAKE_BINARY_DIR})
#  add_subdirectory(${ZEPHYR_NRF_MODULE_DIR} ${CMAKE_BINARY_DIR}/modules/nrf)
#  include(${ZEPHYR_NRF_MODULE_DIR}/cmake/extensions.cmake)
#  include(${ZEPHYR_NRF_MODULE_DIR}/cmake/partition_manager.cmake)
#endif()
