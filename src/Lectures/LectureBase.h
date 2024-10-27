//
// Created by slice on 10/19/24.
//

#ifndef LECTUREBASE_H
#define LECTUREBASE_H

#include <string>
#include "glad/glad.h"
#include "imgui.h"
#include "../Shader.h"
#include "../ShaderProgram.h"

class LectureBase {
protected:
    std::string m_title;
    std::string m_vertexShaderPath;
    std::string m_fragmentShaderPath;
    GLuint m_programId{};
    bool storedProgam{false};
public:
    std::string getTitle() const { return m_title; }

    explicit LectureBase(std::string_view title) : m_title(title) {}

    virtual void init() = 0;
    virtual void render() = 0;

    virtual ~LectureBase() = default;

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

        ShaderProgram program;
        program.attachShader(vertexShader);
        program.attachShader(fragmentShader);

        if (!program.linkProgram()) {
            return false;
        }

        GLuint oldProgram = m_programId;
        m_programId = program.getProgramId();

        // Check if there is already a shader program associated to this lecture
        // in case there is, delete the old program
        if (storedProgam) {
            glDeleteProgram(oldProgram);
        }

        storedProgam = true;
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
};

#endif //LECTUREBASE_H
