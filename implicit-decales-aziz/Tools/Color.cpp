//
// Created by Loïc Barthe on 06/01/2022.
//

#include "Color.h"

Color::Color(): r(0.), g(0.), b(0.){}

Color::Color(QColor c): r(c.redF()), g(c.greenF()), b(c.blueF()){}

Color::Color(const float r, const float g, const float b): r(r), g(g), b(b){}

void Color::setBlack(){

    r=0.; g=0.; b=0.;
}

void Color::setWhite(){

    r=1.; g=1.; b=1.;
}

void Color::setColor(const Color c) {

    r = c.r;
    g = c.g;
    b = c.b;
}
