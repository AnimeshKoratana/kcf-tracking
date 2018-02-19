cmake_minimum_required(VERSION 3.9)
project(bus_tracking)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package( OpenCV REQUIRED )
find_package( Folly REQUIRED )
find_package( Glog REQUIRED )
find_package( DoubleConversion REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS})
include_directories(${FOLLY_INCLUDE_DIR})
include_directories(${GLOG_INCLUDE_DIR})
include_directories(${DOUBLE_CONVERSION_INCLUDE_DIR})

set(LIBS ${LIBS} ${FOLLY_LIBRARY})
add_executable(bus_tracking main.cpp)
target_link_libraries( bus_tracking ${OpenCV_LIBS} ${FOLLY_LIBRARY} ${GLOG_LIBRARY} ${DOUBLE_CONVERSION_LIBRARY})
