//
// Created by Loïc Barthe on 25/03/2020.
//

#include "Vector2D.h"
#include <math.h>

Vector2D::Vector2D(){

    x=0.;
    y=0.;
}


Vector2D::Vector2D(const Vector2D &v) : x(v.x), y(v.y){}

Vector2D::Vector2D(double x, double y){

    this->x=x;
    this->y=y;
}

void Vector2D::copy(const Vector2D &u) {

    x=u.x;
    y=u.y;
}

double Vector2D::operator * (const Vector2D &v){

    return x*v.x+y*v.y;
}

Vector2D Vector2D::operator * (double a){

    return Vector2D(a*x, a*y);
}


Vector2D Vector2D::operator + (const Vector2D &v) {

    return Vector2D(x+v.x, y+v.y);
}

Vector2D Vector2D::operator - (const Vector2D &v) {

    return Vector2D(x-v.x, y-v.y);
}

double Vector2D::norm() {

    return sqrt(x*x+y*y);
}

void Vector2D::normalize () {

    double norm=this->norm();

    x=x/norm;
    y=y/norm;
}

void Vector2D::set(double x, double y) {

    this->x=x;
    this->y=y;
}