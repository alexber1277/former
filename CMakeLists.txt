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

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.6-docking
)
FetchContent_MakeAvailable(imgui)

set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

set(SOURCES
        main.cpp
        src/app.cpp
        src/app.h
        camera/camera.cpp
        camera/camera.h
        input/input.cpp
        input/input.h
        object/object.cpp
        object/object.h
        gui/gui.cpp
        gui/gui.h
        mesh/mesh.cpp
        mesh/mesh.h
        shaders/shader.h
)

list(APPEND SOURCES ${IMGUI_SOURCES})

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

target_include_directories(${PROJECT_NAME} PRIVATE
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
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