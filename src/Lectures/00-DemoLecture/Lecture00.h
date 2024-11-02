//
// Created by slice on 10/19/24.
//

#ifndef LECTURE00_H
#define LECTURE00_H
#include <iostream>

#include "../../RenderBase.h"


class Lecture00 : public LectureBase {
public:
    explicit Lecture00(std::string_view title) : LectureBase(title) {
    }

    void init() override {
        std::cout << "init call" << std::endl;
    }

    void render() override {
        std::cout << "Render update call" << std::endl;
    }
};



#endif //LECTURE00_H
