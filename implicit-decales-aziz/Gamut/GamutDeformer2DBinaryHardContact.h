//
// Created by Loïc Barthe on 27/03/2020.
//

#ifndef TEST_GAMUTDEFORMER2DBINARYHARDCONTACT_H
#define TEST_GAMUTDEFORMER2DBINARYHARDCONTACT_H

#include "../Deformer2D/Deformer2D.h"
#include "GamutField2D.h"

class GamutDeformer2DBinaryHardContact : public Deformer2D {

public:

    GamutDeformer2DBinaryHardContact ();

    GamutDeformer2DBinaryHardContact (GamutField2D *g, DecaleScalarField2D *f, unsigned int index);

    double eval (double x, double y);

    //void applyToDiscreteFields ();

protected:

    GamutField2D *gamut;

    unsigned int hardness;

    double evalInner (double x, double y);
};


#endif //TEST_GAMUTDEFORMER2DBINARYHARDCONTACT_H
