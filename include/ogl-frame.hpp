#ifndef OGL_FRAME_HPP
#define OGL_FRAME_HPP

#include <vector>

#include <ogl-opengl.hpp>

//-----------------------------------------------------------------------------

namespace ogl
{
    class frame
    {
    public:

        frame(GLsizei, GLsizei, GLenum,
              GLenum, bool, bool, bool);

        virtual ~frame();

        void bind_color(GLenum=GL_TEXTURE0) const;
        void free_color(GLenum=GL_TEXTURE0) const;
        void bind_depth(GLenum=GL_TEXTURE0) const;
        void free_depth(GLenum=GL_TEXTURE0) const;

        virtual void bind(double) const;
        virtual void bind(bool)   const;
        virtual void bind(int)    const;
        virtual void bind()       const;
        virtual void free()       const;
        virtual void free(bool)   const;

        virtual void init();
        virtual void fini();
        virtual void draw();

        GLsizei get_w()     const { return w; }
        GLsizei get_h()     const { return h; }
        GLuint  get_color() const { return color; }
        GLuint  get_depth() const { return depth; }

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

    private:

        static std::vector<GLuint> stack;
    };
}

//-----------------------------------------------------------------------------

#endif
