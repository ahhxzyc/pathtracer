set(TARGET_NAME stb_image)
add_library(${TARGET_NAME} 
    STATIC ${CMAKE_CURRENT_SOURCE_DIR}/stb_image_build.cpp)
target_include_directories(${TARGET_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})