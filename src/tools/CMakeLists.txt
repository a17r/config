# Device listing
ADD_EXECUTABLE( openicc-device "${CMAKE_CURRENT_SOURCE_DIR}/openicc_device.c" )
TARGET_LINK_LIBRARIES( openicc-device OpenICC ${EXTRA_LIBS} )
INSTALL (TARGETS openicc-device DESTINATION ${CMAKE_INSTALL_BINDIR})

