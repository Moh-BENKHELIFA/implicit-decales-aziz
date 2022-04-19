//
// Created by Loïc Barthe on 27/03/2020.
//

#ifndef TEST_GAMUTFIELD2D_H
#define TEST_GAMUTFIELD2D_H

#include "../Field2D/Field2D.h"

class GamutField2D : public Field2D {

public:

    GamutField2D ();
    GamutField2D (Field2D *f, int cornerx, int cornery, unsigned int width, unsigned int height, double size, unsigned int n);

    double eval (const double x, const double y);

    void computeDiscreteField ();

    double getDiscreteFieldValue (unsigned int i, unsigned int j);

    unsigned int getCornerX ();
    unsigned int getCornerY ();

    unsigned int getIWidth ();
    unsigned int getIHeight ();

    Field2D *getDistanceField();


protected:

    Field2D *field;

    int cornerx;
    int cornery;

    unsigned int iwidth;
    unsigned int iheight;

    double *discreteField;
};


#endif //TEST_GAMUTFIELD2D_H
