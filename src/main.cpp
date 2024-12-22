#include <iostream>
#include <vector>
#include <memory>
#include "glad/glad.h" // OpenGL 4.2
#include <GLFW/glfw3.h>

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// stb_image
#include "stb_image.h"

// Misc
#include "OrbitCamera.h"
#include "Shader.h"

// Lectures
#include "FPSCamera.h"
#include "Utils.h"
#include "Lectures/00-DemoLecture/Lecture00.h"
#include "Lectures/00-ImGuiTests/ImGuiTests.h"
#include "Lectures/01-Triangle/Lecture01.h"
#include "Assignments/01-GLSL/Assignment01.h"
#include "Final/Project.h"
#include "Lectures/02-MultipleDraws/Lecture02.h"
#include "Lectures/03-MVP/Lecture03.h"
#include "Lectures/04-Lighting/Lecture04.h"
#include "Lectures/05-Textures/Lecture05.h"
#include "Lectures/06-Textures2/Lecture06.h"
#include "Lectures/07-Framebuffer/Lecture07.h"


static bool lbuttonDown = false;
bool disableCamera = false;
float lastX = 1280 / 2.0;
float lastY = 960 / 2.0;

OrbitCamera camera{3.0f};
FPSCamera fpsCamera{glm::vec3{0.0f, 0.0f, 0.0f}};

float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (!disableCamera) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            fpsCamera.processKeyboard(FORWARD, deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            fpsCamera.processKeyboard(BACKWARD, deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            fpsCamera.processKeyboard(LEFT, deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            fpsCamera.processKeyboard(RIGHT, deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            fpsCamera.processKeyboard(UP, deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            fpsCamera.processKeyboard(DOWN, deltaTime);
        }
    }
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;
            case GLFW_KEY_LEFT_SHIFT:
                if (!disableCamera) {
                    fpsCamera.processKeyboard(DOWN, deltaTime);
                }
                break;
            case GLFW_KEY_M:
                Utils::generateMatrixCode(fpsCamera.getViewMatrix());
                break;
            case GLFW_KEY_LEFT_CONTROL:
                disableCamera = !disableCamera;
                if (disableCamera) {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                } else {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
                break;
        }
    }
}


void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            lbuttonDown = true;
        } else if (action == GLFW_RELEASE) {
            lbuttonDown = false;
        }
    }
}

void mouseCallback(GLFWwindow *window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (disableCamera) {
        return;
    }

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float deltaX = xpos - lastX;
    float deltaY = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    fpsCamera.processMouseMovement(deltaX, deltaY);

    if (lbuttonDown) {
        camera.handleMouseDragEvent(deltaX, deltaY);
    }
}

void scrollCallback(GLFWwindow *window, double xOffset, double yOffset) {
    camera.handleMouseScrollEvent(yOffset);
    fpsCamera.processMouseScroll(static_cast<float>(yOffset));
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    // Will be replaced by a ImGui menu in the future
    const size_t activeLecture = 0;

    // GLM test
    glm::vec3 test{1.0f, 2.0f, 3.0f};

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow *window = glfwCreateWindow(1280, 960, "Demo", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scrollCallback);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    // Load OpenGL function pointers using GLAD
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 1280, 960);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io; // Initialize ImGui IO

    // Setup ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 420"); // Use GLSL version 420

    std::vector<std::unique_ptr<RenderBase> > lectures;
    lectures.push_back(std::make_unique<Project>("Terrain", fpsCamera));
    lectures.push_back(std::make_unique<ImGuiTests>("ImGui Tests"));
    lectures.push_back(std::make_unique<Lecture00>("Demo Lecture"));
    lectures.push_back(std::make_unique<Lecture01>("Triangle"));
    lectures.push_back(std::make_unique<Assignment01>("Assignment 1"));
    lectures.push_back(std::make_unique<Lecture02>("Multiple draws"));
    lectures.push_back(std::make_unique<Lecture03>("MVP", camera));
    lectures.push_back(std::make_unique<Lecture04>("Lighting", camera));
    lectures.push_back(std::make_unique<Lecture05>("Textures", camera));
    lectures.push_back(std::make_unique<Lecture06>("Textures 2", fpsCamera));
    lectures.push_back(std::make_unique<Lecture07>("Framebuffer", fpsCamera));

    lectures[activeLecture]->init();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Poll and handle events (Continious input / Camera)
        processInput(window);

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Clear color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the color buffer & depth buffer

        // Rendering
        lectures[activeLecture]->render();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
