//
// Created by Loïc Barthe on 25/03/2020.
//

#ifndef TEST_DECALESCALARFIELD2D_H
#define TEST_DECALESCALARFIELD2D_H

#include "../Field2D/Field2D.h"
#include <vector>
#include <map>
#include "../Tools/Vector2D.h"


typedef std::vector<double *> VectorOfDiscreteFields;

class DecaleScalarField2D : public Field2D {

public:

    DecaleScalarField2D();
    DecaleScalarField2D(double posx, double posy, double size, unsigned int id, int z_index, unsigned int n);

    double eval (const double x, const double y);

    double dist (double x, double y);

    double normalizedDist (double x, double y);

    virtual double variableRadius (double x, double y)=0;

    /**
     * Rotates the Decale by an angle theta (in Radians) from its current rotation.
     * The Decale width, height and corner are adjusted.
     * @param theta : angle of rotation in grads
     */
    void rotate (const double theta);
    /**
     * Scale the Decale along axis by factors scalex and scaley.
     * This function updates the Decale parameters (width, height, corners).
     * This function does not recompute the field buffers.
     * @param scalex : scale the Decale along the x axis
     * @param scaley : scale the Decale along the y axis
     */
    virtual void scale (const double scalex, const double scaley);

    /**
     * Create and compute a field function in a buffer and add this buffer to the vector of field buffers
     */
    void computeDiscreteField ();
    /**
     * Update the field buffer of index indexField in the vector of field buffers with the Decale evaluation function.
     * @param indexField: index of the buffer to recompute in the vector of field buffers.
     */
    void updateDiscreteField (unsigned int indexField);

    /**
     * Create and compute the (u,v) parameterization (in [0,1]*[0,1], or -1 = no image) in a buffer of Vector2D
     * (u,v) computations are done from the field values stored in the list of field buffers at the index indexField.
     * This buffer is stored in the Decale.
     * @param indexField: Decale field buffer from which the (u,v) parameterization is computed.
     */
    void computeDiscreteUVField (unsigned int indexField);
    /**
     * Compute the (u,v) parameterization (in [0,1]*[0,1], or -1 = no image) in the decale UV-buffer
     * (u,v) computations are done from the field values stored in the list of field buffers at the index indexField.
     * @param indexField: Decale field buffer from which the (u,v) parameterization is computed.
     */

    void updateDiscreteUVField (unsigned int indexField);
    /**
     * Change the size of all the Decale buffers (field buffers and UV Buffer) according to the width and height Decale attributes.
     * They all have to be recomputed (can be done with the updates of discrete fields = buffers).
     */
    void updateBuffersSize ();
    /**
     * Tell if the Decale size has changed (in general after a scale or a rotation) and if the Decale buffers size need to be adjusted.
     * @return false if the buffer size is correct and true if the buffer size has to be adjsuted to a new Decale size and orientation
     */
    bool hasToUpdateBuffersSize ();


    Vector2D getUV(unsigned int i, unsigned int j);

    double *getDiscreteField (unsigned int indexField);
    double getDiscreteFieldValue (unsigned int i, unsigned int j, unsigned int indexField);
    void addDiscreteField (double *discreteField);


    double getPosx ();
    double getPosy ();
    void setPosx(double newx);
    void setPosy(double newy);

    unsigned int getCornerX ();
    unsigned int getCornerY ();

    unsigned int getIWidth ();
    unsigned int getIHeight ();

    double getDWidth ();
    double getDHeight ();

    double getWidthBoundary ();

    unsigned int getNbDiscreteFields ();

    unsigned int getId();
    int getZIndex();

    double getScalex() const;
    void setScalex(double value);

    double getScaley() const;
    void setScaley(double value);
    void setZIndex(int value);

protected:

    int id;
    int z_index;

    double *discreteField;
    double *discreteDistanceField;
    Vector2D *discreteUVField;
    VectorOfDiscreteFields discreteFields;

    double posx;
    double posy;

    double dwidth;
    double dheight;

    /**
     * Accumulates scaled width and height while avoiding to over accumulate due to rotations
     */
    double noRotateDWidthWithScale;
    double noRotateDHeightWithScale;

    unsigned int iwidth;
    unsigned int iheight;

    int cornerx;
    int cornery;

    Vector2D u;
    Vector2D v;

    double widthBoundaryUV;

    double scalex;
    double scaley;

    bool hasRotated;
    bool needUpdateBuffersSize;

};


typedef std::vector<DecaleScalarField2D *> VectorOfDecaleFields;


#endif //TEST_SCALARFIELD2D_H
