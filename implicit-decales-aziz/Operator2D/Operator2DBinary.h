//
// Created by Loïc Barthe on 29/03/2020.
//

#ifndef TEST_OPERATOR2DBINARY_H
#define TEST_OPERATOR2DBINARY_H

#include "Operator2D.h"

class Operator2DBinary : public Operator2D {

public:

    Operator2DBinary ();
    Operator2DBinary (Field2D *f0, Field2D *f1);

    double eval (const double x, const double y);


protected:

    Field2D *field0;
    Field2D *field1;
};


#endif //TEST_OPERATOR2DBINARY_H
