//
// Created by Loïc Barthe on 25/03/2020.
//

#ifndef TEST_VECTOR2D_H
#define TEST_VECTOR2D_H


class Vector2D {

public:

    Vector2D();
    Vector2D(const Vector2D &v);
    Vector2D(double x, double y);

    //Dot Product
    double operator * (const Vector2D &v);
    Vector2D operator + (const Vector2D &v);
    Vector2D operator - (const Vector2D &v);

    Vector2D operator * (double a);

    double norm ();
    void normalize ();

    void set(double x, double y);

    void copy(const Vector2D &u);

    double x;
    double y;
};


#endif //TEST_VECTOR2D_H
