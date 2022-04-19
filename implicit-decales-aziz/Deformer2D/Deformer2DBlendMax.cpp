//
// Created by Loïc Barthe on 26/03/2020.
//

#include "Deformer2DBlendMax.h"
#include <iostream>


Deformer2DBlendMax::Deformer2DBlendMax () : Deformer2D() {}

Deformer2DBlendMax::Deformer2DBlendMax (DecaleScalarField2D *f1, unsigned int index1, DecaleScalarField2D *f2, unsigned int index2)
        : Deformer2D (f1, index1, f2, index2) {}



double Deformer2DBlendMax::eval (double x, double y){
    double maxi=0.;
    double sum=0.;
    double value=0.;
    double valueCurrent=0.;
    double boundary = fields[0]->getIso() + fields[0]->getWidthBoundary();

    for (int i=0; i<fields.size();i++){
        int ix = (int)(x-fields[i]->getPosx()+fields[i]->getDWidth()/2.);
        int iy = (int)(y-fields[i]->getPosy()+fields[i]->getDHeight()/2.);

        if (i==innerIndex) valueCurrent = fields[innerIndex]->getDiscreteFieldValue(ix, iy, indexes[innerIndex]);

        if ((ix >= 0) && (ix<fields[i]->getIWidth()) && (iy >= 0) && (iy<fields[i]->getIHeight())){
            value=fields[i]->getDiscreteFieldValue(ix, iy, indexes[i]);
            sum += value;
            if (value>maxi){
                maxi=value;
            }
        }
    }

    if ((maxi == valueCurrent) && (valueCurrent >= boundary)) return valueCurrent;
    if ((maxi == valueCurrent) && (sum <= boundary)) return sum;
    if (maxi == valueCurrent) return boundary;
    if ((maxi != valueCurrent) && (valueCurrent <= 0.5) && (maxi <= 0.5)) return valueCurrent;
    value = valueCurrent - maxi + 0.5;
    if ((value > 0.) && (value < 0.5)) return value;
    if (value <= 0.) return 0.;
    return boundary;
}

double Deformer2DBlendMax::evalInner (double x, double y){

    double bound = 0.6;
    if ((x>bound) && (x>=y)) return x;
    if ((x+y <= bound) && (x>=y)) return x+y;
    if ((x <0.5) && (y <=0.5) && (y>=x)) return x;
    double value = x - y + 0.5;
    if ((value > 0.)&& (value < 0.5)) return value;
    if (value <= 0.) return 0.;
    return bound;
}
