//
// Created by Loïc Barthe on 25/03/2020.
//

#include "Deformer2D.h"
#include <iostream>



Deformer2D::Deformer2D () {}

Deformer2D::Deformer2D(DecaleScalarField2D *f1, unsigned int index1, DecaleScalarField2D *f2, unsigned int index2)  {

    fields.push_back (f1);
    fields.push_back (f2);

    indexes.push_back(index1);
    indexes.push_back(index2);
}

void Deformer2D::addField (DecaleScalarField2D *f, unsigned int index){

    fields.push_back (f);
    indexes.push_back(index);
}

void Deformer2D::removeLastField(){
    //fields.erase(fields.cbegin()+7);
    fields.pop_back();
    indexes.pop_back();

}

void Deformer2D::applyToDiscreteFields() {

    double *discreteField;

        for (int i=0; i<fields.size(); i++){

            innerIndex = i;

            discreteField = new double [fields[i]->getIWidth()*fields[i]->getIHeight()];

            for (int j=0; j<fields[i]->getIWidth(); j++)
                for (int k=0; k<fields[i]->getIHeight();k++) {
                    discreteField[j * fields[i]->getIHeight() + k] = eval(
                            double(j) + fields[i]->getPosx() - fields[i]->getDWidth() / 2.,
                            double(k) + fields[i]->getPosy() - fields[i]->getDHeight() / 2.);
                }
            fields[i]->addDiscreteField(discreteField);
        }

}


void Deformer2D::applyToExistingDiscreteFields(unsigned int index) {

    double *discreteField;

    for (int i=0; i<fields.size(); i++){
        innerIndex = i;
        discreteField = fields[i]->getDiscreteField(index);

        for (int j=0; j<fields[i]->getIWidth(); j++)
            for (int k=0; k<fields[i]->getIHeight();k++) {
                discreteField[j * fields[i]->getIHeight() + k] = eval(
                        double(j) + fields[i]->getPosx() - fields[i]->getDWidth() / 2.,
                        double(k) + fields[i]->getPosy() - fields[i]->getDHeight() / 2.);
            }
    }
}

double Deformer2D::evalInner(double x,double y){

    return 0.;
}




