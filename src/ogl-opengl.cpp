//  Copyright (C) 2005-2011 Robert Kooima
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
#include <cassert>
#include <sstream>

#include <ogl-opengl.hpp>
#include <app-conf.hpp>

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
// WGL swap interval

#ifdef _WIN32

#include <GL/wglew.h>

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

#include <GL/glxew.h>

static void sync(int interval)
{
    glXSwapIntervalSGI(interval);
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

static void init_opt()
{
    std::string option;

    ogl::max_anisotropy         = 0;
    ogl::do_shadow              = 0;
    ogl::do_z_only              = false;
    ogl::do_texture_compression = false;
    ogl::do_hdr_tonemap         = false;
    ogl::do_hdr_bloom           = false;

    // Query GL capabilities.

    ogl::has_depth_stencil = (glewIsSupported("GL_EXT_packed_depth_stencil")       == GL_TRUE);
    ogl::has_multitexture  = (glewIsSupported("GL_multitexture")                   == GL_TRUE);
    ogl::has_multisample   = (glewIsSupported("GL_multisample")                    == GL_TRUE);
    ogl::has_anisotropic   = (glewIsSupported("GL_EXT_texture_filter_anisotropic") == GL_TRUE);
    ogl::has_glsl          = (glewIsSupported("GL_shader_objects")                 == GL_TRUE);
    ogl::has_glsl         &= (glewIsSupported("GL_vertex_shader")                  == GL_TRUE);
    ogl::has_glsl         &= (glewIsSupported("GL_fragment_shader")                == GL_TRUE);
    ogl::has_s3tc          = (glewIsSupported("GL_EXT_texture_compression_s3tc")   == GL_TRUE);
    ogl::has_fbo           = (glewIsSupported("GL_EXT_framebuffer_object")         == GL_TRUE);
    ogl::has_vbo           = (glewIsSupported("GL_vertex_buffer_object")           == GL_TRUE);
    ogl::has_dre           = (glewIsSupported("GL_EXT_draw_range_elements")        == GL_TRUE);

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
        // Shadowing

        option = ::conf->get_s("shadow_method");

        if (option == "map")  ogl::do_shadow = 1;

        // HDR

        ogl::do_hdr_tonemap = (::conf->get_i("hdr_tonemap") != 0);
        ogl::do_hdr_bloom   = (::conf->get_i("hdr_bloom")   != 0);

        // Z-only pass

        ogl::do_z_only = (::conf->get_i("z_only") != 0);
    }

    // Set vertical blanking synchronization state.

    sync(::conf->get_i("sync"));
}

static void init_state(bool multisample)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (ogl::has_multisample && multisample)
        glEnable(GL_MULTISAMPLE);
}

void ogl::init(bool multisample)
{
    init_opt();
    init_state(multisample);
}

//-----------------------------------------------------------------------------

#define MAX_TEXTURE_UNITS 16

static GLenum current_object[MAX_TEXTURE_UNITS];
static GLenum current_unit = GL_TEXTURE0;

void ogl::curr_texture(GLenum unit)
{
    if (unit)
    {
        if (current_unit != unit)
        {
            current_unit  = unit;
            glActiveTexture(unit);
        }
    }
}

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
        curr_texture(unit);

        glBindTexture(target, object);
    }
*/

    if (unit == GL_TEXTURE0) // HACK: the above optimization breaks user code.
        glBindTexture(target, object);
    else
    {
        glActiveTexture(unit);
        glBindTexture(target, object);
        glActiveTexture(GL_TEXTURE0);
    }
}

void ogl::xfrm_texture(GLenum unit, const GLdouble *M)
{
    // Load matrix M as texture matrix UNIT.

    curr_texture(unit);
    glMatrixMode(GL_TEXTURE);
    {
        glLoadMatrixd(M);
    }
    glMatrixMode(GL_MODELVIEW);
}

void ogl::free_texture()
{
    for (int u = MAX_TEXTURE_UNITS - 1; u >= 0; --u)
    {
        glActiveTexture(GL_TEXTURE0 + u);

        glBindTexture(GL_TEXTURE_1D,        0);
        glBindTexture(GL_TEXTURE_2D,        0);
        glBindTexture(GL_TEXTURE_3D,        0);
        glBindTexture(GL_TEXTURE_CUBE_MAP,  0);
        glBindTexture(GL_TEXTURE_RECTANGLE, 0);

        current_object[u] = 0;
    }
}

//-----------------------------------------------------------------------------

void ogl::line_state_init()
{
    // Set up for Z-offset anti-aliased line drawing.

    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_POLYGON_OFFSET_LINE);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glPolygonOffset(-1.1f, -4.0f);
}

void ogl::line_state_fini()
{
    glDepthMask(GL_TRUE);

    glDisable(GL_POLYGON_OFFSET_LINE);
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
}

//-----------------------------------------------------------------------------
