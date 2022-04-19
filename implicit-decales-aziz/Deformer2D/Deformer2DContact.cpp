//
// Created by Loïc Barthe on 26/03/2020.
//

#include "Deformer2DContact.h"
#include <iostream>
#include <math.h>


//Deformer2DMax::Deformer2DMax (VectorOfDecaleFields fs) : Deformer2D (fs) {}

Deformer2DContact::Deformer2DContact () : Deformer2D () {}

Deformer2DContact::Deformer2DContact (DecaleScalarField2D *f1, unsigned int index1, DecaleScalarField2D *f2, unsigned int index2)
        : Deformer2D (f1, index1, f2, index2) {}



double Deformer2DContact::eval (double x, double y){

    double maxi=0.;
    double value=0.;
    double valueCurrent=0.;
    fieldValues.clear();

    for (int i=0; i<fields.size();i++){
        int ix = (int)(x-fields[i]->getPosx()+fields[i]->getDWidth()/2.);
        int iy = (int)(y-fields[i]->getPosy()+fields[i]->getDHeight()/2.);

        if (i==innerIndex) valueCurrent = fields[innerIndex]->getDiscreteFieldValue(ix, iy, indexes[innerIndex]);

        if ((ix >= 0) && (ix<fields[i]->getIWidth()) && (iy >= 0) && (iy<fields[i]->getIHeight())){
            value=fields[i]->getDiscreteFieldValue(ix, iy, indexes[i]);
            if (value>maxi){
                fieldValues.push_back(maxi);
                maxi=value;
            }
            else
                fieldValues.push_back(value);
        }
    }
    double prodFi=1.;
    for (int i=0; i<fieldValues.size(); i++)
        prodFi = prodFi*h(fieldValues[i],maxi);

    if (maxi == valueCurrent) return 0.5 + (maxi-0.5)*prodFi;
    if ((valueCurrent <= 0.5)&&(maxi <= 0.5)) return valueCurrent;
    value = valueCurrent - maxi + 0.5;
    if (value > 0.) return value;
    return 0.;
}

double Deformer2DContact::evalInner (double x, double y){

    if ((x<=0.5) && (y<=0.5)) return x;
    if (((x>0.5) && (y<=0.5)) || (x-y>=0.)) return 0.5 + (x-0.5)*h(y,x);
    double value = x - y + 0.5;
    if (value > 0.) return value;
    return 0.;
}

double Deformer2DContact::h (double s, double t){

    if (s+t >= 1.) return 1. - pow ((s+t-1.)/(2.*t-1.),1./(1.-t));
    return 1.;
}