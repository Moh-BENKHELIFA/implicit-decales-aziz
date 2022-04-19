#include <iostream>
#include <Eigen/Dense>

#include <interpo-lib/color.hpp>
#include <interpo-lib/inGamutHelper.hpp>
#include <interpo-lib/qt/paletteQtCompat.hpp>
#include <interpo-lib/colorvisiondeficiencies.hpp>
#include <interpo-lib/paletteprocessing.hpp>

#include <QSvgGenerator>

namespace Test_Color{
typedef double Scalar;
constexpr Scalar epsilon = 10e-5;
}

template<typename Col1, typename Col2>
bool checkConsistency(const Col1& c1, const Col2& c2){
    bool ok = true;

    using Test_Color::epsilon;


    if ((c1.getRGB() - c2.getRGB()).norm() > epsilon){
        std::cerr << "[RGB] ... FAILED" << std::endl;
        std::cerr << c1.getRGB().transpose() << std::endl;
        std::cerr << c2.getRGB().transpose() << std::endl;
        ok = false;
    }
    else
    if ((c1.getHSV() - c2.getHSV()).norm() > epsilon){
        std::cerr << "[HSV] ... FAILED" << std::endl;
        std::cerr << c1.getHSV().transpose() << std::endl;
        std::cerr << c2.getHSV().transpose() << std::endl;
        ok = false;
    }
    else
    if ((c1.getLAB() - c2.getLAB()).norm() > epsilon){
        std::cerr << "[LAB] ... FAILED" << std::endl;
        std::cerr << c1.getLAB().transpose() << std::endl;
        std::cerr << c2.getLAB().transpose() << std::endl;
        ok = false;
    }
    else
    if ((c1.getLUV() - c2.getLUV()).norm() > epsilon){
        std::cerr << "[LUV] ... FAILED" << std::endl;
        std::cerr << c1.getLUV().transpose() << std::endl;
        std::cerr << c2.getLUV().transpose() << std::endl;
        ok = false;
    }

    return ok;
}

template<typename Col>
bool checkConsistencyFrom(const Col& src){
    using namespace Test_Color;

    RGBColor<Scalar> rgb_color ( src );
    HSVColor<Scalar> hsv_color ( src );
    LABColor<Scalar> lab_color ( src );
    LUVColor<Scalar> luv_color ( src );

    return checkConsistency(rgb_color, hsv_color) &&
           checkConsistency(rgb_color, lab_color) &&
           checkConsistency(hsv_color, lab_color) &&
           checkConsistency(rgb_color, luv_color) &&
           checkConsistency(hsv_color, luv_color) &&
           checkConsistency(lab_color, luv_color);

}

template <int NbCall = 10>
bool runTest(){
    using namespace Test_Color;

    bool ok = true;

    for(int i = 0; i < NbCall && ok; ++i){
        ok =    checkConsistencyFrom (RGBColor<Scalar>::Random()) &&
                checkConsistencyFrom (HSVColor<Scalar>::Random()) &&
                checkConsistencyFrom (LABColor<Scalar>::Random()) &&
                checkConsistencyFrom (LUVColor<Scalar>::Random());
    }

    if (ok)
        std::cout << "All " << NbCall << " conversion tests passed" << std::endl;
    else
        std::cout << "Conversion test failed" << std::endl;

    return ok;
}

template <int NbColors = 10>
bool runPaletteOutputTest(){
    using namespace Test_Color;
    using namespace std;
    using Color::InUnitspaceClampingHelper;

    //std::array< RGBColor<Scalar>, NbColors > container;
    std::vector< RGBColor<Scalar>> container;
    container.resize(NbColors);
    for (auto& color : container)
        color = RGBColor<Scalar>::Random();


    QImage im;
    bool ret =
            Color::PaletteQtCompat::paint(im, container.begin(), container.end());

    if (ret) {
        im.save("out.png");

        QSvgGenerator generator;
        generator.setFileName("out.svg");
        generator.setTitle("SVG Palette Generator");
        generator.setDescription("An SVG drawing created by the Palette SVG Generator");
        ret = Color::PaletteQtCompat::paint(generator, container.begin(), container.end());
    }


    return ret;
}

