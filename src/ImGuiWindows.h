//
// Created by slice on 12/15/24.
//

#ifndef IMGUIWINDOWS_H
#define IMGUIWINDOWS_H
#include "ImGuiWindowCreator.h"


namespace ImGuiWindows {
    inline void controls() {
        ImGuiWindowCreator{"Controls"}
        .heading("Movement")
        .text("W - Forward")
        .text("A - Left")
        .text("S - Backwards")
        .text("D - Right")
        .text("SPACE - Up")
        .text("LSHIFT - Down")
        .spacing()
        .heading("Misc")
        .text("LCTRL - Disable Camera")
        .text("M - View Matrix -> std::cout")
        .end();
    }
};



#endif //IMGUIWINDOWS_H
