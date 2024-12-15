//
// Created by slice on 12/15/24.
//

#ifndef UTILS_H
#define UTILS_H
#include <iomanip>
#include <iostream>
#include <glm/glm.hpp>


namespace Utils {
    inline void generateMatrixCode(const glm::mat4& mat) {
        std::cout << "glm::mat4{" << std::endl;

        for (int i = 0; i < 4; ++i) {
            std::cout << "    ";
            for (int j = 0; j < 4; ++j) {
                std::cout << std::fixed << std::setprecision(6) << mat[i][j];
                if (j < 3) std::cout << ", ";
            }
            std::cout << (i < 3 ? "," : "") << std::endl;
        }

        std::cout << "};" << std::endl;
    }
};



#endif //UTILS_H
