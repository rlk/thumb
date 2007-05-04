#ifndef FRAME_HPP
#define FRAME_HPP

#include "opengl.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class frame
    {
        GLenum target;
        GLuint buffer;

        GLuint color;
        GLuint depth;

        GLenum color_format;
        GLenum depth_format;

        GLsizei w;
        GLsizei h;

    public:

        frame(GLsizei, GLsizei, GLenum=GL_TEXTURE_2D,
                                GLenum=GL_RGBA8,
                                GLenum=GL_DEPTH_COMPONENT24);
        ~frame();

        void bind_color(GLenum=GL_TEXTURE0) const;
        void free_color(GLenum=GL_TEXTURE0) const;
        void bind_depth(GLenum=GL_TEXTURE0) const;
        void free_depth(GLenum=GL_TEXTURE0) const;

        void bind(bool=false) const;
        void free(bool=false) const;

        void init();
        void fini();
    };
}

//-----------------------------------------------------------------------------

#endif
