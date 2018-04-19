#*********************************************************************
#**                                                                 **
#** File   : CMakeLists.txt                                         **
#** Authors: Viktor Richter                                         **
#**                                                                 **
#**                                                                 **
#** GNU LESSER GENERAL PUBLIC LICENSE                               **
#** This file may be used under the terms of the GNU Lesser General **
#** Public License version 3.0 as published by the                  **
#**                                                                 **
#** Free Software Foundation and appearing in the file LICENSE.LGPL **
#** included in the packaging of this file.  Please review the      **
#** following information to ensure the license requirements will   **
#** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
#**                                                                 **
#*********************************************************************

set(APPS
  cut-faces.cpp
  decode-images.cpp
  encode-images.cpp
  rsb-server.cpp
  send-image.cpp
  send-stream.cpp
  show-face-patches.cpp
  show-images.cpp
  write-images.cpp
  write-images-raw.cpp
)

if(BUILD_WITH_ROS)
  list(APPEND APPS
    image-bridge.cpp
    pacemaker.cpp
  )
endif(BUILD_WITH_ROS)

######  creating executables #####
foreach(APP ${APPS})
  STRING(REGEX REPLACE "/.*/" "" APP ${APP})
  STRING(REGEX REPLACE "[.]cpp" "" APP ${APP})
  message(STATUS "-- Adding executable: ${APP}")
  set(APPNAME "${PROJECT_NAME}-${APP}")

  add_executable("${APPNAME}"
    "${PROJECT_SOURCE_DIR}/app/${APP}.cpp"
  )

  set_target_properties("${APPNAME}" PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    INSTALL_RPATH_USE_LINK_PATH TRUE
    )

  target_include_directories(${APPNAME}
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  )

target_include_directories(${APPNAME} SYSTEM
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    ${Boost_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${RSB_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    )
  if(BUILD_WITH_ROS)
    target_include_directories(${APPNAME} SYSTEM
      PUBLIC
      ${ROS_INCLUDE_DIRS}
      )
  endif(BUILD_WITH_ROS)

  #target_link_directories(${PROJECT_NAME}
  #  PUBLIC
  #  ${RSB_LIBRARY_DIRS}
  #  )
  #if(BUILD_WITH_ROS)
  #  target_link_directories(${PROJECT_NAME}
  #    PUBLIC
  #    ${ROS_LIBRARY_DIR}
  #    )
  #endif(BUILD_WITH_ROS)

  target_compile_definitions(${PROJECT_NAME}
    PUBLIC
      ${RSB_DEFINITIONS}
      ${RST_CONVERTERS_CFLAGS}
    )


  target_link_libraries("${APPNAME}"
    ${OPENNI2_LIBRARIES}
    ${PROJECT_NAME}
  )

  if(BUILD_WITH_ROS)
    target_link_libraries("${APPNAME}"
      ${ROS_LIBRARIES}
    )
  endif(BUILD_WITH_ROS)

  install(TARGETS "${APPNAME}"
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib/static
          PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE
         )
endforeach(APP)
