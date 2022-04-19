//
// Created by Loïc Barthe on 28/03/2020.
//

#ifndef TEST_OPERATOR2DBINARYCLEANINTERSECTIONDISTANCE_H
#define TEST_OPERATOR2DBINARYCLEANINTERSECTIONDISTANCE_H

#include "Operator2DBinary.h"


class Operator2DBinaryCleanIntersectionDistance   : public Operator2DBinary {

public:

    Operator2DBinaryCleanIntersectionDistance ();
    Operator2DBinaryCleanIntersectionDistance (Field2D *f0, Field2D *f1);

protected:

    double evalInner (double x, double y, double iso);
};


#endif //TEST_OPERATOR2DBINARYCLEANINTERSECTIONDISTANCE_H
