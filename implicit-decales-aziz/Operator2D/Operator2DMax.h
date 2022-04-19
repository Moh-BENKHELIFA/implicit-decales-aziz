//
// Created by Loïc Barthe on 27/03/2020.
//

#ifndef TEST_OPERATOR2DMAX_H
#define TEST_OPERATOR2DMAX_H

#include <vector>
#include "Operator2DNary.h"




class Operator2DMax  : public Operator2DNary {

public:

    Operator2DMax ();
    Operator2DMax (VectorOfFields fields);

    double eval (const double x, const double y);


protected:

    double evalInner (double x, double y, double iso);

};


#endif //TEST_OPERATOR2DMAX_H
