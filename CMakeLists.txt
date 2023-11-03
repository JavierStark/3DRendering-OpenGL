cmake_minimum_required(VERSION 3.5)
project(VertexRendering LANGUAGES CXX)
# set(CMAKE_CXX_COMPILER /msys64/mingw64/bin/g++)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

add_subdirectory(glad/)
add_subdirectory(glm/)

add_executable(${CMAKE_PROJECT_NAME} src/main.cpp src/shader.h src/camera.h src/stb_image.h src/stb_image.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE sfml-graphics glad glm)


target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_17)
if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${CMAKE_PROJECT_NAME}> $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}> COMMAND_EXPAND_LISTS)
endif()

install(TARGETS ${CMAKE_PROJECT_NAME})
