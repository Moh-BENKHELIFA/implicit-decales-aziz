//
// Created by Loïc Barthe on 27/03/2020.
//

#include "Field2D.h"
#include <math.h>

Field2D::Field2D (): size (1.), n(2), iso(0.5) {

    invAtIso = invFallOff(iso);
    radius = size / invAtIso;
}

Field2D::Field2D (double size, unsigned int n)
        : size(size), n(n), iso(0.5) {

    invAtIso = invFallOff(iso);
    radius = size / invAtIso;
}


double Field2D::normalizeField (double distanceFieldValue){

    return distanceFieldValue / radius;
}

double Field2D::fallOff (double normalizedField){

    if (normalizedField>=1.) return 0.;
    if (normalizedField>0.) return pow(1-normalizedField*normalizedField,n);
    return 1.;
}

double Field2D::invFallOff (double valFallOff){

    return sqrt(1.-pow(valFallOff,1./(double(n))));
}

void Field2D::setSize(double value)
{
    size = value;
}

double Field2D::getSize (){

    return size;
}

double Field2D::getN () {

    return n;
}

double Field2D::getIso (){

    return iso;
}

void Field2D::setIso (double iso){

    this->iso=iso;
}
