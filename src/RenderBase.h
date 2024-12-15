//
// Created by slice on 10/19/24.
//

#ifndef LECTUREBASE_H
#define LECTUREBASE_H

#include <string>
#include <chrono>
#include "../Linking/include/glad/glad.h"
#include "../external/imgui/imgui.h"
#include "Shader.h"
#include "Shaders/BaseShaderProgram.h"
#include "LectureShaderProgram.h"

class RenderBase {
protected:
    std::string m_title;
    std::string m_vertexShaderPath;
    std::string m_fragmentShaderPath;
    GLuint m_programId{};
    LectureShaderProgram m_program;
    bool m_storedProgam{false};
    std::chrono::time_point<std::chrono::system_clock> m_startTime;
public:
    std::string getTitle() const { return m_title; }

    explicit RenderBase(std::string_view title) : m_title(title) {
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    virtual void init() = 0;
    virtual void render() = 0;

    virtual ~RenderBase() = default;

protected:
    bool compileShaders(std::string_view vertexShaderPath, std::string_view fragmentShaderPath) {
        m_vertexShaderPath = vertexShaderPath;
        m_fragmentShaderPath = fragmentShaderPath;

        Shader vertexShader{vertexShaderPath, GL_VERTEX_SHADER};
        bool successVert = vertexShader.compile();

        Shader fragmentShader{fragmentShaderPath, GL_FRAGMENT_SHADER};
        bool successFrag = fragmentShader.compile();

        if (!successVert || !successFrag) {
            return false;
        }

        LectureShaderProgram program;

        program.attachShader(vertexShader);
        program.attachShader(fragmentShader);

        if (!program.linkProgram()) {
            return false;
        }

        GLuint oldProgram = m_programId;
        m_programId = program.getProgramId();
        m_program = program;

        // Check if there is already a shader program associated to this lecture
        // in case there is, delete the old program
        if (m_storedProgam) {
            glDeleteProgram(oldProgram);
        }

        m_storedProgam = true;
        return true;
    }

    void shaderReloadWindow() {
        ImGui::SetNextWindowSize(ImVec2(200, 60));
        ImGui::SetNextWindowPos(ImVec2(0, 0));

        if (ImGui::Begin("Reload shaders", nullptr, ImGuiWindowFlags_NoResize)) {
            if (ImGui::Button("Reload", ImVec2(150, 25))) {
                compileShaders(m_vertexShaderPath, m_fragmentShaderPath);
            }
        }

        ImGui::End();
    }

    void resetClock() {
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    float getElapsedTime() {
        auto currTime = std::chrono::high_resolution_clock::now();
        auto duration = currTime - m_startTime;

        return std::chrono::duration<float>(duration).count();
    }
};

#endif //LECTUREBASE_H
