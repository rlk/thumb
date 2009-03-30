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

bool ogl::has_depth_stencil;
bool ogl::has_multitexture;
bool ogl::has_multisample;
bool ogl::has_anisotropic;
bool ogl::has_glsl;
bool ogl::has_s3tc;
bool ogl::has_fbo;
bool ogl::has_vbo;
bool ogl::has_dre;

int  ogl::max_anisotropy;

int  ogl::do_shadow;
bool ogl::do_z_only;
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
PFNGLUNIFORM1FVARBPROC               glUniform1fvARB;
PFNGLUNIFORM2FVARBPROC               glUniform2fvARB;
PFNGLUNIFORM3FVARBPROC               glUniform3fvARB;
PFNGLUNIFORM4FVARBPROC               glUniform4fvARB;
PFNGLUNIFORMMATRIX3FVARBPROC         glUniformMatrix3fvARB;
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

static void init_ext()
{
    // Query GL capabilities.

    ogl::has_depth_stencil = ogl::check_ext("EXT_packed_depth_stencil");
    ogl::has_multitexture  = ogl::check_ext("ARB_multitexture");
    ogl::has_multisample   = ogl::check_ext("ARB_multisample");
    ogl::has_anisotropic   = ogl::check_ext("EXT_texture_filter_anisotropic");
    ogl::has_glsl          = ogl::check_ext("ARB_shader_objects");
    ogl::has_glsl         &= ogl::check_ext("ARB_vertex_shader");
    ogl::has_glsl         &= ogl::check_ext("ARB_fragment_shader");
    ogl::has_s3tc          = ogl::check_ext("EXT_texture_compression_s3tc");
    ogl::has_fbo           = ogl::check_ext("EXT_framebuffer_object");
    ogl::has_vbo           = ogl::check_ext("ARB_vertex_buffer_object");
    ogl::has_dre           = ogl::check_ext("EXT_draw_range_elements");

    // GL_ARB_multitexture

    if (ogl::has_multitexture) try
    {
        PROC(PFNGLACTIVETEXTUREARBPROC,          glActiveTextureARB);
    }
    catch (std::runtime_error& e) { ogl::has_multitexture = false; }

    // GL_ARB_shader_objects

    if (ogl::has_glsl) try
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
        PROC(PFNGLUNIFORM1FVARBPROC,             glUniform1fvARB);
        PROC(PFNGLUNIFORM2FVARBPROC,             glUniform2fvARB);
        PROC(PFNGLUNIFORM3FVARBPROC,             glUniform3fvARB);
        PROC(PFNGLUNIFORM4FVARBPROC,             glUniform4fvARB);
        PROC(PFNGLUNIFORMMATRIX3FVARBPROC,       glUniformMatrix3fvARB);
        PROC(PFNGLUNIFORMMATRIX4FVARBPROC,       glUniformMatrix4fvARB);
    }
    catch (std::runtime_error& e) { ogl::has_glsl = false; }
        
    // GL_ARB_vertex_shader

    if (ogl::has_glsl) try
    {
        PROC(PFNGLDISABLEVERTEXATTRIBARRAYARBPROC,glDisableVertexAttribArrayARB);
        PROC(PFNGLENABLEVERTEXATTRIBARRAYARBPROC,glEnableVertexAttribArrayARB);
        PROC(PFNGLVERTEXATTRIBPOINTERARBPROC,    glVertexAttribPointerARB);
        PROC(PFNGLBINDATTRIBLOCATIONARBPROC,     glBindAttribLocationARB);
    }
    catch (std::runtime_error& e) { ogl::has_glsl = false; }

    // GL_EXT_framebuffer_object

    if (ogl::has_fbo) try
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
    catch (std::runtime_error& e) { ogl::has_fbo = false; }

    // GL_ARB_vertex_buffer_object

    if (ogl::has_vbo) try
    {
        PROC(PFNGLGENBUFFERSARBPROC,             glGenBuffersARB);
        PROC(PFNGLBINDBUFFERARBPROC,             glBindBufferARB);
        PROC(PFNGLMAPBUFFERARBPROC,              glMapBufferARB);
        PROC(PFNGLBUFFERDATAARBPROC,             glBufferDataARB);
        PROC(PFNGLUNMAPBUFFERARBPROC,            glUnmapBufferARB);
        PROC(PFNGLBUFFERSUBDATAARBPROC,          glBufferSubDataARB);
        PROC(PFNGLDELETEBUFFERSARBPROC,          glDeleteBuffersARB);
    }
    catch (std::runtime_error& e) { ogl::has_vbo = false; }

    // GL_EXT_draw_range_elements

    if (ogl::has_dre) try
    {
        PROC(PFNGLDRAWRANGEELEMENTSEXTPROC,       glDrawRangeElementsEXT);
    }
    catch (std::runtime_error&e) { ogl::has_dre = false; }
}

