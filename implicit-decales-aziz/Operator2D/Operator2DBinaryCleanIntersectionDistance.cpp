//
// Created by Loïc Barthe on 28/03/2020.
//

#include "Operator2DBinaryCleanIntersectionDistance.h"
#include <math.h>
#include <iostream>


Operator2DBinaryCleanIntersectionDistance::Operator2DBinaryCleanIntersectionDistance () : Operator2DBinary () {}

Operator2DBinaryCleanIntersectionDistance::Operator2DBinaryCleanIntersectionDistance (Field2D *f0, Field2D *f1) : Operator2DBinary (f0,f1) {}



double Operator2DBinaryCleanIntersectionDistance::evalInner (double x, double y, double iso) {

    if ((x < iso) && (y < iso)) {
        if (iso > 0.) return std::max(0., iso - sqrt((x - iso) * (x - iso) + (y - iso) * (y - iso)));
        else return iso - sqrt((x - iso) * (x - iso) + (y - iso) * (y - iso));
    }
    return std::min(x,y);
}

