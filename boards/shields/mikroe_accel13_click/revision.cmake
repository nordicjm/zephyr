if(NOT DEFINED shield_qualifiers)
  set(shield_qualifiers "i2c")
else()
  set(supported_qualifiers
      i2c
      i2c/1
      i2c/2
      spi
      spi/1
      spi/2
     )

  if(NOT shield_qualifiers IN_LIST supported_qualifiers)
    message(FATAL_ERROR "Invalid shield revision for ${shield}: ${shield_qualifiers}. Valid revisions are: ${supported_qualifiers}")
  endif()
endif()
