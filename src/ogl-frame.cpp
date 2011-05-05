//  Copyright (C) 2009-2011 Robert Kooima
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

#include <ogl-frame.hpp>

// CAVEAT: This implementation only allows a stencil buffer to be used in
// the presence of a depth buffer.  The OpenGL implementation must support
// EXT_packed_depth_stencil.

//-----------------------------------------------------------------------------

std::vector<ogl::frame::state> ogl::frame::stack;

void ogl::frame::push(GLuint o, GLint x, GLint y, GLsizei w, GLsizei h)
{
    stack.push_back(state(o, x, y, w, h));
    stack.back().apply();
}

void ogl::frame::pop()
{
    stack.pop_back();
    stack.back().apply();
}

//-----------------------------------------------------------------------------

ogl::frame::frame(GLsizei w, GLsizei h,
                  GLenum  t, GLenum  f, bool c, bool d, bool s) :
    target(t),
    format(f),
    buffer(0),
    color(0),
    depth(0),
    has_color(c),
    has_depth(d),
    has_stencil(s),
    w(w),
    h(h)
{
    // If this is the first frame buffer, initialize the frame buffer stack
    // with the on-screen frame buffer (FBO 0) and the current viewport.

    if (stack.empty())
    {
        GLint v[4];

        glGetIntegerv(GL_VIEWPORT, v);
        stack.push_back(state(0, v[0], v[1], v[2], v[3]));
    }

    init();
}

ogl::frame::~frame()
{
    fini();
}

//-----------------------------------------------------------------------------

void ogl::frame::bind_color(GLenum unit) const
{
    ogl::bind_texture(target, unit, color);
}

void ogl::frame::free_color(GLenum unit) const
{
}

void ogl::frame::bind_depth(GLenum unit) const
{
    ogl::bind_texture(target, unit, depth);
}

void ogl::frame::free_depth(GLenum unit) const
{
}

//-----------------------------------------------------------------------------

void ogl::frame::bind(double q) const
{
    push(buffer, 0, 0, int(w * q), int(h * q));
}

void ogl::frame::bind(bool b) const
{
    push(buffer, 0, 0, w, h);

    // TODO: Eliminate this antiquated code (used by the uni modules).

    if (b)
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, w, 0, h, 0, 1);
	
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
    }
}

void ogl::frame::bind(int target) const
{
    push(buffer, 0, 0, w, h);

    // TODO: there's probably a smarter way to handle cube face switching.

    if (target)
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                                  GL_COLOR_ATTACHMENT0,
                                  target, color, 0);
}

void ogl::frame::bind() const
{
    push(buffer, 0, 0, w, h);
}

void ogl::frame::free() const
{
    pop();
}

void ogl::frame::free(bool b) const
{
    if (b)
    {
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }

    pop();
}

//-----------------------------------------------------------------------------

void ogl::frame::init_cube()
{
     // Initialize the cube map color render buffer object.

    ogl::bind_texture(target, 0, color);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format,
                 w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format,
                 w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format,
                 w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format,
                 w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format,
                 w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format,
                 w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_R,     GL_CLAMP_TO_EDGE);
}

void ogl::frame::init_color()
{
     // Initialize the color render buffer object.

    ogl::bind_texture(target, 0, color);

    glTexImage2D(target, 0, format, w, h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
}

void ogl::frame::init_depth()
{
    // Initialize the depth and stencil render buffer objects.

    ogl::bind_texture(target, 0, depth);

#ifdef GL_DEPTH_STENCIL
    if (has_stencil && ogl::has_depth_stencil)
        glTexImage2D(target, 0, GL_DEPTH24_STENCIL8, w, h, 0,
                     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    else
#endif
        glTexImage2D(target, 0, GL_DEPTH_COMPONENT24,    w, h, 0,
                     GL_DEPTH_COMPONENT,   GL_UNSIGNED_BYTE,         NULL);

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    glTexParameteri(target, GL_TEXTURE_COMPARE_MODE,
                            GL_COMPARE_R_TO_TEXTURE);
}

void ogl::frame::init_frame()
{
    // Initialize the frame buffer object.

    if (has_stencil)
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                                  GL_STENCIL_ATTACHMENT,
                                  target, depth, 0);
    if (has_depth)
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                                  GL_DEPTH_ATTACHMENT,
                                  target, depth, 0);
    if (has_color)
    {
        if (target == GL_TEXTURE_CUBE_MAP)
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                      GL_COLOR_ATTACHMENT0,
                                      GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                                      color, 0);
        else
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                      GL_COLOR_ATTACHMENT0,
                                      target, color, 0);
    }
    else
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    // Confirm the frame buffer object status.

    switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
    {
    case GL_FRAMEBUFFER_COMPLETE:
        break; 
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        throw std::runtime_error("Framebuffer incomplete attachment");
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        throw std::runtime_error("Framebuffer missing attachment");
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        throw std::runtime_error("Framebuffer draw buffer");
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        throw std::runtime_error("Framebuffer read buffer");
    case GL_FRAMEBUFFER_UNSUPPORTED:
        throw std::runtime_error("Framebuffer unsupported");
    default:
        throw std::runtime_error("Framebuffer error");
    }
}

void ogl::frame::init()
{
    if (has_color)
    {
        glGenTextures(1, &color);

        if (target == GL_TEXTURE_CUBE_MAP)
            init_cube();
        else
            init_color();
    }
    if (has_depth)
    {
        glGenTextures(1, &depth);
        init_depth();
    }

    glGenFramebuffers(1, &buffer);

    push(buffer, 0, 0, w, h);
    {
        init_frame();
    }
    pop();
}

void ogl::frame::fini()
{
    if (buffer) glDeleteFramebuffers(1, &buffer);

    if (color) glDeleteTextures(1, &color);
    if (depth) glDeleteTextures(1, &depth);
}

//-----------------------------------------------------------------------------

