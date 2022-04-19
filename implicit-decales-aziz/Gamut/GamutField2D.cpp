//
// Created by Loïc Barthe on 27/03/2020.
//

#include "GamutField2D.h"

GamutField2D::GamutField2D () : field(), Field2D() {}

GamutField2D::GamutField2D (Field2D *f, int cornerx, int cornery, unsigned int width, unsigned int height, double size, unsigned int n)
        : cornerx(cornerx), cornery(cornery), iwidth(width), iheight(height), Field2D(size, n) {

    field = f;
}


double GamutField2D::eval (const double x, const double y) {

    return fallOff(invAtIso - normalizeField(field->eval(x, y)));
}

void GamutField2D::computeDiscreteField () {

    discreteField = new double [iwidth*iheight];

    for (int i=0; i<iwidth; i++)
        for (int j=0; j<iheight;j++)
            discreteField[i*iheight+j]=eval(double(i+cornerx), double(j+cornery));
}


double GamutField2D::getDiscreteFieldValue (unsigned int i, unsigned int j){

    return discreteField[i*iheight+j];
}



unsigned int GamutField2D::getCornerX() {

    return cornerx;
}

unsigned int GamutField2D::getCornerY() {

    return cornery;
}

unsigned int GamutField2D::getIWidth() {

    return iwidth;
}

unsigned int GamutField2D::getIHeight() {

    return iheight;
}

Field2D *GamutField2D::getDistanceField()
{
    return field;
}
