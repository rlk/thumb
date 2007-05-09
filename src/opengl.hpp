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
#include <GL/glx.h>
#include <GL/glext.h>
#define glGetProcAddress(n) glXGetProcAddressARB((GLubyte *) n)
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX 1
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#define glGetProcAddress(n) wglGetProcAddress(n)
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif

//-----------------------------------------------------------------------------

#ifndef __APPLE__

// GL_ARB_multitexture

extern PFNGLACTIVETEXTUREARBPROC           glActiveTextureARB;

// GL_ARB_shader_objects

extern PFNGLGETOBJECTPARAMETERIVARBPROC    glGetObjectParameterivARB;
extern PFNGLCREATEPROGRAMOBJECTARBPROC     glCreateProgramObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC      glCreateShaderObjectARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC        glUseProgramObjectARB;
extern PFNGLVALIDATEPROGRAMARBPROC         glValidateProgramARB;
extern PFNGLCOMPILESHADERARBPROC           glCompileShaderARB;
extern PFNGLDELETEOBJECTARBPROC            glDeleteObjectARB;
extern PFNGLATTACHOBJECTARBPROC            glAttachObjectARB;
extern PFNGLSHADERSOURCEARBPROC            glShaderSourceARB;
extern PFNGLLINKPROGRAMARBPROC             glLinkProgramARB;
extern PFNGLGETINFOLOGARBPROC              glGetInfoLogARB;

extern PFNGLGETUNIFORMLOCATIONARBPROC      glGetUniformLocationARB;
extern PFNGLUNIFORM1IARBPROC               glUniform1iARB;
extern PFNGLUNIFORM1FARBPROC               glUniform1fARB;
extern PFNGLUNIFORM2FARBPROC               glUniform2fARB;
extern PFNGLUNIFORM3FARBPROC               glUniform3fARB;
extern PFNGLUNIFORM4FARBPROC               glUniform4fARB;

// GL_ARB_vertex_shader

extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArrayARB;
extern PFNGLVERTEXATTRIBPOINTERARBPROC     glVertexAttribPointerARB;
extern PFNGLBINDATTRIBLOCATIONARBPROC      glBindAttribLocationARB;

// GL_EXT_framebuffer_object

extern PFNGLGENFRAMEBUFFERSEXTPROC         glGenFramebuffersEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC         glBindFramebufferEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC      glDeleteFramebuffersEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    glFramebufferTexture2DEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  glCheckFramebufferStatusEXT;

// GL_ARB_vertex_buffer_object

extern PFNGLGENBUFFERSARBPROC              glGenBuffersARB;
extern PFNGLBINDBUFFERARBPROC              glBindBufferARB;
extern PFNGLMAPBUFFERARBPROC               glMapBufferARB;
extern PFNGLBUFFERDATAARBPROC              glBufferDataARB;
extern PFNGLUNMAPBUFFERARBPROC             glUnmapBufferARB;
extern PFNGLBUFFERSUBDATAARBPROC           glBufferSubDataARB;
extern PFNGLDELETEBUFFERSARBPROC           glDeleteBuffersARB;

// GL_EXT_draw_range_elements

extern PFNGLDRAWRANGEELEMENTSEXTPROC       glDrawRangeElementsEXT;

#endif

//-----------------------------------------------------------------------------

#ifndef NDEBUG
#define OGLCK() ogl::check_err(__FILE__, __LINE__)
#else
#define OGLCK() {}
#endif

namespace ogl
{
    extern bool has_multitexture;
    extern bool has_shader;
    extern bool has_fbo;
    extern bool has_vbo;
    extern bool has_dre;

    extern int  do_shadows;
    extern bool do_z_only;
    extern bool do_reflect;
    extern bool do_refract;

    void check_err(const char *, int);
    bool check_ext(const char *);

    void init();
}

//-----------------------------------------------------------------------------

namespace ogl
{
    class fbo
    {
        GLuint frame;
        GLuint color;
        GLuint depth;

        GLsizei w;
        GLsizei h;

        GLint fb_cache[1];
        GLint vp_cache[4];

    public:

        fbo(GLint, GLint, GLsizei, GLsizei);
       ~fbo();

        void push_frame(bool=false);
        void  pop_frame();

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
    void get_framebuffer(GLint[1], GLint[4]);
    void set_framebuffer(GLint[1], GLint[4]);

    void draw_axes();
}

//-----------------------------------------------------------------------------

#endif
