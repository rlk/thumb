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

#include <stdexcept>
#include <iostream>
#include <cassert>
#include <sstream>
#include <fstream>

#include "ogl-opengl.hpp"
#include "util.hpp"
#include "app-conf.hpp"

//-----------------------------------------------------------------------------

bool ogl::has_multitexture;
bool ogl::has_shader;
bool ogl::has_s3tc;
bool ogl::has_fbo;
bool ogl::has_vbo;
bool ogl::has_dre;

int  ogl::do_shadows;
bool ogl::do_z_only;
bool ogl::do_reflect;
bool ogl::do_refract;
bool ogl::do_texture_compression;
bool ogl::do_hdr_tonemap;
bool ogl::do_hdr_bloom;

//-----------------------------------------------------------------------------

#ifdef __APPLE__
#define PROC(t, n) { }
#else
#define PROC(t, n) if (!(n = (t) glGetProcAddress(#n))) \
                         throw std::runtime_error(#n)

// GL_ARB_multitexture

PFNGLACTIVETEXTUREARBPROC            glActiveTextureARB;

// GL_ARB_shader_objects

PFNGLGETOBJECTPARAMETERIVARBPROC     glGetObjectParameterivARB;
PFNGLCREATEPROGRAMOBJECTARBPROC      glCreateProgramObjectARB;
PFNGLCREATESHADEROBJECTARBPROC       glCreateShaderObjectARB;
PFNGLUSEPROGRAMOBJECTARBPROC         glUseProgramObjectARB;
PFNGLVALIDATEPROGRAMARBPROC          glValidateProgramARB;
PFNGLCOMPILESHADERARBPROC            glCompileShaderARB;
PFNGLDELETEOBJECTARBPROC             glDeleteObjectARB;
PFNGLATTACHOBJECTARBPROC             glAttachObjectARB;
PFNGLSHADERSOURCEARBPROC             glShaderSourceARB;
PFNGLLINKPROGRAMARBPROC              glLinkProgramARB;
PFNGLGETINFOLOGARBPROC               glGetInfoLogARB;

PFNGLGETUNIFORMLOCATIONARBPROC       glGetUniformLocationARB;
PFNGLUNIFORM1IARBPROC                glUniform1iARB;
PFNGLUNIFORM1FARBPROC                glUniform1fARB;
PFNGLUNIFORM2FARBPROC                glUniform2fARB;
PFNGLUNIFORM3FARBPROC                glUniform3fARB;
PFNGLUNIFORM4FARBPROC                glUniform4fARB;
PFNGLUNIFORMMATRIX4FVARBPROC         glUniformMatrix4fvARB;

// GL_ARB_vertex_shader

PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARB;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC  glEnableVertexAttribArrayARB;
PFNGLVERTEXATTRIBPOINTERARBPROC      glVertexAttribPointerARB;
PFNGLBINDATTRIBLOCATIONARBPROC       glBindAttribLocationARB;

// GL_EXT_framebuffer_object

PFNGLBINDRENDERBUFFEREXTPROC         glBindRenderbufferEXT;
PFNGLGENRENDERBUFFERSEXTPROC         glGenRenderbuffersEXT;
PFNGLRENDERBUFFERSTORAGEEXTPROC      glRenderbufferStorageEXT;
PFNGLDELETERENDERBUFFERSEXTPROC      glDeleteRenderbuffersEXT;

PFNGLGENFRAMEBUFFERSEXTPROC          glGenFramebuffersEXT;
PFNGLBINDFRAMEBUFFEREXTPROC          glBindFramebufferEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC       glDeleteFramebuffersEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC     glFramebufferTexture2DEXT;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC  glFramebufferRenderbufferEXT;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC   glCheckFramebufferStatusEXT;

// GL_ARB_vertex_buffer_object

PFNGLGENBUFFERSARBPROC               glGenBuffersARB;
PFNGLBINDBUFFERARBPROC               glBindBufferARB;
PFNGLMAPBUFFERARBPROC                glMapBufferARB;
PFNGLBUFFERDATAARBPROC               glBufferDataARB;
PFNGLUNMAPBUFFERARBPROC              glUnmapBufferARB;
PFNGLBUFFERSUBDATAARBPROC            glBufferSubDataARB;
PFNGLDELETEBUFFERSARBPROC            glDeleteBuffersARB;

// GL_EXT_draw_range_elements

PFNGLDRAWRANGEELEMENTSEXTPROC        glDrawRangeElementsEXT;

#endif

//-----------------------------------------------------------------------------

bool ogl::check_ext(const char *needle)
{
    const GLubyte *haystack, *c;

    // Allow the configuration file to override OpenGL (dangerous).

    std::string option = ::conf->get_s(needle);

    if      (option == "true")  return true;
    else if (option == "false") return false;

    // Search for the given string in the OpenGL extension strings.

    for (haystack = glGetString(GL_EXTENSIONS); *haystack; haystack++)
    {
        for (c = (const GLubyte *) needle; *c && *haystack; c++, haystack++)
            if (*c != *haystack)
                break;

        if ((*c == 0) && (*haystack == ' ' || *haystack == '\0'))
            return true;
    }

    return false;
}

void ogl::check_err(const char *file, int line)
{
    GLenum err;

    if ((err = glGetError()) != GL_NO_ERROR)
    {
        std::ostringstream str;

        str << file << ":"
            << line << " ";

        switch (err)
        {
        case GL_INVALID_ENUM:      str << "Invalid enumerant"; break;
        case GL_INVALID_VALUE:     str << "Invalid value";     break;
        case GL_INVALID_OPERATION: str << "Invalid operation"; break;
        case GL_STACK_OVERFLOW:    str << "Stack overflow";    break;
        case GL_OUT_OF_MEMORY:     str << "Out of memory";     break;
        case GL_TABLE_TOO_LARGE:   str << "Table too large";   break;
        }

        str << std::endl;

        std::cerr << str;

        throw std::runtime_error(str.str().c_str());
    }
}

//-----------------------------------------------------------------------------
// WGL swap interval

#ifdef _WIN32

#include <GL/wglext.h>

static void sync(int interval)
{
    PFNWGLSWAPINTERVALEXTPROC _wglSwapInvervalEXT;

    if ((_wglSwapInvervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
          wglGetProcAddress("wglSwapIntervalEXT")))
         _wglSwapInvervalEXT(interval);
}

#endif

//-----------------------------------------------------------------------------
// GLX swap interval 

#ifdef __linux__

static void sync(int interval)
{
    PFNGLXSWAPINTERVALSGIPROC _glXSwapInvervalSGI;

    if ((_glXSwapInvervalSGI = (PFNGLXSWAPINTERVALSGIPROC)
          glXGetProcAddress((const GLubyte *) "glXSwapIntervalSGI")))
         _glXSwapInvervalSGI(interval);
}

#endif

//-----------------------------------------------------------------------------
// CGL swap interval

#ifdef __APPLE__

#include <OpenGL/OpenGL.h>

static void sync(int interval)
{
    CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &interval);
}

