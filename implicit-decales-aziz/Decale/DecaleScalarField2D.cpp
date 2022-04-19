//
// Created by Loïc Barthe on 25/03/2020.
//

#include "DecaleScalarField2D.h"
#include <math.h>
#include <iostream>



DecaleScalarField2D::DecaleScalarField2D() : widthBoundaryUV (0.05) {}



DecaleScalarField2D::DecaleScalarField2D(double posx, double posy, double size,unsigned int id, int z_index, unsigned int n)
    : Field2D (size, n) {

    this->id=id;
    this->z_index=z_index;

    this->posx=posx;
    this->posy=posy;

    this->u.set(1.,0.);
    this->v.set(0.,1.);

    dwidth=2.*radius;
    dheight=2.*radius;

    noRotateDWidthWithScale=dwidth;
    noRotateDHeightWithScale=dheight;

    iwidth=(unsigned int)dwidth;
    iheight=(unsigned int) dheight;

    cornerx=(int)(posx-radius);
    cornery=(int)(posy-radius);

    widthBoundaryUV=0.05;

    scalex=1.;
    scaley=1.;

    hasRotated = false;
    needUpdateBuffersSize = false;
}

double DecaleScalarField2D::dist (double x, double y) {

    return sqrt(pow((x-posx),2) + pow ((y-posy),2));
}

double DecaleScalarField2D::normalizedDist (double x, double y) {

    return dist(x,y)/variableRadius(x,y);
}

double DecaleScalarField2D::eval(const double x, const double y) {

    double d=normalizedDist(x,y);
    return fallOff (d);
}

void DecaleScalarField2D::computeDiscreteField() {

    discreteField = new double [iwidth*iheight];

    for (int i=0; i<iwidth; i++)
        for (int j=0; j<iheight;j++)
            discreteField[i*iheight+j]=eval(double(i)+posx-dwidth/2., double(j)+posy-dheight/2.);

    discreteFields.push_back(discreteField);
}

void DecaleScalarField2D::updateDiscreteField(unsigned int indexField) {

    for (int i=0; i<iwidth; i++)
        for (int j=0; j<iheight;j++)
            discreteFields[indexField][i*iheight+j]=eval(double(i)+posx-dwidth/2., double(j)+posy-dheight/2.);
}

void DecaleScalarField2D::computeDiscreteUVField (unsigned int indexField){

    discreteUVField = new Vector2D [iwidth*iheight];
    double x,y,valu,valv, valField;
    Vector2D p,s;

    for (int i=0; i<iwidth; i++)
        for (int j=0; j<iheight;j++) {
            valField = getDiscreteFieldValue(i,j,indexField);
            if (valField >= iso) {
                x = double(i)+posx-dwidth/2.;
                y = double(j)+posy-dheight/2.;
                p.x = x-posx;
                p.y = y-posy;
                p.normalize();
                s = p * invFallOff(valField);
                s = s * variableRadius(x,y);

                valu = ((s * u) / (variableRadius(posx+u.x,posy+u.y) * invAtIso) + 1.) / 2.;
                valv = ((s * v) / (variableRadius(posx+v.x,posy+v.y) * invAtIso) + 1.) / 2.;

                discreteUVField[i*iheight+j] = Vector2D(valu,valv);
            }
            else
                discreteUVField[i*iheight+j] = Vector2D(-1.,-1.);
        }
}
void DecaleScalarField2D::updateDiscreteUVField(unsigned int indexField) {

    double x,y,valu,valv, valField;
    Vector2D p,s;

    for (int i=0; i<iwidth; i++)
        for (int j=0; j<iheight;j++) {
            valField = getDiscreteFieldValue(i,j,indexField);
            if (valField >= iso) {
                x = double(i)+posx-dwidth/2.;
                y = double(j)+posy-dheight/2.;
                p.x = x-posx;
                p.y = y-posy;
                p.normalize();
                s = p * invFallOff(valField);
                s = s * variableRadius(x,y);

                valu = ((s * u) / (variableRadius(posx+u.x,posy+u.y) * invAtIso) + 1.) / 2.;
                valv = ((s * v) / (variableRadius(posx+v.x,posy+v.y) * invAtIso) + 1.) / 2.;

                discreteUVField[i*iheight+j] = Vector2D(valu,valv);
            }
            else
                discreteUVField[i*iheight+j] = Vector2D(-1.,-1.);
        }
}


Vector2D DecaleScalarField2D::getUV(unsigned int i, unsigned int j){

    return discreteUVField[i*iheight+j];
}


