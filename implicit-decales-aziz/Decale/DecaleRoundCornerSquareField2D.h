//
// Created by Loïc Barthe on 20/04/2020.
//

#ifndef TEST_DECALEROUNDCORNERSQUAREFIELD2D_H
#define TEST_DECALEROUNDCORNERSQUAREFIELD2D_H

#include "DecaleScalarField2D.h"

class DecaleRoundCornerSquareField2D : public DecaleScalarField2D {

public:

    /**
     * Create a square Decale with round corners
     * @param posx Decale position in x
     * @param posy Decale position in y
     * @param radius Decale half width
     * @param angleCorner Angle from the x axis from which the rounded corner starts
     * @param n Slope of the fallOff function
     */
    DecaleRoundCornerSquareField2D(double posx, double posy, double radius,
                                   double angleCorner, unsigned int id, int z_index, unsigned int n = 2);

    double variableRadius (double x, double y);

    /**
     * Virtual in DecaleScalarFields2D.h
     * Scale the Decale along axis by factors scalex and scaley.
     * This function updates the Decale parameters (width, height, corners).
     * This function does not recompute the field buffers.
     * @param scalex : scale the Decale along the x axis
     * @param scaley : scale the Decale along the y axis
     */
    void scale (const double scalex, const double scaley);

protected:

    double tani;
    Vector2D oi;
    double rx, ry;
    double c;

};


#endif //TEST_DECALEROUNDCORNERSQUAREFIELD2D_H
