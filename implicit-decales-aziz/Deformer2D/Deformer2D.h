//
// Created by Loïc Barthe on 25/03/2020.
//

#ifndef TEST_DEFORMER2D_H
#define TEST_DEFORMER2D_H

#include "../Decale/DecaleScalarField2D.h"

typedef std::vector<unsigned int> VectorOfIndexes;

/**
 * A Deformer 2D is a binary Deformer. It is however implemented to enable the definition of n-ary deformers at the condition that it can be implemented
 * as binary ones, by varying the deforming fields fi, denoted f2 in this implementation.
 * It takes as input a vector of field functions f1, f2, ... fn, and returns the deformed field of f1.
 * It can thus be a deformer D: Rn -> R, D(f1(P), f2(P), ..., fn(P)) = Dp implemented as D: R2 -> R,  D(f1(P), fi(P)) = Dp
 */

class Deformer2D {

public:
    /**
     * Creates an empty Deformer. Field functions have to be added (with @fn addField(f, index));
     */
    Deformer2D ();

    /**
         * Creates a Deformer deforming f1 with the interaction between two initial field functions f1 and f2.
         * @param f1: the field function to deform.
         * @param index1: the index of the field function f1 buffer from which the deformation will be computed and stored in a new buffer of f1.
         * @param f2: the field function with whom f1 deformed.
         * @param index2: the index of the field function f2 buffer from which the deformation will be computed.
         * Additional fields can be added if the deformer is more than binary using @fn addField(f, index).
         */
    Deformer2D (DecaleScalarField2D *f1, unsigned int index1, DecaleScalarField2D *f2, unsigned int index2);

    /**
         * Virtual function defining the equation D(f1(P), fi(P)) of a specific Deformer
         * @param x: value of f1(P)
         * @param y: value of fi(P)
         * @return: Dp = D(f1(P), fi(P))
         */
    virtual double eval (double x, double y) = 0;

    /**
        * It remove a given field,
        *
        * @param f: the removed field function
        * @param index: index of the field buffer that need to be removed
        */
    void addField (DecaleScalarField2D *f, unsigned int index);

    /**
        * If it is the first field, it adds the deformed field to the vector of field functions
        * Otherwise, it adds an additional field function deforming f1
        * @param f: the added field function
        * @param index: index of the field buffer on which the deformer will have to be applied
        */
    void removeLastField ();

    /**
         * For each field of the vector of field functions:
         * It creates a new buffer per field on which the deformed field is computed and stored
         * from the indexed field buffer (buffer index stored in the vector of indices).
         * This new buffer is added to the vector of buffers of the deformed field.
         */
    virtual void applyToDiscreteFields ();

    /**
         * For each field of the vector of field functions:
         * The deformed field is computed and stored in the field buffer of index given as parameter,
         * from the indexed field buffer (buffer index stored in the vector of indices).
         * @param index: field buffer index of the buffer in which the deformed field has to be stored.
         */
    void applyToExistingDiscreteFields(unsigned int index);


public:

    /**
     * Vector of field functions.
     */
    VectorOfDecaleFields fields;
    VectorOfIndexes indexes;

    int innerIndex;

    virtual double evalInner(double x,double y);
};


typedef std::vector<Deformer2D *> VectorOfDeformers;

#endif //TEST_DEFORMER2D_H
