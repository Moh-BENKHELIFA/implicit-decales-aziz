//
// Created by Loïc Barthe on 25/03/2020.
//

#ifndef TEST_DECALEDISKFIELD2D_H
#define TEST_DECALEDISKFIELD2D_H

#include "DecaleScalarField2D.h"

class DecaleDiskField2D : public DecaleScalarField2D{

public:

    DecaleDiskField2D (double posx, double posy, double radius, unsigned int id, int z_index, unsigned int n=2);

    double variableRadius (double x, double y);
};

#endif
