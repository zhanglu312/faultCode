cmake_minimum_required(VERSION 3.6)
project(faultCode)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
find_package(OpenCV 3.3.0 REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS})
include_directories(include)

set(SRC_REG src/codeRecognizor.cpp)

add_executable(TEST_IMG ${SRC_REG} test/test_img.cpp)
target_link_libraries(TEST_IMG ${OpenCV_LIBS})

add_executable(TEST_FILE ${SRC_REG} test/test_file.cpp)
target_link_libraries(TEST_FILE ${OpenCV_LIBS})

add_executable(TEST_PERFORMANCE ${SRC_REG} test/performanceCheck.cpp)
target_link_libraries(TEST_PERFORMANCE ${OpenCV_LIBS})

#add_library(faultCode SHARED ${SRC_REG})

