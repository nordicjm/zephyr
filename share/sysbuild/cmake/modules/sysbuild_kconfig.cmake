# Copyright (c) 2021 Nordic Semiconductor
#
# SPDX-License-Identifier: Apache-2.0

set(EXTRA_KCONFIG_TARGET_COMMAND_FOR_sysbuild_menuconfig
  ${ZEPHYR_BASE}/scripts/kconfig/menuconfig.py
)

set(EXTRA_KCONFIG_TARGET_COMMAND_FOR_sysbuild_guiconfig
  ${ZEPHYR_BASE}/scripts/kconfig/guiconfig.py
)

set(KCONFIG_TARGETS sysbuild_menuconfig sysbuild_guiconfig)
list(TRANSFORM EXTRA_KCONFIG_TARGETS PREPEND "sysbuild_")

if(DEFINED SB_CONF_FILE)
  # SB_CONF_FILE already set so nothing to do.
elseif(DEFINED ENV{SB_CONF_FILE})
  set(SB_CONF_FILE $ENV{SB_CONF_FILE})
elseif(EXISTS      ${APP_DIR}/sysbuild.conf)
  set(SB_CONF_FILE ${APP_DIR}/sysbuild.conf)
else()
  # Because SYSBuild is opt-in feature, then it is permitted to not have a
  # SYSBuild dedicated configuration file.
endif()

if(DEFINED SB_CONF_FILE AND NOT IS_ABSOLUTE SB_CONF_FILE)
  cmake_path(ABSOLUTE_PATH SB_CONF_FILE BASE_DIRECTORY ${APP_DIR} OUTPUT_VARIABLE SB_CONF_FILE)
endif()

if(DEFINED SB_CONF_FILE AND NOT DEFINED CACHE{SB_CONF_FILE})
  # We only want to set this in cache it has been defined and is not already there.
  set(SB_CONF_FILE ${SB_CONF_FILE} CACHE STRING "If desired, you can build the application with \
  SYSbuild configuration settings specified in an alternate .conf file using this parameter. \
  These settings will override the settings in the application’s SYSBuild config file or its \
  default .conf file. Multiple files may be listed, e.g. SB_CONF_FILE=\"sys1.conf sys2.conf\"")
endif()

if(NOT DEFINED SB_CONF_FILE)
  # If there is no SB_CONF_FILE, then use empty.conf to make kconfiglib happy.
  # Not adding it to CMake cache ensures that a later created sysbuild.conf
  # will be automatically detected.
  set(SB_CONF_FILE ${CMAKE_CURRENT_BINARY_DIR}/empty.conf)
endif()



function(sb_soc_port prefix kconfig_fragment new_prefix out_file)
  # Parse the lines prefixed with 'prefix' in ${kconfig_fragment}
  file(
    STRINGS
    ${kconfig_fragment}
    DOT_CONFIG_LIST
    REGEX "^${prefix}"
    ENCODING "UTF-8"
  )

file(WRITE ${out_file} "")

  foreach (CONFIG ${DOT_CONFIG_LIST})
    # CONFIG could look like: CONFIG_NET_BUF=y

    # Match the first part, the variable name
    string(REGEX MATCH "[^=]+" CONF_VARIABLE_NAME ${CONFIG})

    # Match the second part, variable value
#    string(REGEX MATCH "=(.+$)" CONF_VARIABLE_VALUE ${CONFIG})
    # The variable name match we just did included the '=' symbol. To just get the
    # part on the RHS we use match group 1
#    set(CONF_VARIABLE_VALUE ${CMAKE_MATCH_0})
string(SUBSTRING ${CMAKE_MATCH_0} 7 -1 VAR_VALUE)

#file(APPEND ${out_file} ${new_prefix}_${CONFIG}\n\tbool\n\tdefault\ y\n\n)
#file(APPEND ${out_file} config\ ${CONF_VARIABLE_VALUE}\n\tbool\n\tdefault\ y\n\n)

file(APPEND ${out_file} config\ ${VAR_VALUE}\n\tbool\n\tdefault\ y\n\n)
#file(APPEND ${out_file} config\ ${VAR_VALUE}\n\tbool\n\n)

  endforeach()

#file(APPEND ${out_file} config\ WHO_CARES\n\tbool\n\tdefault\ y\n)

#  foreach (CONFIG ${DOT_CONFIG_LIST})
#    string(REGEX MATCH "[^=]+" CONF_VARIABLE_NAME ${CONFIG})
#string(SUBSTRING ${CMAKE_MATCH_0} 7 -1 VAR_VALUE)
#file(APPEND ${out_file} \tselect\ ${VAR_VALUE}\n)
#  endforeach()


#file(APPEND ${out_file} mainmenu\ \"Zephyr\ Kernel\ Configuration\"\n\nsource\ \"Kconfig_sysbuild.zephyr\"\n)
file(APPEND ${out_file} mainmenu\ \"Zephyr\ Kernel\ Configuration\"\n\nsource\ \"share/sysbuild/Kconfig\"\n)
endfunction()

# Empty files to make kconfig.py happy.
file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/empty.conf)
#sb_soc_port(CONFIG_SOC_ "${BOARD_DIR}/${BOARD}_defconfig" SB "${CMAKE_CURRENT_BINARY_DIR}/sb_soc.conf")
sb_soc_port(CONFIG_SOC_ "${BOARD_DIR}/${BOARD}_defconfig" SB "${CMAKE_CURRENT_BINARY_DIR}/sb_soc.Kconfig")
set(APPLICATION_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(KCONFIG_BINARY_DIR     ${CMAKE_CURRENT_BINARY_DIR})
set(AUTOCONF_H             ${CMAKE_CURRENT_BINARY_DIR}/autoconf.h)
set(CONF_FILE              ${SB_CONF_FILE})
set(BOARD_DEFCONFIG        "${CMAKE_CURRENT_BINARY_DIR}/empty.conf")
#set(BOARD_SOCCONFIG        "${CMAKE_CURRENT_BINARY_DIR}/sb_soc.conf")
#set(BOARD_SOCCONFIG        "${CMAKE_CURRENT_BINARY_DIR}/sb_soc.Kconfig")
set(BOARD_SOCCONFIG        "${CMAKE_CURRENT_BINARY_DIR}/empty.conf")
list(APPEND ZEPHYR_KCONFIG_MODULES_DIR BOARD=${BOARD})
set(KCONFIG_NAMESPACE SB_CONFIG)

if(EXISTS ${APP_DIR}/Kconfig.sysbuild)
  set(KCONFIG_ROOT ${APP_DIR}/Kconfig.sysbuild)
else()
#  set(KCONFIG_ROOT ${ZEPHYR_BASE}/Kconfig_sysbuild)
  set(KCONFIG_ROOT ${CMAKE_CURRENT_BINARY_DIR}/sb_soc.Kconfig)
endif()
include(${ZEPHYR_BASE}/cmake/modules/kconfig.cmake)
set(CONF_FILE)
