# Copyright (c) 2021 Nordic Semiconductor
#
# SPDX-License-Identifier: Apache-2.0

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

#-- BOARD=nrf5340dk_nrf5340_cpuapp
#-- BOARD_DIR=/tmp/aa/zephyr/boards/arm/nrf5340dk_nrf5340

include(python)

# Find all boards used
set(USED_BOARDS)
set(USED_BOARD_DIRS)
foreach(image ${IMAGES})
  set(THIS_BOARD)
  set(THIS_BOARD_DIR)

  sysbuild_get(THIS_BOARD IMAGE ${image} VAR "CONFIG_BOARD" KCONFIG)
  sysbuild_get(THIS_BOARD_DIR IMAGE ${image} VAR "BOARD_DIR" CACHE)

  list(APPEND USED_BOARDS "${THIS_BOARD}")
  list(APPEND USED_BOARD_DIRS "${THIS_BOARD_DIR}")
endforeach()

list(REMOVE_DUPLICATES USED_BOARDS)
list(REMOVE_DUPLICATES USED_BOARD_DIRS)

set(USED_BOARDS_STR)
set(USED_BOARD_DIRS_STR)
foreach(arg ${USED_BOARDS})
  set(USED_BOARDS_STR ${USED_BOARDS_STR} ${arg})
endforeach()
foreach(arg ${USED_BOARD_DIRS})
  set(USED_BOARD_DIRS_STR ${USED_BOARD_DIRS_STR} ${arg})
endforeach()

# Load 
set(BOARD_PRIORITY_SCRIPT   ${ZEPHYR_BASE}/scripts/build/board_priority.py)

execute_process(
  COMMAND ${PYTHON_EXECUTABLE} ${BOARD_PRIORITY_SCRIPT}
  --boards ${USED_BOARDS_STR}
  --board-dirs ${USED_BOARD_DIRS_STR}
  RESULT_VARIABLE ret
  OUTPUT_VARIABLE stdout
  )
if(NOT "${ret}" STREQUAL "0")
  message(FATAL_ERROR "board_priority.py failed with return code: ${ret}")
endif()

string(REPLACE " " ";" stdout "${stdout}")
message(WARNING "got: ${stdout}")

#TODO: order domains according to ${stdout}
set(domains_yaml "default: ${DEFAULT_IMAGE}")
set(domains_yaml "${domains_yaml}\nbuild_dir: ${CMAKE_BINARY_DIR}")
set(domains_yaml "${domains_yaml}\ndomains:")
foreach(image ${IMAGES})
  set(domains_yaml "${domains_yaml}\n  - name: ${image}")
  set(domains_yaml "${domains_yaml}\n    build_dir: $<TARGET_PROPERTY:${image},_EP_BINARY_DIR>")
endforeach()
file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/domains.yaml CONTENT "${domains_yaml}")
