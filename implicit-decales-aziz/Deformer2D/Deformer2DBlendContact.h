//
// Created by Loïc Barthe on 01/04/2020.
//

#ifndef TEST_DEFORMER2DBLENDCONTACT_H
#define TEST_DEFORMER2DBLENDCONTACT_H

#include "Deformer2D.h"

class Deformer2DBlendContact  : public Deformer2D {


public:

    Deformer2DBlendContact ();

    Deformer2DBlendContact (DecaleScalarField2D *f1, unsigned int index1, DecaleScalarField2D *f2, unsigned int index2);

    double eval (double x, double y);

protected:

    std::vector<double>fieldValues;

    double h (double s, double t);

    double evalInner (double x, double y);

};


#endif //TEST_DEFORMER2DBLENDCONTACT_H
