//
// Created by Loïc Barthe on 06/10/2020.
//

#ifndef TEST_SDFIELD2DDISK_H
#define TEST_SDFIELD2DDISK_H

#include "../Field2D/Field2D.h"
#include "../Tools/Vector2D.h"

class SDField2DDisk : public Field2D {

public:

    SDField2DDisk ();
    SDField2DDisk (Vector2D c, double r);

    double eval (const double x, const double y);

protected:

    //Disk center
    Vector2D c;
    //Disk radius
    double r;
};


#endif //TEST_SDFIELD2DDISK_H