#endif

//-----------------------------------------------------------------------------

void ogl::init()
{
    // Query GL capabilities.

    has_multitexture = check_ext("ARB_multitexture");
    has_shader       = check_ext("ARB_shader_objects");
    has_shader      &= check_ext("ARB_vertex_shader");
    has_shader      &= check_ext("ARB_fragment_shader");
    has_s3tc         = check_ext("EXT_texture_compression_s3tc");
    has_fbo          = check_ext("EXT_framebuffer_object");
    has_vbo          = check_ext("ARB_vertex_buffer_object");
    has_dre          = check_ext("EXT_draw_range_elements");

    // GL_ARB_multitexture

    if (has_multitexture) try
    {
        PROC(PFNGLACTIVETEXTUREARBPROC,          glActiveTextureARB);
    }
    catch (std::runtime_error& e) { has_multitexture = false; }

    // GL_ARB_shader_objects

    if (has_shader) try
    {
        PROC(PFNGLGETOBJECTPARAMETERIVARBPROC,   glGetObjectParameterivARB);
        PROC(PFNGLCREATEPROGRAMOBJECTARBPROC,    glCreateProgramObjectARB);
        PROC(PFNGLCREATESHADEROBJECTARBPROC,     glCreateShaderObjectARB);
        PROC(PFNGLUSEPROGRAMOBJECTARBPROC,       glUseProgramObjectARB);
        PROC(PFNGLVALIDATEPROGRAMARBPROC,        glValidateProgramARB);
        PROC(PFNGLSHADERSOURCEARBPROC,           glShaderSourceARB);
        PROC(PFNGLCOMPILESHADERARBPROC,          glCompileShaderARB);
        PROC(PFNGLATTACHOBJECTARBPROC,           glAttachObjectARB);
        PROC(PFNGLLINKPROGRAMARBPROC,            glLinkProgramARB);
        PROC(PFNGLGETINFOLOGARBPROC,             glGetInfoLogARB);
        PROC(PFNGLDELETEOBJECTARBPROC,           glDeleteObjectARB);

        PROC(PFNGLGETUNIFORMLOCATIONARBPROC,     glGetUniformLocationARB);
        PROC(PFNGLUNIFORM1IARBPROC,              glUniform1iARB);
        PROC(PFNGLUNIFORM1FARBPROC,              glUniform1fARB);
        PROC(PFNGLUNIFORM2FARBPROC,              glUniform2fARB);
        PROC(PFNGLUNIFORM3FARBPROC,              glUniform3fARB);
        PROC(PFNGLUNIFORM4FARBPROC,              glUniform4fARB);
        PROC(PFNGLUNIFORMMATRIX4FVARBPROC,       glUniformMatrix4fvARB);
    }
    catch (std::runtime_error& e) { has_shader = false; }
        
    // GL_ARB_vertex_shader

    if (has_shader) try
    {
        PROC(PFNGLDISABLEVERTEXATTRIBARRAYARBPROC,glDisableVertexAttribArrayARB);
        PROC(PFNGLENABLEVERTEXATTRIBARRAYARBPROC,glEnableVertexAttribArrayARB);
        PROC(PFNGLVERTEXATTRIBPOINTERARBPROC,    glVertexAttribPointerARB);
        PROC(PFNGLBINDATTRIBLOCATIONARBPROC,     glBindAttribLocationARB);
    }
    catch (std::runtime_error& e) { has_shader = false; }

    // GL_EXT_framebuffer_object

    if (has_fbo) try
    {
        PROC(PFNGLBINDRENDERBUFFEREXTPROC,       glBindRenderbufferEXT);
        PROC(PFNGLGENRENDERBUFFERSEXTPROC,       glGenRenderbuffersEXT);
        PROC(PFNGLRENDERBUFFERSTORAGEEXTPROC,    glRenderbufferStorageEXT);
        PROC(PFNGLDELETERENDERBUFFERSEXTPROC,    glDeleteRenderbuffersEXT);

        PROC(PFNGLGENFRAMEBUFFERSEXTPROC,        glGenFramebuffersEXT);
        PROC(PFNGLBINDFRAMEBUFFEREXTPROC,        glBindFramebufferEXT);
        PROC(PFNGLDELETEFRAMEBUFFERSEXTPROC,     glDeleteFramebuffersEXT);
        PROC(PFNGLFRAMEBUFFERTEXTURE2DEXTPROC,   glFramebufferTexture2DEXT);
        PROC(PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC,glFramebufferRenderbufferEXT);
        PROC(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC, glCheckFramebufferStatusEXT);
    }
    catch (std::runtime_error& e) { has_fbo = false; }

    // GL_ARB_vertex_buffer_object

    if (has_vbo) try
    {
        PROC(PFNGLGENBUFFERSARBPROC,             glGenBuffersARB);
        PROC(PFNGLBINDBUFFERARBPROC,             glBindBufferARB);
        PROC(PFNGLMAPBUFFERARBPROC,              glMapBufferARB);
        PROC(PFNGLBUFFERDATAARBPROC,             glBufferDataARB);
        PROC(PFNGLUNMAPBUFFERARBPROC,            glUnmapBufferARB);
        PROC(PFNGLBUFFERSUBDATAARBPROC,          glBufferSubDataARB);
        PROC(PFNGLDELETEBUFFERSARBPROC,          glDeleteBuffersARB);
    }
    catch (std::runtime_error& e) { has_vbo = false; }

    if (has_dre) try
    {
        PROC(PFNGLDRAWRANGEELEMENTSEXTPROC,       glDrawRangeElementsEXT);
    }
    catch (std::runtime_error&e) { has_dre = false; }

    // GL_EXT_texture_compression_s3tc

    if (has_s3tc && ::conf->get_i("texture_compression"))
        do_texture_compression = true;
    else
        do_texture_compression = false;

    // Configuration options

    std::string option;

    if (has_fbo && has_shader)
    {
        // Shadowing

        option = ::conf->get_s("shadow_method");

        if (option == "map")  do_shadows = 1;
        else                  do_shadows = 0;

        do_reflect = (::conf->get_s("reflect") == "false") ? false : true;
        do_refract = (::conf->get_s("refract") == "false") ? false : true;

        // HDR

        do_hdr_tonemap = ::conf->get_i("hdr_tonemap");
        do_hdr_bloom   = ::conf->get_i("hdr_bloom");
    }
    else
    {
        do_shadows     = 0;
        do_reflect     = false;
        do_refract     = false;
        do_hdr_tonemap = false;
        do_hdr_bloom   = false;
    }

    if (::conf->get_s("z_only") == "false")
        do_z_only = false;
    else
        do_z_only = true;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    sync(::conf->get_i("sync"));
}

//-----------------------------------------------------------------------------

void ogl::line_state_init()
{
    // Set up for Z-offset anti-aliased line drawing.

    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_POLYGON_OFFSET_LINE);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glPolygonOffset(-1.1f, -4.0f);
}

void ogl::line_state_fini()
{
    glDepthMask(GL_TRUE);

    glDisable(GL_POLYGON_OFFSET_LINE);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
}

//-----------------------------------------------------------------------------
