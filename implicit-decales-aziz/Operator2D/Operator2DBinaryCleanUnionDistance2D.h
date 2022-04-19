//
// Created by Loïc Barthe on 28/03/2020.
//

#ifndef TEST_OPERATOR2DBINARYCLEANUNIONDISTANCE_H
#define TEST_OPERATOR2DBINARYCLEANUNIONDISTANCE_H

#include "Operator2DBinary.h"

class Operator2DBinaryCleanUnionDistance  : public Operator2DBinary {

public:

    Operator2DBinaryCleanUnionDistance ();
    Operator2DBinaryCleanUnionDistance (Field2D *f0, Field2D *f1);

protected:

    double evalInner (double x, double y, double iso);

};


#endif //TEST_OPERATOR2DBINARYCLEANUNIONDISTANCE_H
