/******************************************************************************
 *
 * @file	TextureLoader.h
 *
 * @author	Lincoln Scheer
 * @since	02/01/2022
 *
 * @brief	TextureLoader is a Nelson utility that provides easy interface for
 *              loading, storing and retrieving image data. This class uses the
 *              SOIL2 header-only image library to load images.
 *
 *****************************************************************************/

#ifndef TEXTURE_H
#define TEXTURE_H

#include "NelsonEngine.h"

class TextureLoader {
public:
        int width = 0, height = 0;

        /**
         * Constructor loads image from specified path on class instantiation.
         *
         * \param path
         */
        TextureLoader(const char* path) {
                load(path);
        }

        /**
         * Return member texture variable.
         *
         * \return
         */
        unsigned int getTexture() {
                return texture;
        }


private:
        unsigned int texture;

        /**
         * Loads image from specified path into member texture variable.
         *
         * \param path
         * \return
         */
        void load(const char* path) {
                glGenTextures(1, &this->texture);
                glBindTexture(GL_TEXTURE_2D, this->texture);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGBA);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glGenerateMipmap(GL_TEXTURE_2D);
                SOIL_free_image_data(image);
                glBindTexture(GL_TEXTURE_2D, 0);
        }
};

#endif /* TEXTURE_H */