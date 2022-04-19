//
// Created by Loïc Barthe on 29/03/2020.
//

#include "Operator2DNary.h"


Operator2DNary::Operator2DNary () : Operator2D () {}

Operator2DNary::Operator2DNary (VectorOfFields fields) : Operator2D () {

    for (int i=0;i<fields.size();i++)
        this->fields.push_back(fields[i]);
}
