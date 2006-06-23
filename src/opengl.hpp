//  Copyright (C) 2005 Robert Kooima
//
//  THUMB is free software; you can redistribute it and/or modify it under
//  the terms of  the GNU General Public License as  published by the Free
//  Software  Foundation;  either version 2  of the  License,  or (at your
//  option) any later version.
//
//  This program  is distributed in the  hope that it will  be useful, but
//  WITHOUT   ANY  WARRANTY;   without  even   the  implied   warranty  of
//  MERCHANTABILITY  or FITNESS  FOR A  PARTICULAR PURPOSE.   See  the GNU
//  General Public License for more details.

#ifndef GL_HPP
#define GL_HPP

#include <string>

//-----------------------------------------------------------------------------

#ifdef __linux__
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include "glext.h"
#define glGetProcAddress(n) glXGetProcAddressARB((GLubyte *) n)
#endif

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "glext.h"
#define glGetProcAddress(n) wglGetProcAddress(n)
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

//-----------------------------------------------------------------------------

#ifndef __APPLE__
extern PFNGLACTIVETEXTUREARBPROC              glActiveTextureARB;

extern PFNGLGETOBJECTPARAMETERIVARBPROC       glGetObjectParameterivARB;
extern PFNGLBINDATTRIBLOCATIONARBPROC         glBindAttribLocationARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC           glUseProgramObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC         glCreateShaderObjectARB;
extern PFNGLCREATEPROGRAMOBJECTARBPROC        glCreateProgramObjectARB;
extern PFNGLVALIDATEPROGRAMARBPROC            glValidateProgramARB;
extern PFNGLSHADERSOURCEARBPROC               glShaderSourceARB;
extern PFNGLCOMPILESHADERARBPROC              glCompileShaderARB;
extern PFNGLATTACHOBJECTARBPROC               glAttachObjectARB;
extern PFNGLLINKPROGRAMARBPROC                glLinkProgramARB;
extern PFNGLGETINFOLOGARBPROC                 glGetInfoLogARB;
extern PFNGLDELETEOBJECTARBPROC               glDeleteObjectARB;

extern PFNGLGETUNIFORMLOCATIONARBPROC         glGetUniformLocationARB;
extern PFNGLUNIFORM1IARBPROC                  glUniform1iARB;
extern PFNGLUNIFORM1FARBPROC                  glUniform1fARB;
extern PFNGLUNIFORM2FARBPROC                  glUniform2fARB;
extern PFNGLUNIFORM3FARBPROC                  glUniform3fARB;
extern PFNGLUNIFORM4FARBPROC                  glUniform4fARB;

extern PFNGLGENFRAMEBUFFERSEXTPROC            glGenFramebuffersEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC            glBindFramebufferEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC         glDeleteFramebuffersEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC       glFramebufferTexture2DEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC     glCheckFramebufferStatusEXT;

extern PFNGLGENBUFFERSARBPROC                 glGenBuffersARB;
extern PFNGLBINDBUFFERARBPROC                 glBindBufferARB;
extern PFNGLMAPBUFFERARBPROC                  glMapBufferARB;
extern PFNGLUNMAPBUFFERARBPROC                glUnmapBufferARB;
extern PFNGLBUFFERDATAARBPROC                 glBufferDataARB;
extern PFNGLDELETEBUFFERSARBPROC              glDeleteBuffersARB;
#endif

void  init_ogl();
void check_ogl(const char *, int);

#ifndef NDEBUG
#define GL_CHECK() check_ogl(__FILE__, __LINE__)
#else
#define GL_CHECK() {}
#endif

//-----------------------------------------------------------------------------

namespace ogl
{
    class fbo
    {
        GLuint frame;
        GLuint color;
        GLuint depth;

        GLsizei _w;
        GLsizei _h;

    public:

        fbo(GLint, GLint, GLsizei, GLsizei);
       ~fbo();

        void bind_frame()       const;
        void bind_color(GLenum) const;
        void bind_depth(GLenum) const;
    };
}

//-----------------------------------------------------------------------------

namespace ogl
{
    class vbo
    {
        GLuint buffer;

    public:

        vbo(GLenum, GLenum, GLsizei);
       ~vbo();

        void bind(GLenum) const;
    };
}

//-----------------------------------------------------------------------------

namespace ogl
{
    class shader
    {
        void check_log(GLhandleARB);

        GLhandleARB vert;
        GLhandleARB frag;
        GLhandleARB prog;

    public:

        shader(std::string, std::string);
       ~shader();

        void bind() const;

        void uniform(std::string, int)                        const;
        void uniform(std::string, float)                      const;
        void uniform(std::string, float, float)               const;
        void uniform(std::string, float, float, float)        const;
        void uniform(std::string, float, float, float, float) const;
    };
}

//-----------------------------------------------------------------------------

namespace ogl
{
    void get_framebuffer(GLint[1], GLint[4]);
    void set_framebuffer(GLint[1], GLint[4]);

    void draw_disc(int);
    void draw_cube();
    void draw_axes();
}

//-----------------------------------------------------------------------------

#endif
