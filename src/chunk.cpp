
#include "chunk.hpp"

#include <math.h>

#include <OpenSimplexNoise.h>

OpenSimplexNoise::Noise o1(100);

Chunk::Chunk() {
    const int size = 100;
    double* array = new double[size*size*size];

    for (int x=0;x<size;x++) {
        for (int y=0;y<size;y++) {
            for (int z = 0; z < size; z++)
            array[x*size + y*size*size+z] = o1.eval(x,y,z);
        }
    }

    double min = 1000;
    double max = -1000;

    for (int n=0;n<size*size;n++) {
        if (array[n]<min) min = array[n];
        else if (array[n]>max) max = array[n];
    }

    printf("\nmin: %lf, max: %lf \n",min,max);
    
    int i = 0;
    for (int x = 25; x < 75; x++) {
        for (int y = 25; y < 75; y++) {
            for (int z = 50; z < 51; z++) {
                //if (o1.eval(x*0.2,y*0.2,z*0.2) > 0.1) {
                    i++;
                    int index = (z * (w*h) + y * w + x) * 3;
                    this->texData[index] = 255;
                    this->texData[index+1] = 255;
                    this->texData[index+2] = 255;
                //}
            }
        }
    }

    printf("%lf\n", (double)i / (w*h*l));

    glGenTextures(1,&this->tex);
    glBindTexture(GL_TEXTURE_3D,this->tex);
    // Allocate the storage.
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGB8, w, h, l);

    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, w, h, l, GL_RGB, GL_UNSIGNED_BYTE, this->texData);

    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
}

Chunk::~Chunk() {

}

