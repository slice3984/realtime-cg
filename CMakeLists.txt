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
        src/Final/SimplexNoise.h
        src/Final/SimplexNoise.h
)

add_executable(realtime_cg
        ${IMGUI_SOURCES}
        Linking/lib/glad.c
        src/main.cpp
        src/stb.cpp
        src/RenderBase.h
        src/Lectures/00-DemoLecture/Lecture00.h
        src/Lectures/01-Triangle/Lecture01.h
        src/Shader.cpp
        src/Shader.h
        src/Shaders/BaseShaderProgram.cpp
        src/Shaders/BaseShaderProgram.h
        src/Lectures/00-ImGuiTests/ImGuiTests.h
        src/ImGuiWindowCreator.h
        src/Assignments/01-GLSL/Assignment01.h
        src/OpenglUtils.h
        src/OpenglUtils.cpp
        src/Lectures/02-MultipleDraws/Lecture02.h
        src/Lectures/03-MVP/Lecture03.h
        src/GltfLoader.cpp
        src/GltfLoader.h
        src/OrbitCamera.h
        src/Lectures/04-Lighting/Lecture04.h
        src/GPUModelUploader.h
        src/Lectures/05-Textures/Lecture05.h
        src/Lectures/06-Textures2/Lecture06.h
        src/ImageData.h
        src/TextureHandle.h
        src/RenderCall.h
        src/TextureType.h
        src/RenderEntity.h
        src/Shaders/ModelShader/ModelShaderProgram.h
        src/LectureShaderProgram.h
        src/RenderQueue.h
        src/Renderer.h
        src/Shaders/SkyboxShader/SkyboxShaderProgram.h
        src/FPSCamera.h
        src/ImGuiWindows.h
        src/Utils.h
        src/Lectures/07-Framebuffer/Lecture07.h
        src/Shaders/TilingShader/TilingShaderProgram.h
        src/Final/Project.h
        src/Final/TerrainGenerator.h
        src/Shaders/TerrainShader/TerrainShaderProgram.h
        src/Final/SimplexNoise.h
        src/Shaders/WaterShader/WaterShaderProgram.h
        src/GeometryUtils.h
        src/Final/TerrainPatchLODGenerator.h
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
