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

cmake_minimum_required(VERSION 3.5.1)

# this sets the projects name
project(pontoon VERSION 2.0.0 LANGUAGES CXX)

# some output
message(STATUS "")
message(STATUS "==============================================================")
message(STATUS "Configuring ${PROJECT_NAME} v${PROJECT_VERSION}:")

option(BUILD_WITH_ROS "Build with ros" OFF)

# adding cmake module path
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
#include(FindDependency)

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
set(INSTALL_CMAKE_DIR "share/${PROJECT_NAME}/" CACHE PATH "Installation directory for CMake files")
set(INSTALL_DOCUMENTATION_DIR "share/${PROJECT_NAME}/doc" CACHE PATH "Installation directory for CMake files")

# Make relative paths absolute (needed later on)
set(INSTALL_CMAKE_DIR_REL "${INSTALL_CMAKE_DIR}")
foreach(p LIB BIN INCLUDE CMAKE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()

# rsb
find_package(RSB REQUIRED)
find_package(RST REQUIRED sandbox)
find_package(RST_EXPERIMENTAL REQUIRED)
find_package(rst-converters REQUIRED)
set(RSB_INCLUDE_DIRS
  ${RSB_INCLUDE_DIRS}
  ${RST_INCLUDE_DIRS}
  ${RST_CONVERTERS_INCLUDE_DIRS}
  ${RST_EXPERIMENTAL_INCLUDE_DIRS}
  )
set(RSB_LIBRARY_DIRS
  ${RSB_RUNTIME_LIBRARY_DIRS}
  ${RST_RUNTIME_LIBRARY_DIRS}
  ${RST_EXPERIMENTAL_RUNTIME_LIBRARY_DIRS}
  )
set(RSB_LIBRARIES
  ${RST_LIBRARIES}
  ${RSTSANDBOX_LIBRARIES}
  ${RST_EXPERIMENTAL_LIBRARIES}
  ${RSB_LIBRARIES}
  ${RST_CONVERTERS_LIBRARIES}
  )
set(RSB_DEFINITIONS
  ${RST_CFLAGS}
  ${RSTSANDBOX_CFLAGS}
  ${RST_EXPERIMENTAL_CFLAGS}
  )

# ros
if(BUILD_WITH_ROS)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(roscpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(image_transport REQUIRED)
set(ROS_INCLUDE_DIRS
    ${sensor_msgs_INCLUDE_DIRS}
    ${cv_bridge_INCLUDE_DIRS}
    ${roscpp_INCLUDE_DIRS}
    ${std_msgs_INCLUDE_DIRS}
    ${image_transport_INCLUDE_DIRS}
    )
set(ROS_LIBRARY_DIRS
    ${sensor_msgs_LIBRARY_DIRS}
    ${cv_bridge_LIBRARY_DIRS}
    ${roscpp_LIBRARY_DIRS}
    ${std_msgs_LIBRARY_DIRS}
    ${image_transport_LIBRARY_DIRS}
    )
set(ROS_LIBRARIES
    ${sensor_msgs_LIBRARIES}
    ${cv_bridge_LIBRARIES}
    ${roscpp_LIBRARIES}
    ${std_msgs_LIBRARIES}
    ${image_transport_LIBRARIES}
    )
endif(BUILD_WITH_ROS)

# boost
find_package(Boost 1.54 COMPONENTS date_time REQUIRED)

# zlib
find_package(ZLIB REQUIRED)

# opencv
find_package(OpenCV 3.0 REQUIRED COMPONENTS
  core highgui
  )

message(STATUS "opencv_found: ${OpenCV_core_FOUND}")
message(STATUS "opencv_libs: ${OpenCV_LIBS}")


if(NOT ${OpenCV_core_FOUND})
  message(FATAL_ERROR "Could not find core component in OpenCV")
else()
  if("${OpenCV_VERSION}" MATCHES "3.3.1")
    if(${CMAKE_BUILD_TYPE} MATCHES "[Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo]")
      if(${OpenCV_INSTALL_PATH} MATCHES ".*/ros/kinetic")
        message(WARNING
        "Building in ${CMAKE_BUILD_TYPE} mode against OpenCV ${OpenCV_VERSION} within a ros installation. "
        "This combination may lead to OpenCV library not found errors. "
        "Consider building in RELEASE or DEBUG mode."
        )
      endif()
    endif()
  endif()
endif()

message(STATUS "Looking for doxygen")
find_program(DOXYGEN_BIN NAMES doxygen)
if(NOT "${DOXYGEN_BIN}" STREQUAL "DOXYGEN_BIN-NOTFOUND")
  message(STATUS "found Doxygen, a documentation can be built with 'make doc'")
  add_custom_target(doc ALL
    COMMAND ${DOXYGEN_BIN} ${PROJECT_BINARY_DIR}/Doxyfile
    COMMENT "Creating html and tex documentation in doc/"
  )
  install(
    DIRECTORY ${PROJECT_BINARY_DIR}/doc/html
    DESTINATION "share/doc/${PROJECT_NAME}/"
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ WORLD_EXECUTE OWNER_EXECUTE GROUP_EXECUTE
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE
 )
else()
  message(STATUS "  doxygen not found, documentation will not be available")
endif()

configure_file(
  "${PROJECT_SOURCE_DIR}/Doxyfile.in"
  "${PROJECT_BINARY_DIR}/Doxyfile"
)

add_subdirectory(src)
add_subdirectory(app)

##### setup cmake config #####
# Project name in caps
string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)
# Add all targets to the build-tree export set
export(TARGETS ${PROJECT_NAME} ${APPLICATIONS}
  FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

# Create the Config.cmake and ConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}"
   "${INSTALL_INCLUDE_DIR}")
# ... for the install tree
set(CONF_INCLUDE_DIRS "\${FFORMATION_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(cmake/Config.cmake.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake" @ONLY)
# ... for both
configure_file(cmake/ConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" @ONLY)

# Install the Config.cmake and ConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

# Install the export set for use with the install-tree
install(EXPORT ${PROJECT_NAME}Targets DESTINATION
  "${INSTALL_CMAKE_DIR_REL}" COMPONENT dev)
