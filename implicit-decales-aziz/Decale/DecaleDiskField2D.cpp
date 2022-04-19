//
// Created by Loïc Barthe on 25/03/2020.
//

#include "DecaleDiskField2D.h"
#include <math.h>


DecaleDiskField2D::DecaleDiskField2D(double posx, double posy, double radius,unsigned int id, int z_index, unsigned int n)
        : DecaleScalarField2D(posx, posy, radius,id, z_index, n) {

}

double DecaleDiskField2D::variableRadius (double x, double y) {

    Vector2D p(x-posx,y-posy);
    double t = atan (scalex/scaley * (v*p)/(u*p));

    return Vector2D(radius*scalex*cos(t), radius*scaley*sin(t)).norm();
}
