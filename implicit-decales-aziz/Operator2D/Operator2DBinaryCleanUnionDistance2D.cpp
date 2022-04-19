//
// Created by Loïc Barthe on 28/03/2020.
//

#include "Operator2DBinaryCleanUnionDistance2D.h"
#include <math.h>
#include <iostream>


Operator2DBinaryCleanUnionDistance::Operator2DBinaryCleanUnionDistance () : Operator2DBinary () {}

Operator2DBinaryCleanUnionDistance::Operator2DBinaryCleanUnionDistance (Field2D *f0, Field2D *f1) : Operator2DBinary (f0,f1) {}


double Operator2DBinaryCleanUnionDistance::evalInner (double x, double y, double iso) {

    if ((x > iso) && (y > iso)) {
        if (iso > 0) return std::min(1., iso + sqrt((x - iso) * (x - iso) + (y - iso) * (y - iso)));
        else return iso + sqrt((x - iso) * (x - iso) + (y - iso) * (y - iso));
    }
    return std::max(x,y);
}

