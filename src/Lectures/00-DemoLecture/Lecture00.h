//
// Created by slice on 10/19/24.
//

#ifndef LECTURE01_H
#define LECTURE01_H
#include <iostream>

#include "../LectureBase.h"


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



#endif //LECTURE01_H
