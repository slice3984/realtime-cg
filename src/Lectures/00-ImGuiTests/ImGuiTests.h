//
// Created by slice on 10/28/24.
//

#ifndef IMGUITESTS_H
#define IMGUITESTS_H
#include "imgui.h"
#include "../../RenderBase.h"
#include "../../ImGuiWindowCreator.h"

class ImGuiTests : public RenderBase {
private:
    int a = 5;
    int b = 7;
    float c = 23.4f;
    bool d = true;
public:
    explicit ImGuiTests(std::string_view title) : RenderBase(title) {}

    void init() override {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(400, 600));
    }

    void render() override {
        float time = getElapsedTime();

        if (!d) {
            resetClock();
        }

        ImGuiWindowCreator("Test")
            .display("Time", time)
            .display("My float", c)
            .display("My int", a)
            .spacing()
            .input("My int", &a)
            .input("Bool", &d)
            .heading("My sliders")
            .slider("Float slider", &c, 2.5f, 10.0f)
            .slider("Int slider", &a, 1, 100)
            .end();
    }
};



#endif //IMGUITESTS_H
