//
// Created by Loïc Barthe on 29/03/2020.
//

#ifndef TEST_OPERATOR2D_H
#define TEST_OPERATOR2D_H

#include "../Field2D/Field2D.h"

class Operator2D : public Field2D {

public:

    Operator2D();

protected:


    virtual double evalInner(double x,double y, double iso) = 0;
};


#endif //TEST_OPERATOR2D_H
