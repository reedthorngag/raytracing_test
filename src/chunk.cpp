
#include "chunk.hpp"

Chunk::Chunk() {
    
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            for (int z = 0; z < l; z++) {
                
            }
        }
    }

    glGenTextures(1,&this->tex);
    glBindTexture(GL_TEXTURE_2D_ARRAY,this->tex);
    // Allocate the storage.
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, w, h, l);

    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, w, h, l, GL_RGBA, GL_UNSIGNED_BYTE, this->texData);
}

Chunk::~Chunk() {

}

