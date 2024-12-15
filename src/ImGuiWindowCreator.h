//
// Created by slice on 11/2/24.
//

#ifndef WINDOWCREATOR_H
#define WINDOWCREATOR_H

#include <variant>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"

class ImGuiWindowCreator {
public:

    ImGuiWindowCreator(std::string_view windowTitle) {
        if (!ImGui::Begin(windowTitle.begin(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::End();
        }
    }

    template <typename T>
    ImGuiWindowCreator &display(std::string_view name, T var) {
        if constexpr (std::is_same_v<T, int>) {
            ImGui::Text("%s: %d", name.data(), var);
        } else if constexpr (std::is_same_v<T, float>) {
            ImGui::Text("%s: %.2f", name.data(), var);
        } else if constexpr (std::is_same_v<T, double>) {
            ImGui::Text("%s: %.2f", name.data(), var);
        } else if constexpr (std::is_same_v<T, long>) {
            ImGui::Text("%s: %ld", name.data(), var);
        } else if constexpr (std::is_same_v<T, long long>) {
            ImGui::Text("%s: %lld", name.data(), var);
        } else if constexpr (std::is_same_v<T, glm::mat4>) {
            ImGui::Text("%s:", name.data());
            float* ptr = glm::value_ptr(var);
            ImGui::PushItemWidth(50.0f);

            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    char value[64];
                    snprintf(value, sizeof(value), "%.3f", ptr[i * 4 + j]);

                    ImGui::PushID(i * 4 + j);
                    ImGui::InputText("", value, sizeof(value), ImGuiInputTextFlags_ReadOnly);
                    if (j < 3) ImGui::SameLine();
                    ImGui::PopID();
                }
            }

            ImGui::PopItemWidth();
        } else {
            ImGui::Text("%s: Unsupported type", name.data());
        }

        return *this;
    }

    template <typename T>
    ImGuiWindowCreator &input(std::string_view name, T* var) {
        if constexpr (std::is_same_v<T, int>) {
            ImGui::InputInt(name.data(), var);
        } else if constexpr (std::is_same_v<T, float>) {
            ImGui::InputFloat(name.data(), var);
        } else if constexpr (std::is_same_v<T, double>) {
            ImGui::InputDouble(name.data(), var);
        } else if constexpr  (std::is_same_v<T, bool>) {
            ImGui::Checkbox(name.data(), var);
        } else if constexpr (std::is_same_v<T, glm::vec3>) {
            ImGui::InputFloat3(name.data(), glm::value_ptr(*var));
        }
        else {
            ImGui::Text("%s: Unsupported type", name.data());
        }

        return *this;
    }

    template <typename T, typename  U>
    ImGuiWindowCreator &slider(std::string_view name, T* var, U min, U max) {
        if constexpr (std::is_same_v<T, int>) {
            ImGui::SliderInt(name.data(), var, min, max);
        } else if constexpr (std::is_same_v<T, float>) {
            ImGui::SliderFloat(name.data(), var, min, max);
        } else if constexpr (std::is_same_v<T, glm::vec3>) {
            ImGui::SliderFloat3(name.data(), glm::value_ptr(*var), min, max);
        } else {
            ImGui::Text("%s: Unsupported type", name.data());
        }

        return *this;
    }

    ImGuiWindowCreator &heading(std::string_view text) {
        ImGui::SetWindowFontScale(1.5f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));

        ImGui::Text(text.data());

        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Dummy(ImVec2(0.0f, 5.0f));

        return *this;
    }

    ImGuiWindowCreator &text(std::string_view text) {
        ImGui::Text(text.data());

        return *this;
    }

    ImGuiWindowCreator &spacing(float size = 5) {
        ImGui::Dummy(ImVec2(0.0f, size));

        return *this;
    }

    void end() {
        ImGui::End();
    }
};



#endif //WINDOWCREATOR_H
