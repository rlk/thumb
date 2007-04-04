#ifndef FRAME_HPP
#define FRAME_HPP

#include "opengl.hpp"
#include "image.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class frame
    {
        GLsizei w;
        GLsizei h;

        GLenum target;
        GLuint buffer;

        image *color;
        image *depth;

    public:

        frame(GLsizei, GLsizei, GLenum=GL_TEXTURE_2D,
                                GLenum=GL_RGBA8,
                                GLenum=GL_DEPTH_COMPONENT24);
        ~frame();

        void bind_color(GLenum=GL_TEXTURE0) const;
        void free_color(GLenum=GL_TEXTURE0) const;

        void bind(bool=false) const;
        void free(bool=false) const;
        void draw()           const;
        void null()           const;
    };
}

//-----------------------------------------------------------------------------

#endif