double DecaleScalarField2D::getPosx() {

    return posx;
}

double DecaleScalarField2D::getPosy() {

    return posy;
}


void DecaleScalarField2D::setPosx(double newx) {

    posx = newx;
    cornerx=(int)(posx-dwidth/2.);
}

void DecaleScalarField2D::setPosy(double newy) {

    posy = newy;
    cornery=(int)(posy-dheight/2.);
}

unsigned int DecaleScalarField2D::getId() {

    return id;
}

int DecaleScalarField2D::getZIndex(){
    return z_index;
}

unsigned int DecaleScalarField2D::getCornerX() {

    return cornerx;
}

unsigned int DecaleScalarField2D::getCornerY() {

    return cornery;
}

unsigned int DecaleScalarField2D::getIWidth() {

    return iwidth;
}

unsigned int DecaleScalarField2D::getIHeight() {

    return iheight;
}

double DecaleScalarField2D::getDWidth() {

    return dwidth;
}

double DecaleScalarField2D::getDHeight() {

    return dheight;
}


double *DecaleScalarField2D::getDiscreteField (unsigned int indexField){

    return discreteFields[indexField];
}


double DecaleScalarField2D::getDiscreteFieldValue (unsigned int i, unsigned int j, unsigned int indexField){

    return discreteFields[indexField][i*iheight+j];
}

void DecaleScalarField2D::addDiscreteField (double *discreteField){

    discreteFields.push_back(discreteField);
}

unsigned int DecaleScalarField2D::getNbDiscreteFields (){

    return discreteFields.size();
}

double DecaleScalarField2D::getScalex() const
{
    return scalex;
}

void DecaleScalarField2D::setScalex(double value)
{
    scalex = value;
}

double DecaleScalarField2D::getScaley() const
{
    return scaley;
}

void DecaleScalarField2D::setScaley(double value)
{
    scaley = value;
}
void DecaleScalarField2D::setZIndex(int value){
    z_index = value;
}
double DecaleScalarField2D::getWidthBoundary (){

    return widthBoundaryUV;
}

void DecaleScalarField2D::rotate(const double theta) {

    this->u.set (cos(theta)*u.x - sin(theta)*u.y, sin(theta)*u.x + cos(theta)*u.y);
    this->v.set (cos(theta)*v.x - sin(theta)*v.y, sin(theta)*v.x + cos(theta)*v.y);

    hasRotated = true;

    dwidth = sqrt(dwidth*dwidth+dheight*dheight);
    dheight = dwidth;

    iwidth=(unsigned int)dwidth;
    iheight=(unsigned int) dheight;

    cornerx=(int)(posx - dwidth/2.);
    cornery=(int)(posy - dheight/2.);

    /***********************************
    // The field and UV buffer size need to be updates and buffers need to be re-computed
    ***********************************/
    needUpdateBuffersSize = true;

}

void DecaleScalarField2D::scale(const double scalex, const double scaley) {

    /***********************************
    // Scale accumulation. Provides the scales from the rest pose.
    ***********************************/
    this->scalex = this->scalex * scalex;
    this->scaley = this->scaley * scaley;

    /***********************************
    // Accumulation of the width and height from scales.
    ***********************************/
    noRotateDWidthWithScale = noRotateDWidthWithScale * scalex;
    noRotateDHeightWithScale = noRotateDHeightWithScale * scaley;

    /***********************************
    // Conservative Decale overall size (up to 0-values) avoiding a too small size
    // when a scaled decale (larger) is rotated.
    ***********************************/
    if (hasRotated) dwidth = dheight = std::max(noRotateDWidthWithScale, noRotateDHeightWithScale);
    else {
        dwidth = noRotateDHeightWithScale;
        dheight = noRotateDHeightWithScale;
    }

    iwidth=(unsigned int)dwidth;
    iheight=(unsigned int)dheight;

    cornerx=(int)(posx - dwidth/2.);
    cornery=(int)(posy - dheight/2.);

    /***********************************
    // The field and UV buffer size need to be updates and buffers need to be re-computed
    ***********************************/
    needUpdateBuffersSize = true;
}
void DecaleScalarField2D::updateBuffersSize() {

    for (int i=0; i<discreteFields.size();i++){
        delete(discreteFields[i]);
        discreteFields[i] = new double [iwidth*iheight];
    }
    delete(discreteUVField);
    discreteUVField = new Vector2D [iwidth*iheight];

    needUpdateBuffersSize = false;
}

bool DecaleScalarField2D::hasToUpdateBuffersSize() {

    return needUpdateBuffersSize;
}


