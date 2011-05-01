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

#ifndef OGL_OPENGL_HPP
#define OGL_OPENGL_HPP

//-----------------------------------------------------------------------------

#ifdef __linux__
/*
#include <gl.h>
#include <glx.h>
#include <glext.h>
*/
/*
#include "/usr/share/doc/NVIDIA_GLX-1.0/include/GL/gl.h"
#include "/usr/share/doc/NVIDIA_GLX-1.0/include/GL/glx.h"
#include "/usr/share/doc/NVIDIA_GLX-1.0/include/GL/glext.h"
*/
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
//#define glGetProcAddress(n) glXGetProcAddressARB((GLubyte *) n)
#define glGetProcAddress(n) glXGetProcAddress((GLubyte *) n)
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

extern PFNGLACTIVETEXTUREARBPROC            glActiveTexture_;

// GL_ARB_shader_objects

extern PFNGLGETOBJECTPARAMETERIVARBPROC     glGetObjectParameterivARB;
extern PFNGLCREATEPROGRAMOBJECTARBPROC      glCreateProgramObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC       glCreateShaderObjectARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC         glUseProgramObjectARB;
extern PFNGLVALIDATEPROGRAMARBPROC          glValidateProgramARB;
extern PFNGLCOMPILESHADERARBPROC            glCompileShaderARB;
extern PFNGLDELETEOBJECTARBPROC             glDeleteObjectARB;
extern PFNGLATTACHOBJECTARBPROC             glAttachObjectARB;
extern PFNGLSHADERSOURCEARBPROC             glShaderSourceARB;
extern PFNGLLINKPROGRAMARBPROC              glLinkProgramARB;
extern PFNGLGETINFOLOGARBPROC               glGetInfoLogARB;

extern PFNGLGETUNIFORMLOCATIONARBPROC       glGetUniformLocationARB;
extern PFNGLUNIFORM1IARBPROC                glUniform1iARB;
extern PFNGLUNIFORM1FARBPROC                glUniform1fARB;
extern PFNGLUNIFORM2FARBPROC                glUniform2fARB;
extern PFNGLUNIFORM3FARBPROC                glUniform3fARB;
extern PFNGLUNIFORM4FARBPROC                glUniform4fARB;
extern PFNGLUNIFORM1FVARBPROC               glUniform1fvARB;
extern PFNGLUNIFORM2FVARBPROC               glUniform2fvARB;
extern PFNGLUNIFORM3FVARBPROC               glUniform3fvARB;
extern PFNGLUNIFORM4FVARBPROC               glUniform4fvARB;
extern PFNGLUNIFORMMATRIX3FVARBPROC         glUniformMatrix3fvARB;
extern PFNGLUNIFORMMATRIX4FVARBPROC         glUniformMatrix4fvARB;

// GL_ARB_vertex_shader

extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARB;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC  glEnableVertexAttribArrayARB;
extern PFNGLVERTEXATTRIBPOINTERARBPROC      glVertexAttribPointerARB;
extern PFNGLBINDATTRIBLOCATIONARBPROC       glBindAttribLocationARB;

// GL_EXT_framebuffer_object

extern PFNGLBINDRENDERBUFFEREXTPROC         glBindRenderbufferEXT;
extern PFNGLGENRENDERBUFFERSEXTPROC         glGenRenderbuffersEXT;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC      glRenderbufferStorageEXT;
extern PFNGLDELETERENDERBUFFERSEXTPROC      glDeleteRenderbuffersEXT;

extern PFNGLGENFRAMEBUFFERSEXTPROC          glGenFramebuffersEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC          glBindFramebufferEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC       glDeleteFramebuffersEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC     glFramebufferTexture2DEXT;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC  glFramebufferRenderbufferEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC   glCheckFramebufferStatusEXT;

// GL_ARB_vertex_buffer_object

extern PFNGLGENBUFFERSARBPROC               glGenBuffersARB;
extern PFNGLBINDBUFFERARBPROC               glBindBufferARB;
extern PFNGLMAPBUFFERARBPROC                glMapBufferARB;
extern PFNGLBUFFERDATAARBPROC               glBufferDataARB;
extern PFNGLUNMAPBUFFERARBPROC              glUnmapBufferARB;
extern PFNGLBUFFERSUBDATAARBPROC            glBufferSubDataARB;
extern PFNGLDELETEBUFFERSARBPROC            glDeleteBuffersARB;

// GL_EXT_draw_range_elements

extern PFNGLDRAWRANGEELEMENTSEXTPROC        glDrawRangeElementsEXT;

#else
#define glActiveTexture_ glActiveTextureARB
#endif

//-----------------------------------------------------------------------------

#ifndef NDEBUG
#define OGLCK() ogl::check_err(__FILE__, __LINE__)
#else
#define OGLCK() {}
#endif

namespace ogl
{
    extern bool has_depth_stencil;
    extern bool has_multitexture;
    extern bool has_multisample;
    extern bool has_anisotropic;
    extern bool has_glsl;
    extern bool has_s3tc;
    extern bool has_fbo;
    extern bool has_vbo;
    extern bool has_dre;

    extern int  max_anisotropy;

    extern int  do_shadow;
    extern bool do_z_only;
    extern bool do_texture_compression;
    extern bool do_hdr_tonemap;
    extern bool do_hdr_bloom;

    void check_err(const char *, int);
    bool check_ext(const char *);

    void init(bool);

    void curr_texture(GLenum);
    void bind_texture(GLenum, GLenum, GLuint);
    void xfrm_texture(GLenum, const GLdouble *);
    void free_texture();

    void line_state_init();
    void line_state_fini();
}

//-----------------------------------------------------------------------------

#endif
