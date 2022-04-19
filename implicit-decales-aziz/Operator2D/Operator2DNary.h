//
// Created by Loïc Barthe on 29/03/2020.
//

#ifndef TEST_OPERATOR2DNARY_H
#define TEST_OPERATOR2DNARY_H

#include "Operator2D.h"

class Operator2DNary : public Operator2D {

public:

    Operator2DNary ();
    Operator2DNary (VectorOfFields fields);


protected:

    VectorOfFields fields;
};


#endif //TEST_OPERATOR2DNARY_H
