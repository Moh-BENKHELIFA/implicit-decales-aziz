//
// Created by Loïc Barthe on 25/03/2020.
//

#ifndef TEST_DEFORMER2DMAX_H
#define TEST_DEFORMER2DMAX_H

#include "Deformer2D.h"

class Deformer2DMax : public Deformer2D {

public:

    Deformer2DMax ();
    //Deformer2DMax (VectorOfDecaleFields fs);
    Deformer2DMax (DecaleScalarField2D *f1, unsigned int index1, DecaleScalarField2D *f2, unsigned int index2);

    double eval (double x, double y);

protected:

    double evalInner (double x, double y);

};


#endif //TEST_DEFORMER2DMAX_H
