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

# Global list of images enabled in this multi image build system, with internal variables used
# for holding a list of added images and directories for processing includes.
set(IMAGES)
set(ITERATE_IMAGES)
set_property(GLOBAL PROPERTY ITERATE_IMAGES ${ITERATE_IMAGES})
set(ITERATE_IMAGE_DIRS)
set_property(GLOBAL PROPERTY ITERATE_IMAGE_DIRS ${ITERATE_IMAGE_DIRS})

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

# Images must be processed in a nested loop to allow adding additional targets
get_property(ITERATE_IMAGES GLOBAL PROPERTY ITERATE_IMAGES)
get_property(ITERATE_IMAGE_DIRS GLOBAL PROPERTY ITERATE_IMAGE_DIRS)
list(LENGTH ITERATE_IMAGES ITERATE_IMAGE_SIZE)
while(ITERATE_IMAGE_SIZE GREATER 0)
  set(ITERATE_IMAGE_INDEX 0)

  # Use local loop variables to allow for appending to existing loops (which are cleared)
  set(ITERATE_IMAGE_LOOP ${ITERATE_IMAGES})
  set(ITERATE_IMAGES)
  set_property(GLOBAL PROPERTY ITERATE_IMAGES ${ITERATE_IMAGES})

  set(ITERATE_IMAGE_DIR_LOOP ${ITERATE_IMAGE_DIRS})
  set(ITERATE_IMAGE_DIRS)
  set_property(GLOBAL PROPERTY ITERATE_IMAGE_DIRS ${ITERATE_IMAGE_DIRS})

  foreach(IMAGE ${ITERATE_IMAGE_LOOP})
    # Include target sysbuild file, if it exists
    list(GET ITERATE_IMAGE_DIR_LOOP ${ITERATE_IMAGE_INDEX} IMAGE_DIR)
    include(${IMAGE_DIR}/sysbuild.cmake OPTIONAL)
    math(EXPR ITERATE_IMAGE_INDEX "${ITERATE_IMAGE_INDEX} + 1")
  endforeach()

  get_property(ITERATE_IMAGES GLOBAL PROPERTY ITERATE_IMAGES)
  get_property(ITERATE_IMAGE_DIRS GLOBAL PROPERTY ITERATE_IMAGE_DIRS)
  list(LENGTH ITERATE_IMAGES ITERATE_IMAGE_SIZE)
endwhile()

# TODO: deal with sysbuild Kconfig and sysbuild cmake
set_property(GLOBAL PROPERTY letpmrun "yes")

get_property(tmpvar GLOBAL PROPERTY somevar)
message(WARNING "finally it's ${tmpvar}")

message(WARNING "apps: ${IMAGES}")

#message(WARNING "list: ${ZEPHYR_SYSBUILD_MODULE_NAMES}")

include(cmake/domains.cmake)

message(WARNING "teh apps: ${IMAGES}")

set(VAR1)
set(VAR2)
set(VAR3)

foreach(image ${IMAGES})
message(WARNING "image: ${image}")

#include(${CMAKE_BINARY_DIR}/${image}/shared_vars.cmake)

set(blah1)
set(blah2)
sysbuild_get(blah1 IMAGE ${image} VAR PM_YML_DEP_FILES)
sysbuild_get(blah2 IMAGE ${image} VAR PM_YML_FILES)
#get_property(blah1 TARGET ${image}_shared_property_target PROPERTY PM_YML_DEP_FILES)
#get_property(blah2 TARGET ${image}_shared_property_target PROPERTY PM_YML_FILES)
list(APPEND VAR1 ${blah1})
list(APPEND VAR2 ${blah2})
if (DEFINED blah1)
#if (${blah1} STREQUAL "")
#else()
list(APPEND VAR3 ${image})
endif()

message(WARNING "1) ${blah1}, 2) ${blah2}")

endforeach()

message(WARNING "3) ${VAR1}, 4) ${VAR2}")

