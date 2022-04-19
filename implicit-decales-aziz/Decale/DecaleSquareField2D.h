//
// Created by Loïc Barthe on 25/03/2020.
//

#ifndef TEST_DECALESQUAREFIELD2D_H
#define TEST_DECALESQUAREFIELD2D_H


#include "DecaleScalarField2D.h"

class DecaleSquareField2D : public DecaleScalarField2D{

public:

    DecaleSquareField2D (double posx, double posy, double radius, unsigned int id, int z_index, unsigned int n=2);

    double variableRadius (double x, double y);
};


#endif //TEST_SQUAREFIELD2D_H
