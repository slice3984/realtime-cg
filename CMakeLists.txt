cmake_minimum_required(VERSION 3.28)
project(realtime_cg)

set(CMAKE_CXX_STANDARD 17)

set(IMGUI_SOURCES
        external/imgui/imgui.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/backends/imgui_impl_opengl3.cpp
        external/imgui/backends/imgui_impl_glfw.cpp
)

add_executable(realtime_cg
        ${IMGUI_SOURCES}
        Linking/lib/glad.c
        src/main.cpp
        src/stb.cpp
        src/Lectures/LectureBase.h
        src/Lectures/00-DemoLecture/Lecture00.h
        src/Lectures/01-Triangle/Lecture01.h
        src/Shader.cpp
        src/Shader.h
        src/ShaderProgram.cpp
        src/ShaderProgram.h
)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

target_include_directories(${PROJECT_NAME} PUBLIC
        Linking/include
        external/glfw/include
        external/imgui
        external/imgui/backends
        external/glm
)

find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME}
        OpenGL::GL
        glfw
)
