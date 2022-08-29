cmake_minimum_required(VERSION 3.20)
project(pathtracer)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)

file(GLOB_RECURSE SRC_FILES
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/src/*.h)

add_executable(${PROJECT_NAME} ${SRC_FILES} )
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/stb_image)
target_link_libraries(${PROJECT_NAME} stb_image)

set(BUILD_TESTING OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/glm)
target_link_libraries(${PROJECT_NAME} glm::glm)

add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/tinyobj)
target_link_libraries(${PROJECT_NAME} tinyobjloader)


FIND_PACKAGE( OpenMP REQUIRED)
if(OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
