#ifndef OGL_FRAME_HPP
#define OGL_FRAME_HPP

#include <vector>

#include "ogl-opengl.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class frame
    {
        struct state
        {
            GLuint  o;
            GLint   x;
            GLint   y;
            GLsizei w;
            GLsizei h;

            state(GLuint o, GLint x, GLint y, GLsizei w, GLsizei h)
                : o(o), x(x), y(y), w(w), h(h) { }

            void apply()
            {
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, o);
                glViewport(x, y, w, h);
            }
        };

        static std::vector<state> stack;

    protected:

        static void push(GLuint, GLint, GLint, GLsizei, GLsizei);
        static void pop();

        GLenum target;
        GLenum format;
        GLuint buffer;

        GLuint color;
        GLuint depth;

        bool has_color;
        bool has_depth;
        bool has_stencil;

        GLsizei w;
        GLsizei h;

        void init_cube ();
        void init_color();
        void init_depth();
        void init_frame();

    public:

        frame(GLsizei, GLsizei, GLenum,
              GLenum, bool, bool, bool);

        virtual ~frame();

        void bind_color(GLenum=GL_TEXTURE0) const;
        void free_color(GLenum=GL_TEXTURE0) const;
        void bind_depth(GLenum=GL_TEXTURE0) const;
        void free_depth(GLenum=GL_TEXTURE0) const;

        virtual void bind(double) const;
        virtual void bind(int)    const;
        virtual void bind()       const;
        virtual void free()       const;

        virtual void init();
        virtual void fini();

        GLsizei get_w() const { return w; }
        GLsizei get_h() const { return h; }
    };
}

//-----------------------------------------------------------------------------

#endif
