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

cmake_minimum_required(VERSION 2.8.2)

# this sets the projects name
project(pontoon)

# the projects version number
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 1)

# some output
message(STATUS "")
message(STATUS "==============================================================")
message(STATUS "Configuring ${PROJECT_NAME}:")

# add c++11 compiler flag
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()


# adding cmake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(FindDependency)

# checking build type, setting to release when not set
IF(NOT CMAKE_BUILD_TYPE)
  MESSAGE(STATUS "No Specific build type specified: using Release")
  SET(CMAKE_BUILD_TYPE "Release")
ENDIF()

# rsb
find_package(RSB REQUIRED)
find_package(RCT REQUIRED)
find_package(RST REQUIRED sandbox)
find_package(RST_EXPERIMENTAL 0.15 REQUIRED)
find_package(rst-converters REQUIRED)
add_definitions(${RST_CONVERTERS_CFLAGS})
set(RSB_INCLUDE_DIRS
  ${RCT_INCLUDE_DIRS}
  ${RSB_INCLUDE_DIRS}
  ${RST_INCLUDE_DIRS}
  ${RST_CONVERTERS_INCLUDE_DIRS}
  ${RST_EXPERIMENTAL_INCLUDE_DIRS}
  )
set(RSB_LIBRARY_DIRS
  ${RSB_RUNTIME_LIBRARY_DIRS}
  ${RCT_LIBRARY_DIRS}
  ${RST_RUNTIME_LIBRARY_DIRS}
  ${RST_EXPERIMENTAL_RUNTIME_LIBRARY_DIRS}
  )
set(RSB_LIBRARIES
  ${RCT_LIBRARIES}
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

# boost
find_package(Boost 1.54 COMPONENTS date_time REQUIRED)

# zlib
find_package(ZLIB REQUIRED)

# opencv


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

# get all header files
FILE(GLOB HEADERS "${PROJECT_SOURCE_DIR}/src/*.h")
# get all sources
FILE(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
# get all apps
FILE(GLOB APPS "${PROJECT_SOURCE_DIR}/app/*.cpp")

# add all headers
include_directories("${PROJECT_SOURCE_DIR}/src/")
include_directories(${Boost_INCLUDE_DIR})
include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(BEFORE SYSTEM ${RSB_INCLUDE_DIRS})
include_directories(BEFORE SYSTEM ${ROS_INCLUDE_DIRS})

# add rsb-rst link directories
link_directories(${RSB_LIBRARY_DIRS})
link_directories(${ROS_LIBRARY_DIRS})

#add rst definitions
add_definitions(${RSB_DEFINITIONS})

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#create library
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME}
  ${RSB_LIBRARIES}
  ${ROS_LIBRARIES}
  ${ZLIB_LIBRARIES}
)
install(TARGETS "${PROJECT_NAME}"
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE
)


######  creating executables #####
foreach(APP ${APPS})
  STRING(REGEX REPLACE "/.*/" "" APP ${APP})
  STRING(REGEX REPLACE "[.]cpp" "" APP ${APP})
  message(STATUS "-- Adding executable: ${APP}")

  add_executable("${PROJECT_NAME}-${APP}"
    "${PROJECT_SOURCE_DIR}/app/${APP}.cpp"
  )

  target_link_libraries("${PROJECT_NAME}-${APP}"
    ${ICL_LIBRARIES}
    ${OPENNI2_LIBRARIES}
    ${PROJECT_NAME}
  )

  install(TARGETS "${PROJECT_NAME}-${APP}"
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib/static
          PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE
         )
endforeach(APP)

