//
// Created by Loïc Barthe on 29/03/2020.
//

#include "Operator2DBinary.h"


Operator2DBinary::Operator2DBinary () : Operator2D () {}

Operator2DBinary::Operator2DBinary (Field2D *f0, Field2D *f1) : Operator2D () {

    field0 = f0;
    field1 = f1;
}


double Operator2DBinary::eval (const double x, const double y) {

    double f0 = field0->eval(x,y);
    double f1 = field1->eval(x,y);

    return evalInner(f0,f1,field0->getIso());
}