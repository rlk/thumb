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
bool ogl::has_fbo;
bool ogl::has_vbo;
bool ogl::has_dre;

int  ogl::do_shadows;
bool ogl::do_z_only;
bool ogl::do_reflect;
bool ogl::do_refract;

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

    return true;

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
        assert(0);

        throw std::runtime_error(str.str().c_str());
    }
}

//-----------------------------------------------------------------------------

#ifdef _WIN32

#include <GL/wglext.h>

static void sync(int interval)
{
    PFNWGLSWAPINTERVALEXTPROC _wglSwapInvervalEXT;

    if ((_wglSwapInvervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
          wglGetProcAddress("wglSwapIntervalEXT")))
         _wglSwapInvervalEXT(interval);
}

#else

static void sync(int interval)
{
    PFNGLXSWAPINTERVALSGIPROC _glXSwapInvervalSGI;

    if ((_glXSwapInvervalSGI = (PFNGLXSWAPINTERVALSGIPROC)
          glXGetProcAddress((const GLubyte *) "glXSwapIntervalSGI")))
         _glXSwapInvervalSGI(interval);
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

    // Configuration options

    std::string option;

    if (has_fbo && has_shader)
    {
        option = ::conf->get_s("shadows");

        if (option == "csm")  do_shadows = 2;
        if (option == "map")  do_shadows = 1;
        else                  do_shadows = 0;

        do_reflect = (::conf->get_s("reflect") == "false") ? false : true;
        do_refract = (::conf->get_s("refract") == "false") ? false : true;
    }
    else
    {
        do_shadows = 0;
        do_reflect = false;
        do_refract = false;
    }

    if (::conf->get_s("z_only") == "false")
        do_z_only = false;
    else
        do_z_only = true;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    sync(::conf->get_i("sync"));
}

//-----------------------------------------------------------------------------

void ogl::draw_axes()
{
    glBegin(GL_LINES);
    {
        float k = 16.0f;

        glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
        glVertex3f(-k, 0.0f, 0.0f);
        glVertex3f(+k, 0.0f, 0.0f);

        glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
        glVertex3f(0.0f, -k, 0.0f);
        glVertex3f(0.0f, +k, 0.0f);

        glColor4f(0.0f, 0.0f, 1.0f, 0.5f);
        glVertex3f(0.0f, 0.0f, -k);
        glVertex3f(0.0f, 0.0f, +k);
    }
    glEnd();
}

//-----------------------------------------------------------------------------
/*
static int texture_total = 0;

static int texture_size(GLenum target)
{
    int   T  = 0;
    GLint l0 = 0;
    GLint l1 = 0;

//  glGetTexParameteriv(target, GL_TEXTURE_BASE_LEVEL, &l0);
//  glGetTexParameteriv(target, GL_TEXTURE_MAX_LEVEL,  &l1);

    for (GLint level = l0; level <= l1; ++level)
    {
        GLint w, h, d, r, g, b, a, l, i, z;
        
        glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH,          &w);
        glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT,         &h);
        glGetTexLevelParameteriv(target, level, GL_TEXTURE_DEPTH,          &d);
        glGetTexLevelParameteriv(target, level, GL_TEXTURE_RED_SIZE,       &r);
        glGetTexLevelParameteriv(target, level, GL_TEXTURE_GREEN_SIZE,     &g);
        glGetTexLevelParameteriv(target, level, GL_TEXTURE_BLUE_SIZE,      &b);
        glGetTexLevelParameteriv(target, level, GL_TEXTURE_ALPHA_SIZE,     &a);
        glGetTexLevelParameteriv(target, level, GL_TEXTURE_LUMINANCE_SIZE, &l);
        glGetTexLevelParameteriv(target, level, GL_TEXTURE_INTENSITY_SIZE, &i);
        glGetTexLevelParameteriv(target, level, GL_TEXTURE_DEPTH_SIZE,     &z);

        printf("w%04d h%04d d%04d r%02d g%02d b%02d a%02d l%02d i%02d z%02d", 
               w, h, d, r, g, b, a, l, i, z);

        T += w * h * d * (r + g + b + a + l + i + z) / 8;
    }
    return T;
}
*/
void ogl::new_texture(GLenum target)
{
/*
    printf("new ");

    texture_total += texture_size(target);

    printf(" Total: %d (%4.2fMB)\n", texture_total,
           double(texture_total) / (1024 * 1024));
*/
}

void ogl::del_texture(GLenum target)
{
/*
    printf("del ");

    texture_total -= texture_size(target);

    printf(" Total: %d (%4.2fMB)\n", texture_total,
           double(texture_total) / (1024 * 1024));
*/
}

//-----------------------------------------------------------------------------
