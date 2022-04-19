//
// Created by Loïc Barthe on 27/03/2020.
//

#include "Operator2DMax.h"
#include <iostream>


Operator2DMax::Operator2DMax () : Operator2DNary () {}

Operator2DMax::Operator2DMax (VectorOfFields fields) : Operator2DNary (fields) {}



double Operator2DMax::eval (const double x, const double y) {

    double maxi = fields[0]->eval(x,y);
    double value;

    for (int i=1; i<fields.size(); i++) {
        value = fields[i]->eval(x, y);
        if (value > maxi) maxi = value;
    }
    return maxi;
}

double Operator2DMax::evalInner (double x, double y, double iso) {

    return std::max(x,y);
}