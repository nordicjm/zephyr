if(SB_CONFIG_BOARD_NRF7120DK_NRF7120_CPUAPP_NS AND SB_CONFIG_BOOTLOADER_MCUBOOT)
  # MCUboot requires building on the secure image board target
  string(REPLACE "/" ";" split_board_qualifiers "${BOARD_QUALIFIERS}")
  list(REMOVE_AT split_board_qualifiers 2)
  string(REPLACE ";" "/" split_board_qualifiers "${split_board_qualifiers}")

  if(DEFINED BOARD_REVISION)
    set(board_target_secure "${BOARD}@${BOARD_REVISION}/${split_board_qualifiers}")
  else()
    set(board_target_secure "${BOARD}/${split_board_qualifiers}")
  endif()

  set_target_properties(mcuboot PROPERTIES BOARD ${board_target_secure})
endif()
