//
// Created by Loïc Barthe on 06/01/2022.
//

#ifndef TEST_COLORIMAGE_H
#define TEST_COLORIMAGE_H

#include "Color.h"
#include "../Gamut/GamutField2D.h"
#include "../Decale/DecaleDiskField2D.h"
#include <QPixmap>

class ColorImage {
public:

    ColorImage ();

    ColorImage (const QString &fileName, unsigned int decaleId);
    ColorImage (const int w, const int h);

    Color getColor (const unsigned int i, const unsigned int j);
    void setPixelColor (const unsigned int i, const unsigned int j, Color c);

    /**
     * Color the Image in black
     */
    void setBlack ();
    /**
     * Color the Image with the color c
     */
    void setColor (Color c);

    /**
     * Draw the Gamut in the image in black
     */
    void setBlackGamut (GamutField2D *gf);
    /**
     * Draw the Gamut in the image with the gamut color c
     */
    void setColorGamut (GamutField2D *gf, Color c);

    /**
     * Clear the color at the location of the Decale with the background color outside the Gamut and the gaut color inside
     */
    void clearDecaleColorImage(DecaleScalarField2D *decale, ColorImage *imgDecale, GamutField2D *gamut, Color gamutColor, Color bgColor);

    void clearDecaleColorImage(DecaleScalarField2D *decale, GamutField2D *gamut, Color gamutColor, Color bgColor);
    /**
     * Require the Decale UV buffer to be computed. Draw the Decale image deformed with the decale UV buffer
     */
    void addColorImageWithDecaleUV(DecaleScalarField2D *decale, ColorImage *imgDecale);

    /**
     * Compute the QPixmap from the Qimage and return the QPixmap
     */
    QPixmap &getQPixmap ();

    int getWidth();
    int getHeight();
    int getId();
    /**
     * Compute the image ratio height/width
     */
    double getRatioHW();

private:

    QPixmap piximg;
    QImage qimg;

    int width;
    int height;
    unsigned int decaleId;
};

typedef std::vector<ColorImage *> VectorOfColorImages;

#endif //TEST_COLORIMAGE_H
