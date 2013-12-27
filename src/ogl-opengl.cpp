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
#include <cstdio>
#include <sstream>

#include <ogl-opengl.hpp>
#include <app-conf.hpp>

//-----------------------------------------------------------------------------

bool ogl::has_depth_stencil;
bool ogl::has_multisample;
bool ogl::has_anisotropic;
bool ogl::has_s3tc;

int  ogl::max_lights;
int  ogl::max_anisotropy;

bool ogl::do_texture_compression;
bool ogl::do_hdr_tonemap;
bool ogl::do_hdr_bloom;

//-----------------------------------------------------------------------------

static void init_opt()
{
    std::string option;

    printf("OpenGL %s GLSL %s\n", glGetString(GL_VERSION),
                                  glGetString(GL_SHADING_LANGUAGE_VERSION));

    ogl::max_anisotropy         = 0;
    ogl::do_texture_compression = false;
    ogl::do_hdr_tonemap         = false;
    ogl::do_hdr_bloom           = false;

    // Query GL capabilities.

    ogl::has_depth_stencil = glewIsSupported("GL_EXT_packed_depth_stencil");
    ogl::has_multisample   = glewIsSupported("GL_multisample");
    ogl::has_anisotropic   = glewIsSupported("GL_EXT_texture_filter_anisotropic");
    ogl::has_s3tc          = glewIsSupported("GL_EXT_texture_compression_s3tc");

    // The light count is constrained by both uniform and varying limits.

    GLint maxl;
    GLint maxc;

    glGetIntegerv(GL_MAX_LIGHTS,             &maxl);
    glGetIntegerv(GL_MAX_VARYING_COMPONENTS, &maxc);

    ogl::max_lights = std::min(maxl, (maxc / 4 - 1) / 3);

    // Configuration options

    if (ogl::has_s3tc && ::conf->get_i("texture_compression"))
        ogl::do_texture_compression = true;

    if (ogl::has_anisotropic)
    {
        int maxa;

        glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxa);

        ogl::max_anisotropy = ::conf->get_i("max_anisotropy");
        ogl::max_anisotropy = std::min(ogl::max_anisotropy, maxa);
        ogl::max_anisotropy = std::max(ogl::max_anisotropy, 1);
    }

    // Off-screen rendering

    ogl::do_hdr_tonemap = (::conf->get_i("hdr_tonemap") != 0);
    ogl::do_hdr_bloom   = (::conf->get_i("hdr_bloom")   != 0);
}

static void init_state(bool multisample)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (ogl::has_multisample && multisample)
        glEnable(GL_MULTISAMPLE);

    glAlphaFunc(GL_GREATER, 0.5f);
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
