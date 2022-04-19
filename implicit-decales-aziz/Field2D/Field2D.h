//
// Created by Loïc Barthe on 27/03/2020.
//
// Specialized Class for compact field function f defined from a distance field d.
// The distance field equals 0 at the primitive center and equals "radius" at the field limit of influence.
// The slope of f is parameterized by n (the larger, the steeper).

//  f(p) = ( 1 - (d(p)/radius)^2 )^n

// The evaluation of f is done in two steps:
// Step 1: Normalize the distance field: computes nf = d/radius
// Step 2: Apply the falloff function to the normalized field: computes f = (1-nf)^n


#ifndef TEST_FIELD2D_H
#define TEST_FIELD2D_H

#include <vector>


class Field2D {

public:

    // Constructors
    Field2D ();
    // size is the size of the primitive (see below)
    // n controls the slope of the falloff function (see below)
    Field2D (double size, unsigned int n);

    // Field evaluation at any position (x,y) => to be defined by type of Field Function
    // example of a bounded circle of center (0,0) and radius r => size = r and the eval function is written as:
    // return falloff(normalizeField(sqrt(x^2+y^2)));
    virtual double eval (const double x, const double y)=0;

    // Returns the iso-value used to define the primitive boundary
    // In general: 0.5 for compact fields and 0 for global fields
    double getIso ();

    // Returns the size if the primitive whose boundary is defined by f=Iso
    double getSize ();

    void setSize(double value);

    // returns the power of the falloff function
    double getN ();

    // Set the iso-value used to define the primitive boundary (0.5 by default)
    void setIso (double iso);



protected:

    // The distance field value is normalized so that its value is 0 at the primitive center
    // and 1 at the field variation limit (at the distance "radius" from the center).
    // Outside this field variation limit the falloff function assigns 0
    double normalizeField (double distanceFieldValue);

    // Computes the final field value : 1 at the primitive center, decreasing up to 0 at the
    // field variation limit (at the distance "radius" from the center). The field is the 0
    // outside this variation limit.
    double fallOff (double normalizedField);

    // Inverse of the Falloff function: returns the normalized distance from the field value
    double invFallOff (double valFallOff);

    // Iso-value of the field function at the primitive boundary (by default 0.5)
    double iso;

    //Size of the primitive boundary (at the Iso-isosurface)
    //Radius for a sphere
    double size;

    //Radius of influence (Field = 0 outside)
    double radius;

    //inverse of iso by the Field function
    double invAtIso;

    // Parameter controlling the slope of the falloff function.
    // The larger n, the steeper the slope of f.
    unsigned int n;
};

typedef std::vector <Field2D *> VectorOfFields;

#endif //TEST_FIELD2D_H
