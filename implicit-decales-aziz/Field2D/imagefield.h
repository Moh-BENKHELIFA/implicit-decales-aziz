#ifndef IMAGEFIELD_H
#define IMAGEFIELD_H

#include "Field2D.h"
#include <QString>
#include <QImage>
#include <queue>

class ImageField : public Field2D
{
public:
    ImageField(const QString &path);
    double eval (double x, double y) override;

protected:
    void calculateDistnaceField();
    bool isInsideTheImage(int i,int j, int w, int h);

    std::vector< std::vector<int> > m;
    QImage img_src;
    int d; //positive max val of field
    int dn;//negative max val of field

    std::queue<QPoint> pixel_coord;
};

#endif // IMAGEFIELD_H
