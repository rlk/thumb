#ifndef FRAME_HPP
#define FRAME_HPP

#include "opengl.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class frame
    {
        GLenum target;
        GLenum format;
        GLuint buffer;

        GLuint color;
        GLuint depth;

        bool has_depth;
        bool has_stencil;

        GLsizei w;
        GLsizei h;

    public:

        frame(GLsizei, GLsizei, GLenum=GL_TEXTURE_2D,
              GLenum=GL_RGBA8, bool=true, bool=false);

        virtual ~frame();

        void bind_color(GLenum=GL_TEXTURE0) const;
        void free_color(GLenum=GL_TEXTURE0) const;
        void bind_depth(GLenum=GL_TEXTURE0) const;
        void free_depth(GLenum=GL_TEXTURE0) const;

        virtual void bind(bool=false) const;
        virtual void free(bool=false) const;

        void init();
        void fini();
    };
}

//-----------------------------------------------------------------------------

#endif
