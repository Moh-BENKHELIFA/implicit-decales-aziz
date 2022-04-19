//
// Created by Loïc Barthe on 27/03/2020.
//

#include "GamutDeformer2DBinaryHardContact.h"
#include <math.h>
#include <iostream>


GamutDeformer2DBinaryHardContact::GamutDeformer2DBinaryHardContact () : hardness (4), Deformer2D () {}

GamutDeformer2DBinaryHardContact::GamutDeformer2DBinaryHardContact (GamutField2D *g, DecaleScalarField2D *f, unsigned int index)
        : Deformer2D () {

    fields.push_back (f);
    indexes.push_back(index);

    gamut = g;

    hardness = 4;
}

double GamutDeformer2DBinaryHardContact::eval (double x, double y){

    int ix0 = (int) (x - gamut->getCornerX());
    int iy0 = (int) (y - gamut->getCornerY());
    int ix1 = (int) (x - fields[innerIndex]->getPosx() + fields[innerIndex]->getDWidth() / 2.);
    int iy1 = (int) (y - fields[innerIndex]->getPosy() + fields[innerIndex]->getDHeight() / 2.);

    double f0=0.;
    double f1=0.;

    if ((ix0 >= 0) && (ix0 < gamut->getIWidth()) && (iy0 >= 0) && (iy0 < gamut->getIHeight()))
        f0 = gamut->getDiscreteFieldValue(ix0, iy0);
    if ((ix1 >= 0) && (ix1 < fields[innerIndex]->getIWidth()) && (iy1 >= 0) && (iy1 < fields[innerIndex]->getIHeight()))
        f1 = fields[innerIndex]->getDiscreteFieldValue(ix1, iy1, indexes[innerIndex]);

    double t=pow(2*f0,hardness);

    /* Composition operator */
    /*if ((f0<=fields[innerIndex]->getIso()) && (f1<=fields[innerIndex]->getIso())) return f1;
    if (f0>fields[innerIndex]->getIso()) return f0;
    return (1.-t)*f1+t/2.;*/

    /* Decale deformer */
    if ((f0<=fields[innerIndex]->getIso()) && (f1<fields[innerIndex]->getIso())) return f1;
    if ((f0>=fields[innerIndex]->getIso()) && (f1>=fields[innerIndex]->getIso())) return 1.-f0;
    if ((f0>fields[innerIndex]->getIso()) && (f1<fields[innerIndex]->getIso()))
        return std::max(0.,0.5-sqrt((0.5-f0)*(0.5-f0)+(0.5-f1)*(0.5-f1)));
    return (1.-t)*f1+t/2.;
    //return f1;
}


double GamutDeformer2DBinaryHardContact::evalInner (double x, double y) {

    double t=pow(2*x,hardness);

    if ((x<=0.5) && (y<=0.5)) return y;
    if ((x>=0.5) && (y>=0.5)) return 1.-x;
    if ((x>0.5) && (y<0.5))
        return std::max(0.,0.5-sqrt((0.5-x)*(0.5-x)+(0.5-y)*(0.5-y)));
    return (1.-t)*y+t/2.;

}

/*
void GamutDeformer2DBinaryHardContact::applyToDiscreteFields (){

    double *discreteField;
    for (int i=0; i<fields.size(); i++){
        discreteField = new double [fields[i]->getIWidth()*fields[i]->getIHeight()];

        for (int j=0; j<fields[i]->getIWidth(); j++)
            for (int k=0; k<fields[i]->getIHeight();k++) {
                discreteField[j * fields[i]->getIHeight() + k] = eval(
                        double(j) + fields[i]->getPosx() - fields[i]->getDWidth() / 2.,
                        double(k) + fields[i]->getPosy() - fields[i]->getDHeight() / 2.);
            }
        fields[i]->addDiscreteField(discreteField);
    }
}*/