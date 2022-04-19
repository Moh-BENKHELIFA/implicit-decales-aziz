//
// Created by Loïc Barthe on 27/03/2020.
//

#include "SDField2DLinear.h"

SDField2DLinear::SDField2DLinear () : d(0.), n(1,0), Field2D() {

    iso = 0.;
}

SDField2DLinear::SDField2DLinear (Vector2D n, double d) : d(0.), n(1,0), Field2D() {

    iso = 0.;
    this->n.copy(n);
    this->d = d;
}

double SDField2DLinear::eval (const double x, const double y){

    return n.x*x + n.y*y + d;
}

void SDField2DLinear::normalize (){

    n.normalize();
    d = d/n.norm();
}