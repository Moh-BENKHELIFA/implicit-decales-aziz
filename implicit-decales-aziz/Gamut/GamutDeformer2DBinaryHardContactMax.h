//
// Created by Loïc Barthe on 18/01/2022.
//

#ifndef TEST_GAMUTDEFORMER2DBINARYHARDCONTACTMAX_H
#define TEST_GAMUTDEFORMER2DBINARYHARDCONTACTMAX_H

#include "../Deformer2D/Deformer2D.h"
#include "GamutField2D.h"

class GamutDeformer2DBinaryHardContactMax : public Deformer2D  {

public:

    GamutDeformer2DBinaryHardContactMax ();

    GamutDeformer2DBinaryHardContactMax (GamutField2D *g, DecaleScalarField2D *f, unsigned int index);

    double eval (double x, double y);

    //void applyToDiscreteFields ();

protected:

    GamutField2D *gamut;

    double evalInner (double x, double y);
};


#endif //TEST_GAMUTDEFORMER2DBINARYHARDCONTACTMAX_H
