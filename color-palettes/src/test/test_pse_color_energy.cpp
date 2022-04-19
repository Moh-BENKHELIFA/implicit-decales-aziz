#include <QtGlobal> //QT_VERSION

#include <iostream>
#include <Eigen/Dense>

#include <interpo-lib/color.hpp>
#include <interpo-lib/inGamutHelper.hpp>
#include <interpo-lib/discrepancy/color_energy.hpp>
#include <interpo-lib/qt/colorQtCompat.hpp>

#include <QCustomPlot/qcustomplot.h> // use colormaps gradients

#include <QImage>
#include <QFile>
#include <QTextStream>

namespace Test_Energy{
typedef double Scalar;
constexpr Color::Space space = Color::LAB;
using ColorT = Color::ColorBase<Scalar, space>;
using DiscrT = EnergyDiscrepancy::RGBMaxDiscrepancyFunctor<ColorT>;
using Clamp  = Color::InXXXspaceClampingHelper<Scalar,Color::RGB>;
}




/*!
 * Slice the Lab space along the luminance axis, compute consumption values and
 * output them as gnuplot data file
 *
 * Gnuplot code:
 * \code
 * set pm3d; set palette
 * unset surface
 * splot "out_001.txt" matrix
 * \endcode
 */
void sliceToGnuplot(unsigned int w, unsigned int h, int nbSlices) {
    using namespace Test_Energy;

    const Scalar ws = w, hs = h;

#pragma omp parallel for
    for (int i = 0; i < nbSlices; ++i){
        DiscrT nrjFunctor;   // object computing the energy consumption
        Clamp  clampFunctor; // object validating the colors
        ColorT c;

        Scalar lum = Scalar(i) / (nbSlices); // compute luminance value, avoid 0 and 1

        QFile file(QString("out_") +
                   QString::number(i).rightJustified(3, '0') +
                   QString(".txt"));
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)){

            QTextStream out(&file);

            for (unsigned int y = 0; y<h; ++y){
                for (unsigned int x = 0; x<w; ++x){
                    c = ColorT(lum, Scalar(x)/ws, Scalar(y)/hs);
                    int energy = 0;
                    if (clampFunctor.isValid(c)) {
                        energy = nrjFunctor.eval({{c}}, 0);
                    }
                    out << energy << "\t" ;
                }
                out << "\n";
            }

            out.flush();
            file.close();
        }
    }
}

/*!
 * Slice the Lab space along the luminance axis, compute consumption values and
 * output them as image
 */
void sliceToQImage(unsigned int w, unsigned int h, int nbSlices) {
    using namespace Test_Energy;

    const Scalar ws = w, hs = h;

#pragma omp parallel for
    for (int i = 0; i < nbSlices; ++i){
        DiscrT nrjFunctor;   // object computing the energy consumption
        Clamp  clampFunctor; // object validating the colors
        ColorT c;

        Scalar lum = Scalar(i+1) / (nbSlices+1); // compute luminance value, avoid 0 and 1
        QImage output (w*2, h, QImage::Format_ARGB32_Premultiplied);
        output.fill(QColor(255, 255, 255, 0)); // set transparent background

        QCPColorGradient gradient (QCPColorGradient::gpHot);
        gradient.setLevelCount(255);
        QCPRange gradientRange(0, 255);


        for (unsigned int y = 0; y<h; ++y){
            for (unsigned int x = 0; x<w; ++x){
                c = ColorT(lum, Scalar(x)/ws, Scalar(y)/hs);

                if (clampFunctor.isValid(c)) {
                    Scalar energy = nrjFunctor.eval({{c}}, 0);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    output.setPixelColor(x,y,Color::ColorQtCompat<space>::toQColor(c));
    output.setPixelColor(w+x,y,gradient.color(energy, gradientRange));
#else
    output.setPixel(x,y,Color::ColorQtCompat<space>::toQColor(c).rgb());
    output.setPixel(w+x,y,gradient.color(energy, gradientRange));
#endif


                }
            }
        }

        output.save(QString("out_") +
                    QString::number(i).rightJustified(8, '0') +
                    QString(".png"));
    }
}


int main(int /*argc*/, char */*argv*/[])
{
    using namespace Test_Energy;

    const int nbSlices = 20;

    sliceToQImage(500, 500, nbSlices);
    sliceToGnuplot(100, 100, nbSlices);


    return EXIT_SUCCESS;
}