static void init_opt()
{
    std::string option;

    ogl::max_anisotropy         = 0;
    ogl::do_shadow              = 0;
    ogl::do_z_only              = false;
    ogl::do_texture_compression = false;
    ogl::do_hdr_tonemap         = false;
    ogl::do_hdr_bloom           = false;

    // Configuration options

    if (ogl::has_s3tc && ::conf->get_i("texture_compression"))
        ogl::do_texture_compression = true;

    if (ogl::has_anisotropic)
    {
        int max;

        glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max);

        ogl::max_anisotropy = ::conf->get_i("max_anisotropy");

        ogl::max_anisotropy = std::min(ogl::max_anisotropy, max);
        ogl::max_anisotropy = std::max(ogl::max_anisotropy, 1);
    }

    if (ogl::has_fbo)
    {
        if (ogl::has_glsl)
        {
            // Shadowing

            option = ::conf->get_s("shadow_method");

            if (option == "map")  ogl::do_shadow = 1;

            // HDR

            ogl::do_hdr_tonemap = ::conf->get_i("hdr_tonemap");
            ogl::do_hdr_bloom   = ::conf->get_i("hdr_bloom");
        }

        // Z-only pass

        ogl::do_z_only = ::conf->get_i("z_only");
    }

    // Set vertical blanking synchronization state.

    sync(::conf->get_i("sync"));
}

static void init_state(bool multisample)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableVertexAttribArrayARB(6);

    if (ogl::has_multisample && multisample)
        glEnable(GL_MULTISAMPLE_ARB);
}

void ogl::init(bool multisample)
{
    init_ext();
    init_opt();
    init_state(multisample);
}

//-----------------------------------------------------------------------------

#define MAX_TEXTURE_UNITS 16

static GLenum current_object[MAX_TEXTURE_UNITS];
static GLenum current_unit = GL_TEXTURE0;

void ogl::bind_texture(GLenum target, GLenum unit, GLuint object)
{
    // Bind a texture OBJECT to TARGET of texture UNIT with as little state
    // change as possible.  If UNIT is zero, then use whatever is current.
/*
    int u = unit ? int(unit         - GL_TEXTURE0)
                 : int(current_unit - GL_TEXTURE0);

    if (current_object[u] != object)
    {
        current_object[u]  = object;

        if (unit)
        {
            if (current_unit != unit)
            {
                current_unit  = unit;
                glActiveTextureARB(unit);
            }
        }

        glBindTexture(target, object);
    }
*/

    if (unit) // HACK: the above optimization breaks font rendering.
    {
        glActiveTextureARB(unit);
        glBindTexture(target, object);
        glActiveTextureARB(GL_TEXTURE0);
    }
    else
        glBindTexture(target, object);

    OGLCK();
}

void ogl::free_texture()
{
    for (int u = MAX_TEXTURE_UNITS - 1; u >= 0; --u)
    {
        glActiveTextureARB(GL_TEXTURE0 + u);

        glBindTexture(GL_TEXTURE_1D,            0);
        glBindTexture(GL_TEXTURE_2D,            0);
        glBindTexture(GL_TEXTURE_3D,            0);
        glBindTexture(GL_TEXTURE_CUBE_MAP,      0);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

        current_object[u] = 0;
    }
}

//-----------------------------------------------------------------------------

void ogl::line_state_init()
{
    // Set up for Z-offset anti-aliased line drawing.

    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_OFFSET_LINE);

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
