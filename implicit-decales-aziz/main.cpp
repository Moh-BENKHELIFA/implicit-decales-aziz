#include <iostream>
#include <QApplication>
#include <Qwidget>
#include <QLabel>
#include <QGridLayout>
#include "Tools/ColorImage.h"
#include "Decale/DecaleDiskField2D.h"
#include "Decale/DecaleSquareField2D.h"
#include "Decale/DecaleRoundCornerSquareField2D.h"
#include "Deformer2D/Deformer2D.h"
#include "Deformer2D/Deformer2DMax.h"
#include "Deformer2D/Deformer2DContact.h"
#include "Deformer2D/Deformer2DBlendMax.h"
#include "Deformer2D/Deformer2DBinaryHardContact.h"
#include "Deformer2D/Deformer2DBinaryHardContactMax.h"
#include "SDField2D/SDField2DLinear.h"
#include "SDField2D/SDField2DDisk.h"
#include "Gamut/GamutDeformer2DBinaryHardContact.h"
#include "Gamut/GamutDeformer2DBinaryHardContactMax.h"
#include "Operator2D/Operator2DMax.h"
#include "Operator2D/Operator2DBinaryCleanIntersectionDistance.h"
#include "Operator2D/Operator2DBinaryCleanUnionDistance2D.h"
#include "Deformer2D/Deformer2DBlendContact.h"
#include "GUI-Decale/QWidgetMyDecale.h"
#include <math.h>
#include "Field2D/imagefield.h"


unsigned int wi_width=1024;
unsigned int wi_height=740;

VectorOfDecaleFields fields;
VectorOfColorImages decaleImages;
VectorOfFields gamutComponentFields;
GamutField2D *gamut;
VectorOfDeformers deformers;


