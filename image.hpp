#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <string>

#include "opengl.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class image
    {
    protected:

        GLsizei width;
        GLsizei height;
        GLenum  target;
        GLenum  format;
        GLuint  texture;

    public:

        image();
       ~image();

        void bind(GLenum) const;
        void draw()       const;
    };

    class image_file : public image
    {
    public:
        image_file(std::string);
    };

    class image_snap : public image
    {
    public:
        image_snap();
    };
}

//-----------------------------------------------------------------------------

#endif
