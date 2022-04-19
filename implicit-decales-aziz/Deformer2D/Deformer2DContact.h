//
// Created by Loïc Barthe on 26/03/2020.
//

#ifndef TEST_DEFORMER2DCONTACT_H
#define TEST_DEFORMER2DCONTACT_H

#include "Deformer2D.h"

class Deformer2DContact : public Deformer2D {


public:

    Deformer2DContact ();

    Deformer2DContact (DecaleScalarField2D *f1, unsigned int index1, DecaleScalarField2D *f2, unsigned int index2);

    double eval (double x, double y);

private:

    std::vector<double>fieldValues;

    double h (double s, double t);

    double evalInner (double x, double y);

};


#endif //TEST_DEFORMER2DCONTACT_H
