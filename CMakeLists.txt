cmake_minimum_required(VERSION 3.20)
project(pathtracer)
set(TARGET_NAME ${PROJECT_NAME})

# Qt components
set(QT_COMPONENTS Core Gui Widgets)
find_package(Qt5 REQUIRED COMPONENTS ${QT_COMPONENTS})
set(CMAKE_AUTOMOC ON)

# find source files
file(GLOB_RECURSE SRC_FILES
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/include/*.h)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_FILES})

# create executable target
add_executable(${TARGET_NAME} ${SRC_FILES} )
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# add Qt dependency
target_link_libraries(${TARGET_NAME} Qt5::Widgets Qt5::Core)
foreach(QT_COMPONENT ${QT_COMPONENTS})
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt5::${QT_COMPONENT}>
        $<TARGET_FILE_DIR:${TARGET_NAME}>)
endforeach()

# openMP
FIND_PACKAGE( OpenMP REQUIRED)
if(OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

# add libs
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
target_link_libraries(${TARGET_NAME} libs)