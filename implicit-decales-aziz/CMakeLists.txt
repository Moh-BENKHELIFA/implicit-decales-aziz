cmake_minimum_required(VERSION 3.8)
project(Test)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp Decale/DecaleScalarField2D.h Decale/DecaleScalarField2D.cpp
    Decale/DecaleDiskField2D.h Decale/DecaleDiskField2D.cpp Tools/Vector2D.cpp Tools/Vector2D.h
    Decale/DecaleSquareField2D.cpp Decale/DecaleSquareField2D.h Deformer2D/Deformer2D.cpp Deformer2D/Deformer2D.h
    Deformer2D/Deformer2DMax.cpp Deformer2D/Deformer2DMax.h Deformer2D/Deformer2DContact.cpp
    Deformer2D/Deformer2DContact.h Deformer2D/Deformer2DBlendMax.cpp Deformer2D/Deformer2DBlendMax.h
    Deformer2D/Deformer2DBinaryHardContact.cpp Deformer2D/Deformer2DBinaryHardContact.h
    Deformer2D/Deformer2DBinaryHardContactMax.cpp Deformer2D/Deformer2DBinaryHardContactMax.h
    Field2D/Field2D.cpp Field2D/Field2D.h SDField2D/SDField2DLinear.cpp SDField2D/SDField2DLinear.h
    Gamut/GamutField2D.cpp Gamut/GamutField2D.h Gamut/GamutDeformer2DBinaryHardContact.cpp
    Gamut/GamutDeformer2DBinaryHardContact.h Operator2D/Operator2DMax.cpp Operator2D/Operator2DMax.h
    Operator2D/Operator2DBinaryCleanUnionDistance2D.cpp Operator2D/Operator2DBinaryCleanUnionDistance2D.h
    Operator2D/Operator2DBinaryCleanIntersectionDistance.cpp Operator2D/Operator2DBinaryCleanIntersectionDistance.h
    Operator2D/Operator2DBinary.cpp Operator2D/Operator2DBinary.h Operator2D/Operator2DNary.cpp
    Operator2D/Operator2DNary.h Operator2D/Operator2D.cpp Operator2D/Operator2D.h
    Deformer2D/Deformer2DBlendContact.cpp Deformer2D/Deformer2DBlendContact.h GUI-Decale/QWidgetMyWidget.cpp
    GUI-Decale/QWidgetMyWidget.h GUI-Decale/QWidgetMyDecale.cpp GUI-Decale/QWidgetMyDecale.h
    Decale/DecaleRoundCornerSquareField2D.cpp Decale/DecaleRoundCornerSquareField2D.h SDField2D/SDField2DDisk.cpp
    SDField2D/SDField2DDisk.h Tools/ColorImage.cpp Tools/ColorImage.h Tools/Color.cpp Tools/Color.h
    Gamut/GamutDeformer2DBinaryHardContactMax.cpp Gamut/GamutDeformer2DBinaryHardContactMax.h
    Field2D/imagefield.h Field2D/imagefield.cpp Solver/genericsolver.cpp Solver/genericsolver.h
    Solver/mydecalsolver.cpp Solver/mydecalsolver.h Solver/constraints.cpp Solver/constraints.h Solver/scene.cpp
    Solver/scene.h)

#set(Qt6_DIR "/Users/lbarthe/Developper/Qt5-9/5.9.2/clang_64/lib/cmake/QT5")
set(Qt6_DIR "C:/Qt/6.2.4/mingw_64/lib/cmake/Qt6")

#find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
find_package(pse REQUIRED)

add_executable(Test ${SOURCE_FILES})

target_link_libraries(Test Qt6::Core Qt6::Widgets PSE::pse)


