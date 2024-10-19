//
// Created by slice on 10/19/24.
//

#ifndef LECTUREBASE_H
#define LECTUREBASE_H

#include <string>

class LectureBase {
protected:
    std::string m_title;
public:
    std::string getTitle() const { return m_title; }

    explicit LectureBase(std::string_view title) : m_title(title) {}
    virtual void init() = 0;
    virtual void render() = 0;

    virtual ~LectureBase() = default;
};

#endif //LECTUREBASE_H
