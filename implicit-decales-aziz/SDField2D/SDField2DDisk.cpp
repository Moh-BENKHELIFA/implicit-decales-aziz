//
// Created by Loïc Barthe on 06/10/2020.
//

#include "SDField2DDisk.h"
#include <cmath>


SDField2DDisk::SDField2DDisk () : Field2D(), c(0.,0.), r(1.) {

    iso = 0.;
}

SDField2DDisk::SDField2DDisk (Vector2D c, double r) : Field2D(), c(c), r(r){

    iso = 0.;
}

double SDField2DDisk::eval (const double x, const double y){

    return r - sqrt((x-c.x)*(x-c.x)+(y-c.y)*(y-c.y));
}
