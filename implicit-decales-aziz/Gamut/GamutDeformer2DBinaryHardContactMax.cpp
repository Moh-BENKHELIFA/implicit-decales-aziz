//
// Created by Loïc Barthe on 18/01/2022.
//

#include "GamutDeformer2DBinaryHardContactMax.h"
#include <math.h>


GamutDeformer2DBinaryHardContactMax::GamutDeformer2DBinaryHardContactMax () : Deformer2D () {}

GamutDeformer2DBinaryHardContactMax::GamutDeformer2DBinaryHardContactMax (GamutField2D *g, DecaleScalarField2D *f, unsigned int index)
        : Deformer2D () {

    fields.push_back (f);
    indexes.push_back(index);

    gamut = g;
}

double GamutDeformer2DBinaryHardContactMax::eval (double x, double y){

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

    /* Decale deformer */
    if ((f0<=fields[innerIndex]->getIso()) && (f1<fields[innerIndex]->getIso())) return f1;
    if ((f0>=fields[innerIndex]->getIso()) && (f1>=fields[innerIndex]->getIso())) return 1.-f0;
    if ((f0>fields[innerIndex]->getIso()) && (f1<fields[innerIndex]->getIso()))
        return std::max(0.,0.5-sqrt((0.5-f0)*(0.5-f0)+(0.5-f1)*(0.5-f1)));
    return f1;
}


double GamutDeformer2DBinaryHardContactMax::evalInner (double x, double y) {


    if ((x<=0.5) && (y<=0.5)) return y;
    if ((x>=0.5) && (y>=0.5)) return 1.-x;
    if ((x>0.5) && (y<0.5))
        return std::max(0.,0.5-sqrt((0.5-x)*(0.5-x)+(0.5-y)*(0.5-y)));
    return x;

}
