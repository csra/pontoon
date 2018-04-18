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

# set all header files
set(HEADERS
  utils/Subject.h
  utils/CvHelpers.h
  utils/RsbHelpers.h
  utils/FpsLimiter.h
  utils/Exception.h
  utils/SynchronizedQueue.h
  convert/ScaleImageOpenCV.h
  convert/ConvertRstImageOpenCV.h
  convert/CompressRstImageZlib.h
  io/rst/ListenerCVImage.h
  io/rst/Listener.h
  io/rst/ListenerFaces.h
  io/rst/InformerCVImage.h
  io/rst/Informer.h
  io/ImageIO.h
  io/Cause.h
  )

if(BUILD_WITH_ROS)
  list(APPEND HEADERS
    convert/ConvertRstRosImage.h
    io/ros/ImageListener.h
    io/ros/Informer.h
  )
endif(BUILD_WITH_ROS)


# set all sources
set(SOURCES
  utils/SynchronizedQueue.cpp
  utils/RsbHelpers.cpp
  utils/Subject.cpp
  utils/Exception.cpp
  utils/CvHelpers.cpp
  utils/FpsLimiter.cpp
  convert/ScaleImageOpenCV.cpp
  convert/CompressRstImageZlib.cpp
  convert/ConvertRstImageOpenCV.cpp
  io/rst/ListenerFaces.cpp
  io/rst/ListenerCVImage.cpp
  io/rst/Listener.cpp
  io/rst/InformerCVImage.cpp
  io/rst/Informer.cpp
  io/ImageIO.cpp
  io/Cause.cpp
)

if(BUILD_WITH_ROS)
  list(APPEND SOURCES
    convert/ConvertRstRosImage.cpp
    io/ros/ImageListener.cpp
    io/ros/Informer.cpp
    )
endif(BUILD_WITH_ROS)

#create library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_include_directories(${PROJECT_NAME} SYSTEM
    PUBLIC
        ${Boost_INCLUDE_DIR}
        ${ZLIB_INCLUDE_DIRS}
        ${RSB_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
)
if(BUILD_WITH_ROS)
  target_include_directories(${PROJECT_NAME} SYSTEM
    PUBLIC
    ${ROS_INCLUDE_DIRS}
  )
endif(BUILD_WITH_ROS)

#target_link_directories(${PROJECT_NAME}
#    PUBLIC
#        ${RSB_LIBRARY_DIRS}
#)
#if(BUILD_WITH_ROS)
#    target_link_directories(${PROJECT_NAME}
#      PUBLIC
#      ${ROS_LIBRARY_DIR}
#  )
#endif(BUILD_WITH_ROS)

target_compile_definitions(${PROJECT_NAME}
  PUBLIC
    ${RSB_DEFINITIONS}
    ${RST_CONVERTERS_CFLAGS}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  INSTALL_RPATH_USE_LINK_PATH TRUE
  )

target_link_libraries(${PROJECT_NAME}
  ${RSB_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${OpenCV_LIBS}
)
if(BUILD_WITH_ROS)
  target_link_libraries(${PROJECT_NAME} ${ROS_LIBRARIES})
endif(BUILD_WITH_ROS)

install(TARGETS "${PROJECT_NAME}"
        EXPORT  ${PROJECT_NAME}Targets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE
)
