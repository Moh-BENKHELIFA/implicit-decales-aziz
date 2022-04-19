#define EIGEN_DONT_ALIGN_STATICALLY

#include <iostream>
#include <Eigen/Dense>
#include <Eigen/StdVector>

#include <QImage>
#include <QColor>
#include <QPainter>

#include <interpo-lib/qt/colorQtCompat.hpp>
#include <interpo-lib/qt/paletteQtCompat.hpp>
#include <interpo-lib/qt/palettizedrasterQtCompat.hpp>
#include <interpo-lib/color.hpp>
#include <interpo-lib/inGamutHelper.hpp>
#include <interpo-lib/palettizer.hpp>

#include <QSvgGenerator>



namespace Test_Palettizer{
typedef double Scalar;
}


bool runTest(){
    using namespace Test_Palettizer;

    QImage inImage(QString("./test.jpg"));
    const size_t k = 9;

    const int width  = inImage.width();
    const int height = inImage.height();

    using PalettizerT = Palettizer<Scalar>;
    using PalettizedRasterT = PalettizedRaster<Scalar, PalettizerT::space>;
    PalettizerT palettizer(k, 0.001);
    PalettizedRasterT palettizedRaster;

    using ColorQtCompatT = Color::ColorQtCompat<PalettizerT::space>;


    ////////////////////////////////////////////////////////////////////////////
    prProcessFromQImage(palettizedRaster, inImage, palettizer);
    std::cout << "PalettizedRaster applied in "
              << palettizer.getNbIter()
              << " iterations "
              << std::endl;


    ////////////////////////////////////////////////////////////////////////////
    /// compute palette and store it as image
    /// Store the colors using Palettizer internal type to avoid unecessary
    /// conversion when using the palette
    std::vector<typename PalettizerT::ColorT> palette;
    palettizer.getPalette(palette);
    Color::PaletteQtCompat::Params params;

    QSvgGenerator generator_horizontal;
    generator_horizontal.setFileName("palette_horizontal.svg");
    generator_horizontal.setTitle("SVG Palette Generator");
    generator_horizontal.setDescription("An SVG drawing created by the Palette SVG Generator");
    params.vertical = false;
    Color::PaletteQtCompat::paint(generator_horizontal, palette.begin(), palette.end(), params);

    QSvgGenerator generator;
    generator.setFileName("palette.svg");
    generator.setTitle("SVG Palette Generator");
    generator.setDescription("An SVG drawing created by the Palette SVG Generator");
    params.vertical = true;
    Color::PaletteQtCompat::paint(generator, palette.begin(), palette.end(), params);



    ////////////////////////////////////////////////////////////////////////////
    /// output displacement image
    ///
    {
        QImage segmentImage (width, height, QImage::Format_Indexed8);
        int idx = 0;
        for (typename PalettizerT::ColorT& color : palette){ // set the palette
            segmentImage.setColor(idx, ColorQtCompatT::toQColor<Scalar>(color).rgb());
            idx++;
        }
#pragma omp parallel for collapse(2)
        for (int x = 0; x < width; ++x){
            for (int y = 0; y < height; ++y){
                const typename PalettizedRasterT::Pixel&
                        pixel = palettizedRaster.data()[y*width + x];
                segmentImage.setPixel(x, y, pixel.colorId);
            }
        }

        segmentImage.save("segments.png");        

        QImage reconstructedImage;
        prReconstructFromPaletteToQImage(palettizedRaster, palette, reconstructedImage);
        reconstructedImage.save("reconstructed.png");

        /// Random palette
        ///
        for (typename PalettizerT::ColorT& color : palette)
            color = PalettizerT::ColorT::Random();
        palette.back() = PalettizerT::ColorT::Black();

        QSvgGenerator random_generator;
        random_generator.setFileName("palette_random.svg");
        random_generator.setTitle("SVG Palette Generator");
        random_generator.setDescription("An SVG drawing created by the Palette SVG Generator");
        Color::PaletteQtCompat::paint(random_generator, palette.begin(), palette.end(), params);

        prReconstructFromPaletteToQImage(palettizedRaster, palette, reconstructedImage);
        reconstructedImage.save("reconstructed_random.png");
    }

    return true;
}

int main(int /*argc*/, char */*argv*/[])
{
    using namespace Test_Palettizer;
    using namespace std;
    using Color::InUnitspaceClampingHelper;

    return runTest() ? EXIT_SUCCESS : EXIT_FAILURE;
}