#set_property(GLOBAL PROPERTY PM_IMAGES ${IMAGES})
set_property(GLOBAL PROPERTY PM_IMAGES ${VAR3})
#set(CONFIG_FLASH_SIZE 1024)
set(CONFIG_FLASH_SIZE 512)
set(CONFIG_FLASH_BASE_ADDRESS 0x0)
#set(CONFIG_PM_SRAM_SIZE 0x40000)
set(CONFIG_PM_SRAM_SIZE 0x20000)
set(CONFIG_PM_SRAM_BASE 0x20000000)

set(NRF_DIR "/tmp/bb/nrf" CACHE PATH "NCS root directory")
#set_property(GLOBAL PROPERTY PM_SUBSYS_PREPROCESSED)
#set_property(GLOBAL PROPERTY PM_DOMAINS)


#if(ZEPHYR_NRF_MODULE_DIR)
#set(ZEPHYR_BINARY_DIR ${CMAKE_BINARY_DIR})
#  add_subdirectory(${ZEPHYR_NRF_MODULE_DIR} ${CMAKE_BINARY_DIR}/modules/nrf)
  include(${ZEPHYR_NRF_MODULE_DIR}/cmake/extensions.cmake)
  include(${ZEPHYR_NRF_MODULE_DIR}/cmake/partition_manager.cmake)
#endif()


#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

foreach(image ${IMAGES})
message(WARNING "test signing for image: ${image}")

#      --header-size $<TARGET_PROPERTY:partition_manager,PM_MCUBOOT_PAD_SIZE>
#sysbuild_get(mcuboot_FLASH_WRITE_BLOCK_SIZE IMAGE ${image} VAR CONFIG_MCUBOOT_FLASH_WRITE_BLOCK_SIZE)
#sysbuild_get(mcuboot_IMAGE_VERSION IMAGE ${image} VAR CONFIG_MCUBOOT_IMAGE_VERSION)
set(mcuboot_FLASH_WRITE_BLOCK_SIZE 4)
set(mcuboot_IMAGE_VERSION 0.0.0)
    set(sign_cmd
      ${PYTHON_EXECUTABLE}
      ${ZEPHYR_MCUBOOT_MODULE_DIR}/scripts/imgtool.py
      sign
      --key ${SB_CONFIG_BOOT_SIGNATURE_KEY_FILE}
      --header-size 0x200
      --align       ${mcuboot_FLASH_WRITE_BLOCK_SIZE}
      --version     ${mcuboot_IMAGE_VERSION}
      --pad-header
      )

set(TEST $<TARGET_PROPERTY:partition_manager,PM_MCUBOOT_PRIMARY_SIZE>)
    add_custom_command(
      OUTPUT
      ${image}/zephyr/zephyr_signed.hex            # Signed hex of input hex.

      COMMAND
      # Create signed hex file from input hex file.
      # This does not have the IMAGE_MAGIC at the end. So for this hex file
      # to be applied by mcuboot, the application is required to write the
      # IMAGE_MAGIC into the image trailer.
      ${sign_cmd}
#      ${sign_dependencies}
     --slot-size ${TEST}
#${SIGN_ARG_SLOT_SIZE}
#      ${SIGN_ARG_SIGNED_BIN_FILE_IN}
      ${image}/zephyr/zephyr.hex
      ${image}/zephyr/zephyr_signed.hex

      DEPENDS
      ${image}/zephyr/zephyr.hex
        )

#./mcuboot/zephyr/zephyr.hex
#./smp_svr/zephyr/zephyr.hex
#mcuboot_PM_HEX_FILE=/tmp/bb/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/_AA/mcuboot/zephyr/zephyr.hex

    add_custom_target(${image}_sign_target
      DEPENDS ${${image}_PM_HEX_FILE} ${image}/zephyr/zephyr_signed.hex
#      DEPENDS ${image}/zephyr/zephyr.hex ${image}/zephyr/zephyr_signed.hex
      )
add_dependencies(${image}_sign_target ${image})

#${app_signed_hex} ${app_signed_bin} ${app_signed_test_hex}

#  set_property(GLOBAL APPEND PROPERTY extra_post_build_commands COMMAND
#          ${sign_cmd} ${image}/zephyr/zephyr.hex ${image}/zephyr/zephyr_signed.hex)

endforeach()