/*****************************************************************************/
/***********************************
// Create all the Decales (including their images) and the Gamut
***********************************/
/*****************************************************************************/
void buildFields (){

    /***********************************
    // Controls the slop of the fallOff functions (Gamut and Decales)
    // n = 2 is the smoother, n > 2 makes it sharper
    **********************************/
    int n = 2;

/***********************************
// BEGIN GAMUT
***********************************/

    /***********************************
    // Create the gamut fields
    // ni : plane normalized normals
    // ci : disk centers
    **********************************/
    Vector2D n1(-1., 0.);
    Vector2D n2(0, -1);
    Vector2D n3(1, 0);
    Vector2D n4(0, 1);
    Vector2D n5(-1., 1.);
    n5.normalize();
    Vector2D c1((double) wi_width / 2., (double) wi_height / 2.);
    Vector2D c2(100., 100.);
    Vector2D c3((double) (wi_width - 100), 100);

    /***********************************
    // Radius of the FallOff function of the Gamut
    **********************************/
    double sizeFallOff = 80.;

    /***********************************
    // Create Gamut elements (plans and disk) as Signed Distance Fields (SDF)
    // Store them in a vector of fields
    ***********************************/
    gamutComponentFields.push_back(new SDField2DLinear(n1, 100.));
    gamutComponentFields.push_back(new SDField2DLinear(n2, 100.));
    gamutComponentFields.push_back(new SDField2DLinear(n3, -(double) (wi_width - 100)));
    gamutComponentFields.push_back(new SDField2DLinear(n4, -(double) (wi_height - 100)));
    gamutComponentFields.push_back(new SDField2DLinear(n5, -300));
    gamutComponentFields.push_back(new SDField2DDisk(c1, 90.));
    gamutComponentFields.push_back(new SDField2DDisk(c2, 200.));
    gamutComponentFields.push_back(new SDField2DDisk(c3, 120.));

    /***********************************
    // Combines the gamut elements (SDF) in a composition tree
    // to get the final SDF that will be used to build the gamut field
    ***********************************/
    //Field2D *gamutSDF = new Operator2DMax(gamutComponentFields);

    //QString path = "C:/Users/aniyazov/Pictures/interactive_decal_interface/mainWindow_binaryImage";
    QString path = "C:/Users/mbenkhel/Documents/Internship/dev/Qt/implicit-decales-aziz/Images/gamut_images/test4";
    std::cout<< "Hello"<<std::endl;

    Field2D *gamutSDF = new ImageField(path);


    /***********************************
    // Build a Gamut field from the SDF (Gamut is of compact support)
    // the size for the falloff function is the size from the center to isosurface
    ***********************************/
    gamut = new GamutField2D(gamutSDF, 0, 0, wi_width, wi_height, sizeFallOff, n);

    /***********************************
    // Pre-computation of the gamut field values in a buffer of size wi_width*wi_height
    ***********************************/
    gamut->computeDiscreteField();

/***********************************
// END GAMUT
***********************************/


/***********************************
// BEGIN DECALS
***********************************/

    /***********************************
    // Different Decale sizes in pixels
    **********************************/
    double decaleSize0 = 100.;
    double decaleSize1 = 44.;
    double decaleSize2 = 90.;
    double decaleSize3 = 34.;
    double decaleSize4 = 110.;

    /***********************************
    // Create each Decale with its eventual rotation and its image
    // Store them in a vector of Decale fields and a vector of corresponding (by index) of Decale images
    ***********************************/
    fields.push_back(new DecaleSquareField2D(700., 270., decaleSize0, 0, 0, n));
    decaleImages.push_back(new ColorImage ("../Images/windows.jpg", 0));
    //fields[0]->rotate(M_PI/8.);
    //fields[0]->scale(1, decaleImages[0]->getRatioHW()*1);

    fields.push_back(new DecaleSquareField2D(333., 510., decaleSize1, 1, 0, n));
    decaleImages.push_back(new ColorImage ("../Images/IRIT.png", 1));

//    fields[1]->scale(1.88,decaleImages[1]->getRatioHW()*1.88);  //update the scale


    fields.push_back(new DecaleSquareField2D(360., 290., decaleSize3, 2, 0, n));
    decaleImages.push_back(new ColorImage ("../Images/Spotify.png", 2));

    fields[2]->scale(1,decaleImages[2]->getRatioHW()*1);


    fields.push_back(new DecaleRoundCornerSquareField2D(290., 650., decaleSize1, M_PI/6, 3, 0, n));
    decaleImages.push_back(new ColorImage ("../Images/EIT.png", 3));


    std::cout<<"decal size: "<<fields[1]->getSize()<<std::endl;


    fields.push_back(new DecaleSquareField2D (330.,140.,decaleSize3, 4, 0, n));
    decaleImages.push_back(new ColorImage ("../Images/youtube", 4));

//    fields.push_back(new DecaleDiskField2D (410.,140.,decaleSize3, n));
//    decaleImages.push_back(new ColorImage ("../Images/round-google.png"));

//    fields.push_back(new DecaleDiskField2D (490.,140.,decaleSize3, n));
//    decaleImages.push_back(new ColorImage ("../Images/round-whatsapp.png"));

    /***********************************
    // Pre-compute the field values of each Decale in a buffer
    ***********************************/
    for (int i = 0; i < fields.size(); i++) fields[i]->computeDiscreteField();

/***********************************
// END DECALS
***********************************/

/***********************************
// BEGIN DEFORMERS
***********************************/

    /***********************************
    // Create the deformers
    // Here: contact between Decales and rigid Gamut vs deformed in contact Decales
    // Deformers are stored in a vectors of Deformers
    // the Decales are added to the contact Derformer
    // the Decales are also added to the Gamut Deformer
    ***********************************/
    deformers.push_back(new Deformer2DContact(fields[0], 0, fields[1], 0));
   // deformers.push_back(new Deformer2DMax(fields[0], 0, fields[1], 0));
    for (int i = 2; i < fields.size(); i++) deformers[deformers.size()-1]->addField(fields[i], 0);

    //deformers.push_back(new GamutDeformer2DBinaryHardContact(gamut, fields[0], 1));
    deformers.push_back(new GamutDeformer2DBinaryHardContactMax(gamut, fields[0], 1));
    for (int i = 1; i < fields.size(); i++) deformers[deformers.size()-1]->addField(fields[i], 1);

    /***********************************
    //Apply the Deformers to the field values stored in the top buffers (the one with the larger index)
    //of all the Decales it deforms and store it for each Decale in a new buffer.
    ***********************************/
    for (int i = 0; i < deformers.size(); i++)  deformers[i]->applyToDiscreteFields();

/***********************************
// END DEFORMERS
***********************************/

/***********************************
// BEGIN Compute UV buffers for each Decale
***********************************/

    for (int i = 0; i < fields.size(); i++) fields[i]->computeDiscreteUVField(fields[i]->getNbDiscreteFields() - 1);

/***********************************
// END Compute UV buffers for each Decale
***********************************/
}



/*****************************************************************************/
/***********************************
// main : create the Qapp, create the Widget and its components, create the fields (Gamut and Decales)
// set the Widget elements and initialize the Widget image
***********************************/
/*****************************************************************************/
int main(int argc, char** argv)
{

    QApplication app(argc, argv);
//    app.setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, false );
//    app.setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false );

    QWidgetMyDecale myWidget (wi_width, wi_height, Color("#D3D3D3"));
    QLabel  *label  = new QLabel(&myWidget);
    QGridLayout *gridLayout = new QGridLayout;

    gridLayout->addWidget(label);

    myWidget.setLayout(gridLayout);
    myWidget.setLabel(label);

    buildFields();

    myWidget.setDecales(fields);
    myWidget.setDeformers(deformers);
    myWidget.setDecaleImages(decaleImages);
    myWidget.setGamut(gamut);
    myWidget.setGamutColor(Color(0.,0.,0.));
    myWidget.prepareSolver(gamut);

    /***********************************
    // Compute the Widget image
    ***********************************/
//    myWidget.update();

    myWidget.show();

    return app.exec();
}
