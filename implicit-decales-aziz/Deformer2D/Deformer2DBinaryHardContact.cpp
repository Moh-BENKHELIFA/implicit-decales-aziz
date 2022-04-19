//
// Created by Loïc Barthe on 26/03/2020.
//

#include "Deformer2DBinaryHardContact.h"
#include <math.h>
#include <iostream>


Deformer2DBinaryHardContact::Deformer2DBinaryHardContact () : Deformer2D (), hardness(4) {}

Deformer2DBinaryHardContact::Deformer2DBinaryHardContact
        (DecaleScalarField2D *f1, unsigned int index1, DecaleScalarField2D *f2, unsigned int index2)
        : Deformer2D (f1, index1, f2, index2) {

    hardness = 4;
}



double Deformer2DBinaryHardContact::eval (double x, double y){

    int ix0 = (int) (x - fields[0]->getPosx() + fields[0]->getDWidth() / 2.);
    int iy0 = (int) (y - fields[0]->getPosy() + fields[0]->getDHeight() / 2.);
    int ix1 = (int) (x - fields[1]->getPosx() + fields[1]->getDWidth() / 2.);
    int iy1 = (int) (y - fields[1]->getPosy() + fields[1]->getDHeight() / 2.);

    double f0=0.;
    double f1=0.;

    if ((ix0 >= 0) && (ix0 < fields[0]->getIWidth()) && (iy0 >= 0) && (iy0 < fields[0]->getIHeight()))
        f0 = fields[0]->getDiscreteFieldValue(ix0, iy0, indexes[0]);
    if ((ix1 >= 0) && (ix1 < fields[1]->getIWidth()) && (iy1 >= 0) && (iy1 < fields[1]->getIHeight()))
        f1 = fields[1]->getDiscreteFieldValue(ix1, iy1, indexes[1]);

    double t=pow(2.*f0,hardness);

    if ((f0<=fields[0]->getIso()) && (f1<=fields[0]->getIso())) return f1;
    if (f0>fields[0]->getIso()) return f0;
    return (1.-t)*f1+t/2.;
}

double Deformer2DBinaryHardContact::evalInner (double x, double y){

    double t=pow(2.*x,hardness);

    if ((x<=0.5) && (y<=0.5)) return y;
    if (x>0.5) return x;
    return (1.-t)*y+t/2.;
}
