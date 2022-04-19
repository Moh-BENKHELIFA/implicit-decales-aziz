#include "imagefield.h"
#include <iostream>

ImageField::ImageField(const QString &filename)
{ 
    img_src.load(filename);
    calculateDistnaceField();
}

double ImageField::eval(double x, double y)
{
    //return signed distance to the gamut: distance field
    //std::cout<<"x: "<<x<<std::endl;
    return m[x][y];
}

bool ImageField::isInsideTheImage(int i,int j, int w, int h){
    return i>=0 and i<w &&  j>=0 and j<h;
}

void ImageField::calculateDistnaceField()
{
    m.resize(size_t(img_src.width()), std::vector<int>(size_t(img_src.height())));

    for(size_t i = 0; i <m.size(); i++){
        for(size_t j = 0; j<m[0].size(); j++){
            QColor col = img_src.pixelColor(static_cast<int>(i),static_cast<int>(j));
            //set initial values when loading a binary image
            if(col == QColor(Qt::black)){
               m[i][j] = 0;//black - window area - inside gamut
            }else{
               m[i][j] = -1;//white - not projected area - outside of gamut
            }
        }
    }


    //--------------------------------------------------positive side
    d = 1;//iso value - level

    //Manipulate with an image(black - projection)
    //adding pixels that are inside the shape(with value 0) to a stack "coord"
    for(size_t i = 0; i <m.size(); i++){
        for(size_t j = 0; j<m[0].size(); j++){
            if(m[i][j] == 0){
                pixel_coord.push({int(i),int(j)});
            }
        }
    }
    //saving initial number of pixels with value of '0'
    size_t currentLevelSize = pixel_coord.size();

    while(pixel_coord.size() != 0){
        auto &currentCoord = pixel_coord.front();
        for(int i = currentCoord.x() - 1; i <= currentCoord.x() + 1; i++){
            for(int j = currentCoord.y() - 1; j<= currentCoord.y() + 1 ; j++){
                if(isInsideTheImage(int(i),int(j), m.size(), m[0].size()) and m[i][j]==-1){
                      m[i][j] = d;
                      pixel_coord.push({int(i), int(j)}); //pushing new level of neighbours
                }
            }
        }
        //remove 1st coord with value of 0
        pixel_coord.pop();
        currentLevelSize--;

        //next level:
        if(currentLevelSize == 0){
            currentLevelSize = pixel_coord.size();
            d++;
        }
    }

    //----------------------------------------negative side

    //adding negative values - making unsigned distance field
    //adding pixels with value '1' to a stack "coord"
    for(size_t i = 0; i <m.size(); i++){
        for(size_t j = 0; j<m[0].size(); j++){
            if(m[i][j] == 1){
                pixel_coord.push({int(i), int(j)});
            }
        }
    }

    //saving initial number of pixels with value of '0'
    currentLevelSize = pixel_coord.size();
    //distance to negative side - negative iso value
    dn = -1;

    while(pixel_coord.size() != 0){
        auto &currentCoord = pixel_coord.front();
        for(int i = currentCoord.x() - 1; i <= currentCoord.x() + 1; i++){
            for(int j = currentCoord.y() - 1; j<= currentCoord.y() + 1; j++){
                if(isInsideTheImage(int(i),int(j), m.size(), m[0].size()) and m[i][j]==0){
                      m[i][j] = dn;
                      pixel_coord.push({int(i),int(j)}); //pushing new level of neighbours
                }
            }
        }
        //remove 1st coord with value of 1
        pixel_coord.pop();
        currentLevelSize--;

        //next level:
        if(currentLevelSize == 0){
            currentLevelSize = pixel_coord.size();
            dn--;
        }
    }
}
