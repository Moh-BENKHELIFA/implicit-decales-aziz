#include <iostream>
#include <Eigen/Dense>

#include <Functionnal/functionnal.h>
#include <Functionnal/constrainedBezier.h>

namespace Test_Bezier{
typedef double Scalar;
enum{Dim=2};
enum{Degree=5};

typedef Functionnal::Bezier<Scalar, Degree, Dim> BezierCurve;
typedef Functionnal::BezierMap<Scalar, Degree, Dim> BezierMap;
typedef Functionnal::ConstrainedBezierMap<Scalar, Degree+2, Dim> ConstrBezierMap;

}


int main(int /*argc*/, char */*argv*/[])
{
    using namespace Test_Bezier;

    const int nbSample = 100;

    // build a first bezier curve
    BezierCurve bezier;
    bezier.coeffs = BezierCurve::CoeffType::Random();

    bezier.initCoeffs();

    std::cout<<"Control points" << std::endl;
    std::cout << bezier << std::endl;

    {
        std::cout<<"Unconstrained curve" << std::endl;
        BezierCurve::EmbeddedVectorType input;
        BezierCurve::EmbeddingVectorType result;

        for (int t = 0; t != nbSample; ++t){
            input << Scalar(t)/Scalar(nbSample-1);
            result = bezier.eval( input );

            std::cout << result.transpose() << std::endl;
        }
    }


    // extend using a constrained BezierMap
    ConstrBezierMap cBezier ( bezier.coeffs.data() );
    cBezier.startPoint << 0.0, 0.0;
    cBezier.endPoint   << 1.0, 1.0;


    std::cout<<"Constrained control points" << std::endl;
    std::cout << cBezier << std::endl;

    {
//        std::cout<<"Constrained curve" << std::endl;
//        ConstrBezierMap::EmbeddedVectorType input;
//        ConstrBezierMap::EmbeddingVectorType result;

//        for (int t = 0; t != nbSample; ++t){
//            input << Scalar(t)/Scalar(nbSample-1);
//            result = cBezier.eval( input );

//            std::cout << result.transpose() << std::endl;
//        }
    }

    // test map to object conversion
    BezierMap bezierMap ( bezier.coeffs.data() );
    BezierCurve cBezierCopy = bezierMap.asFunctionnal();
    std::cout << "Autonomous copy of Constrained control points"<<std::endl;
    std::cout << cBezierCopy << std::endl;

    // test derivative computation
    BezierCurve::Derivative bezierDer = cBezierCopy.derivative();
    std::cout << "Derivative"<<std::endl;
    std::cout << bezierDer << std::endl;

    // compute stretch
    {
        std::cout<<"Bezier curve stretch" << std::endl;
        ConstrBezierMap::EmbeddedVectorType input;
        ConstrBezierMap::EmbeddingVectorType result;

        for (int t = 0; t != nbSample; ++t){
            input << Scalar(t)/Scalar(nbSample-1);
            std::cout << input(0) << " " << bezierDer.eval( input ).norm() << std::endl;
        }
    }

    return EXIT_SUCCESS;
}