template <int NbColors = 10>
bool runPaletteProcessingTest(){
    using namespace Test_Color;
    using namespace std;
    using Color::InUnitspaceClampingHelper;

    //std::array< RGBColor<Scalar>, NbColors > container;
    std::vector< RGBColor<Scalar>> container;
    container.resize(NbColors);
    for (auto& color : container)
        color = RGBColor<Scalar>::Random();

    QImage im;
    bool ret =
            Color::PaletteQtCompat::paint(im, container.begin(), container.end());

    if (ret) {
        im.save("out.png");

        QSvgGenerator generator;
        generator.setFileName("out.svg");
        generator.setTitle("SVG Palette Generator");
        generator.setDescription("An SVG drawing created by the Palette SVG Generator");
        ret = Color::PaletteQtCompat::paint(generator, container.begin(), container.end());
    }


    std::vector<int> groupIds;
    int nGroup =
    PaletteProcessing::groupSimilar<0, Color::HSV>(Scalar(0.1), container, groupIds);

    RGBColor<Scalar> maskedColor (RGBColor<Scalar>::White());
    for(int i = 0; i != nGroup; ++i){
        std::vector< RGBColor<Scalar>> containercpy = container;

        for (int j = 0; j != NbColors; ++j)
            if(groupIds[j] != i)
                containercpy[j] = maskedColor;
        bool ret =
                Color::PaletteQtCompat::paint(im, containercpy.begin(), containercpy.end());

        if (ret) {
            im.save(("out_group_") + QString::number(i) + QString(".png"));
        }
    }

    return ret;
}

template <int NbCall>
inline bool testCVD(){

    using namespace Test_Color;
    using namespace std;

    for (int i = 0; i < NbCall; ++i){
        RGBColor<Scalar> mcolor (RGBColor<Scalar>::Random());


        // test conversions to LMS
        {
            RGBColor<Scalar> mcolorBis;
            mcolorBis.setFromUnscaled<Color::LMS_Cat02>(mcolor.getUnscaledAs<Color::LMS_Cat02>());
            if((mcolor.getNative() - mcolorBis.getNative()).norm() > epsilon ){
                cerr << "LMS conversion test failed " << endl;
                cout << "RBG         : " << mcolor.getRGB().transpose() << endl;
                cout << "RBG Bis     : " << mcolorBis.getRGB().transpose() << endl;
                cout << "LMS         : " << mcolor.getUnscaledAs<Color::LMS_Cat02>().transpose() << endl;
            }
        }
    }

    // test color deficiency simulation, by comparing colors with those
    {

        int NbColors = 30;
        std::vector< RGBColor<Scalar>> container, containerDeut, containerProt;
        container.resize(NbColors);
        containerDeut.resize(NbColors);
        containerProt.resize(NbColors);
        //for (auto& color : container){
        for(int i = 0; i != NbColors; ++i) {
            RGBColor<Scalar> c = RGBColor<Scalar>::Random();
            container[i] = c;
            containerDeut[i] = Color::CVD::Rasche2005::well2deuteranopes(c);
            containerProt[i] = Color::CVD::Rasche2005::well2protanopes(c);

            std::cout << containerDeut[i].getNative().transpose() << " - "
                      << containerProt[i].getNative().transpose() << std::endl;
        }

        auto paintPalette = [](const std::vector< RGBColor<Scalar>>&palette,
                               QString path){

            QSvgGenerator generator;
            generator.setFileName(path);
            generator.setTitle("SVG Palette Generator");
            generator.setDescription("An SVG drawing created by the Palette SVG Generator");
            Color::PaletteQtCompat::paint(generator, palette.begin(), palette.end());

        };

        paintPalette(container, "pal_normalvision.svg");
        paintPalette(containerDeut, "pal_deuteranopia.svg");
        paintPalette(containerProt, "pal_protanopia.svg");

    }


    return true;
}


