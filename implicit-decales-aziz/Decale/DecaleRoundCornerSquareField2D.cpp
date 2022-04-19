//
// Created by Loïc Barthe on 20/04/2020.
//

#include "DecaleRoundCornerSquareField2D.h"
#include <math.h>
#include <iostream>


DecaleRoundCornerSquareField2D::DecaleRoundCornerSquareField2D(double posx, double posy, double radius,
                                                               double angleCorner,unsigned int id, int z_index, unsigned int n)
        : DecaleScalarField2D(posx, posy, radius,id, z_index, n) {

    tani = tan(angleCorner);
    oi.set(this->radius * tani,this->radius * tani);
    rx = this->radius * (1. - tani);
    ry = rx;
    c = oi.x*oi.x/(rx*rx) + oi.y*oi.y/(ry*ry) - 1.;
}

double DecaleRoundCornerSquareField2D::variableRadius (double x, double y) {

    Vector2D p(x-posx,y-posy);

    double up = u*p;
    double vp = v*p;

    p.set(up,vp);

    if ((fabs(vp)*scalex > fabs(up*tani)*scaley) && (fabs(up)*scaley > fabs(vp*tani)*scalex)) {

        Vector2D dir (fabs(p.x/p.norm()), fabs(p.y/p.norm()));

        double a = dir.x*dir.x/(rx*rx) + dir.y*dir.y/(ry*ry);
        double b = -2.*(oi.x*dir.x/(rx*rx) + oi.y*dir.y/(ry*ry));

        double delta = b*b - 4.*a*c;

        return (-b + sqrt(delta)) / (2.*a);
    }

    if (fabs(up*scaley) >= fabs(vp*scalex)) return radius*scalex*p.norm()/fabs(up);
    return radius*scaley*p.norm()/fabs(vp);
}

void DecaleRoundCornerSquareField2D::scale(const double scalex, const double scaley) {

    DecaleScalarField2D::scale(scalex, scaley);

    oi.set(oi.x*scalex,oi.y*scaley);

    rx = rx*scalex;
    ry = ry*scaley;

    c = oi.x*oi.x/(rx*rx) + oi.y*oi.y/(ry*ry) -1.;
}
