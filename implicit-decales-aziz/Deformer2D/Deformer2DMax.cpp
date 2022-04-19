//
// Created by Loïc Barthe on 25/03/2020.
//

#include "Deformer2DMax.h"
#include <iostream>


Deformer2DMax::Deformer2DMax () : Deformer2D () {}

Deformer2DMax::Deformer2DMax (DecaleScalarField2D *f1, unsigned int index1, DecaleScalarField2D *f2, unsigned int index2)
        : Deformer2D (f1, index1, f2, index2) {}



double Deformer2DMax::eval (double x, double y){
    double maxi=0.;
    double value=0.;
    double valueCurrent=0.;

    for (int i=0; i<fields.size();i++){
        int ix = (int)(x-fields[i]->getPosx()+fields[i]->getDWidth()/2.);
        int iy = (int)(y-fields[i]->getPosy()+fields[i]->getDHeight()/2.);

        if (i==innerIndex) valueCurrent = fields[innerIndex]->getDiscreteFieldValue(ix, iy, indexes[innerIndex]);

        if ((ix >= 0) && (ix<fields[i]->getIWidth()) && (iy >= 0) && (iy<fields[i]->getIHeight())){
            value=fields[i]->getDiscreteFieldValue(ix, iy, indexes[i]);
            if (value>maxi){
                maxi=value;
            }
        }
    }

    if (maxi == valueCurrent) return maxi;
    if ((valueCurrent <= 0.5)&&(maxi <= 0.5)) return valueCurrent;
    value = valueCurrent - maxi + 0.5;
    if (value > 0.) return value;
    return 0.;
}

double Deformer2DMax::evalInner (double x, double y){

    if ((x>=y) || ((x<=0.5) && (y<=0.5))) return x;
    double value = x-y+0.5;
    if (value > 0.) return value;
    return 0.;
}