template <int NbCall>
inline bool testHex() {
    using namespace Test_Color;

    bool ok = true;
    for(int i = 0; ok && i < NbCall; ++i){
        LABColor<Scalar> c (LABColor<Scalar>::Random());
        LABColor<Scalar> c2 (LABColor<Scalar>::fromRGBHex(c.getRGBHex()));
        LABColor<Scalar> c3 (RGBColor<Scalar>::fromRGBHex(c.getRGBHex()));
        LUVColor<Scalar> c4 (RGBColor<Scalar>::fromRGBHex(c.getRGBHex()));

        if ((c.getRGB() - c2.getRGB()).norm() > 10e-3)
            ok = false;

        if ((c.getRGB() - c3.getRGB()).norm() > 10e-3)
            ok = false;

        if ((c.getRGB() - c4.getRGB()).norm() > 10e-3)
            ok = false;
    }
    std::cout << "testHex " << (ok ? " PASSED" : " FAILED") << std::endl;
    return ok;
}

int main(int /*argc*/, char */*argv*/[])
{
    using namespace Test_Color;
    using namespace std;
    using Color::InUnitspaceClampingHelper;

    // Generate a random color and print it's value in different spaces
    RGBColor<Scalar> mcolor (RGBColor<Scalar>::Random());
    cout << "RGB: " << mcolor.getRGB().transpose() << endl;
    cout << "HSV: " << mcolor.getHSV().transpose() << endl;
    cout << "LAB: " << mcolor.getLAB().transpose() << endl;
    cout << "LUV: " << mcolor.getLUV().transpose() << endl;

    // modify color to red, expressed in hsv space:
    mcolor.setHSV(typename RGBColor<Scalar>::CVector(0., 1., 1.));
    cout << "RGB: " << mcolor.getRGB().transpose() << endl;
    cout << "HSV: " << mcolor.getHSV().transpose() << endl;
    cout << "LAB: " << mcolor.getLAB().transpose() << endl;
    cout << "LUV: " << mcolor.getLUV().transpose() << endl;

    // check color temperature from
    // http://en.wikipedia.org/wiki/Color_temperature#Categorizing_different_lighting
    mcolor.setRGB(RGBColor<Scalar>::CVector(255, 122,   0));
    cout << "CCT (target = 1700): " << mcolor.computeCCT() << std::endl;
    mcolor.setRGB(RGBColor<Scalar>::CVector(255, 157,  60));
    cout << "CCT (target = 2400): " << mcolor.computeCCT() << std::endl;
    mcolor.setRGB(RGBColor<Scalar>::CVector(255, 170,  84));
    cout << "CCT (target = 2700): " << mcolor.computeCCT() << std::endl;
    mcolor.setRGB(RGBColor<Scalar>::CVector(255, 188, 118));
    cout << "CCT (target = 3200): " << mcolor.computeCCT() << std::endl;
    mcolor.setRGB(RGBColor<Scalar>::CVector(255, 212, 168));
    cout << "CCT (target = 4100): " << mcolor.computeCCT() << std::endl;
    mcolor.setRGB(RGBColor<Scalar>::CVector(255, 228, 206));
    cout << "CCT (target = 5000): " << mcolor.computeCCT() << std::endl;
    mcolor.setRGB(RGBColor<Scalar>::CVector(255, 246, 246));
    cout << "CCT (target = 6200): " << mcolor.computeCCT() << std::endl;
    mcolor.setRGB(RGBColor<Scalar>::CVector(163, 193, 255));
    cout << "CCT (target = 15000): " << mcolor.computeCCT() << std::endl;

    mcolor.setRGB (RGBColor<Scalar>::CVector::Random() * Scalar(2.));
    cout << "Checking gamut mapping" << endl;
    cout << "Initial: " << mcolor.getNative().transpose() << endl;
    cout << "Mapped : "
         << InUnitspaceClampingHelper::process(mcolor).getNative().transpose()
         << endl;


    return (runTest<100000>()
            && runPaletteOutputTest()
            && runPaletteProcessingTest()
            && testHex<50>()
            && testCVD<50>())
            ? EXIT_SUCCESS : EXIT_FAILURE;
}
