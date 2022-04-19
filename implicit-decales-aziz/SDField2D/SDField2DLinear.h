//
// Created by Loïc Barthe on 27/03/2020.
//

#ifndef TEST_SDFIELD2DLINEAR_H
#define TEST_SDFIELD2DLINEAR_H

#include "../Field2D/Field2D.h"
#include "../Tools/Vector2D.h"

class SDField2DLinear : public Field2D{

public:

    SDField2DLinear ();
    SDField2DLinear (Vector2D n, double d);

    double eval (const double x, const double y);

    void normalize ();

protected:

    Vector2D n;
    double d;
};


#endif //TEST_SDFIELD2DLINEAR_H
