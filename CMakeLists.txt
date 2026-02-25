cmake_minimum_required(VERSION 3.16)
project(RTNV)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        glm
        GIT_REPOSITORY	https://github.com/g-truc/glm.git
        GIT_TAG 	0af55ccecd98d4e5a8d1fad7de25ba429d60e863
)
FetchContent_MakeAvailable(glm)

set(SOURCES
        main.cpp
        src/app.cpp
        src/app.h
        camera/camera.cpp
        camera/camera.h
        input/input.cpp
        input/input.h
        mesh/Mesh.cpp
        mesh/Mesh.h
        renderer/render.cpp
        renderer/render.h
)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(assimp REQUIRED)


add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
        OpenGL::GL
        GLEW::GLEW
        glfw
        glm::glm
        assimp::assimp
)

add_custom_command(TARGET RTNV POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:RTNV>/shaders
)

add_custom_command(TARGET RTNV POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:RTNV>/assets
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${GLEW_INCLUDE_DIRS}
        ${GLFW_INCLUDE_DIRS}
)