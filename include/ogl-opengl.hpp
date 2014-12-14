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

#ifndef OGL_OPENGL_HPP
#define OGL_OPENGL_HPP

//-----------------------------------------------------------------------------

#include <GL/glew.h>

#ifndef GL_TEXTURE_RECTANGLE
#define GL_TEXTURE_RECTANGLE GL_TEXTURE_RECTANGLE_ARB
#endif

namespace ogl
{
    extern bool context;

    extern bool has_depth_stencil;
    extern bool has_multisample;
    extern bool has_anisotropic;
    extern bool has_s3tc;

    extern int  max_lights;
    extern int  max_anisotropy;

    extern bool do_texture_compression;
    extern bool do_hdr_tonemap;
    extern bool do_hdr_bloom;

    void check_err(const char *, int);
    bool check_ext(const char *);

    void init(bool);
    void fini();

    void curr_texture(GLenum);
    void bind_texture(GLenum, GLenum, GLuint);
    void xfrm_texture(GLenum, const GLdouble *);
    void free_texture();

    void line_state_init();
    void line_state_fini();
}

//-----------------------------------------------------------------------------

#endif
