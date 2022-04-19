//
// Created by Loïc Barthe on 06/01/2022.
//

#ifndef TEST_COLOR_H
#define TEST_COLOR_H

#include <QColor>

class Color {
public:

    Color();
    Color(QColor c);
    Color(const float r, const float g, const float b);

    void setColor (const Color c);

    void setBlack();
    void setWhite();

    float r;
    float g;
    float b;
};


#endif //TEST_COLOR_H
