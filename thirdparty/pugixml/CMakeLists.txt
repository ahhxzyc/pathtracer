cmake_minimum_required(VERSION 3.4)
set(TARGET_NAME pugixml)

add_library(${TARGET_NAME} STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/pugixml.cpp)
target_include_directories(${TARGET_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)