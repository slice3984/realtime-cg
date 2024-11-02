//
// Created by slice on 11/2/24.
//

#ifndef WINDOWCREATOR_H
#define WINDOWCREATOR_H

#include <variant>
#include <string>
#include "imgui.h"

class ImGuiWindowCreator {
public:

    ImGuiWindowCreator(std::string_view windowTitle) {
        if (!ImGui::Begin(windowTitle.begin(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::End();
        }
    }

    template <typename T>
    ImGuiWindowCreator &display(std::string_view name, T* var) {
        if constexpr (std::is_same_v<T, int>) {
            ImGui::Text("%s: %d", name.data(), *var);
        } else if constexpr (std::is_same_v<T, float>) {
            ImGui::Text("%s: %.2f", name.data(), *var);
        } else if constexpr (std::is_same_v<T, double>) {
            ImGui::Text("%s: %.2f", name.data(), *var);
        } else {
            ImGui::Text("%s: Unsupported type", name.data());
        }

        return *this;
    }

    template <typename T>
    ImGuiWindowCreator &input(std::string_view name, T* var) {
        if constexpr (std::is_same_v<T, int>) {
            ImGui::InputInt(name.data(), var);
        }
        else if constexpr (std::is_same_v<T, float>) {
            ImGui::InputFloat(name.data(), var);
        }
        else if constexpr (std::is_same_v<T, double>) {
            ImGui::InputDouble(name.data(), var);
        } else if constexpr  (std::is_same_v<T, bool>) {
            ImGui::Checkbox(name.data(), var);
        }
        else {
            ImGui::Text("%s: Unsupported type", name.data());
        }

        return *this;
    }

    template <typename T>
    ImGuiWindowCreator &slider(std::string_view name, T* var, T min, T max) {
        if constexpr (std::is_same_v<T, int>) {
            ImGui::SliderInt(name.data(), var, min, max);
        } else if constexpr (std::is_same_v<T, float>) {
            ImGui::SliderFloat(name.data(), var, min, max);
        }  else {
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

    ImGuiWindowCreator &spacing(float size = 5) {
        ImGui::Dummy(ImVec2(0.0f, size));

        return *this;
    }

    void end() {
        ImGui::End();
    }
};



#endif //WINDOWCREATOR_H
