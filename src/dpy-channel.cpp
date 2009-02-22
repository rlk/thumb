//  Copyright (C) 2007 Robert Kooima
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

#include <cassert>

#include "matrix.hpp"
#include "default.hpp"
#include "app-glob.hpp"
#include "app-conf.hpp"
#include "app-event.hpp"
#include "ogl-frame.hpp"
#include "ogl-program.hpp"
#include "dpy-channel.hpp"

#define HALF(i) ((i + 1) / 2)

//-----------------------------------------------------------------------------

const ogl::program *dpy::channel::downsample = 0;
const ogl::program *dpy::channel::h_gaussian = 0;
const ogl::program *dpy::channel::v_gaussian = 0;
const ogl::program *dpy::channel::tonemap    = 0;

ogl::frame *dpy::channel::blur = 0;
ogl::frame *dpy::channel::ping = 0;
ogl::frame *dpy::channel::pong = 0;

//-----------------------------------------------------------------------------

dpy::channel::channel(app::node node) : src(0), dst(0), processed(false)
{
    v[0] = p[0] = app::get_attr_f(node, "x");
    v[1] = p[1] = app::get_attr_f(node, "y");
    v[2] = p[2] = app::get_attr_f(node, "z");

    c[0] = GLubyte(app::get_attr_f(node, "r", 1.0) * 0xFF);
    c[1] = GLubyte(app::get_attr_f(node, "g", 1.0) * 0xFF);
    c[2] = GLubyte(app::get_attr_f(node, "b", 1.0) * 0xFF);
    c[3] = GLubyte(app::get_attr_f(node, "a", 1.0) * 0xFF);

    w = app::get_attr_d(node, "w", DEFAULT_PIXEL_WIDTH);
    h = app::get_attr_d(node, "h", DEFAULT_PIXEL_HEIGHT);
}

dpy::channel::~channel()
{
}

void dpy::channel::set_head(const double *p,
                            const double *q)
{
    // Cache the view position in the head's coordinate system.

    double M[16], w[3];

    set_quaternion(M, q);

    mult_mat_vec3(w, M, v);

    this->p[0] = p[0] + w[0];
    this->p[1] = p[1] + w[1];
    this->p[2] = p[2] + w[2];
}

//-----------------------------------------------------------------------------

void dpy::channel::test() const
{
    // Clear the bound buffer to the calibration color.

    glClearColor(c[0], c[1], c[2], c[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}

void dpy::channel::bind() const
{
    // Bind the off-screen render target.

    assert(src);
    src->bind();
}

void dpy::channel::free() const
{
    // Unbind the off-screen render target.

    assert(src);
    src->free();
}

void dpy::channel::bind_color(GLenum t) const
{
    // Bind the off-screen render target for reading.

    if (processed)
    {
        assert(dst);
        dst->bind_color(t);
    }
    else
    {
        assert(src);
        src->bind_color(t);
    }
}

void dpy::channel::free_color(GLenum t) const
{
    // Unbind the off-screen render target for reading.

    if (processed)
    {
        assert(dst);
        dst->free_color(t);
    }
    else
    {
        assert(src);
        src->free_color(t);
    }
}

//-----------------------------------------------------------------------------

static void apply(ogl::frame *src,
                  ogl::frame *dst,
                  int sw, int sh,
                  int dw, int dh)
{
    double x0 =  -1.0;
    double y0 =  -1.0;
    double x1 =  -1.0 + 2.0 * double(dw) / double(dst->get_w());
    double y1 =  -1.0 + 2.0 * double(dh) / double(dst->get_h());

    ogl::program::current->uniform("size", sw, sh);

    src->bind_color();
    {
        dst->bind();
        {
            glRectd(x0, y0, x1, y1);
        }
        dst->free();
    }
    src->free_color();
}

void dpy::channel::proc()
{
    if (ogl::do_hdr_tonemap)
    {
        assert(downsample);
        assert(h_gaussian);
        assert(v_gaussian);
        assert(tonemap);
        assert(src);
        assert(blur);
        assert(ping);
        assert(pong);

        // Downsample the scene.

        downsample->bind();
        {
            ogl::frame *temp;

            int W = w;
            int H = h;

            // Downsample the source to the blur buffer.

            if (W > 1 || W > 1)
            {
                apply(src, blur, W, H, HALF(W), HALF(H));
                W = HALF(W);
                H = HALF(H);
            }

            // Downsample the blur to the ping buffer.

            if (W > 1 || H > 1)
            {
                apply(blur, ping, W, H, HALF(W), HALF(H));
                W = HALF(W);
                H = HALF(H);
            }

            // Subsequent downsamplings work from ping to pong, and swap them.

            while (W > 1 || H > 1)
            {
                apply(ping, pong, W, H, HALF(W), HALF(H));
                W = HALF(W);
                H = HALF(H);

                temp = ping;
                ping = pong;
                pong = temp;
            }

            // The average color is now in pong.
        }
        downsample->free();

        if (ogl::do_hdr_bloom)
        {
            // Apply the horizontal Gaussion from blur to pong.

            h_gaussian->bind();
            {
                apply(blur, pong, HALF(w), HALF(h), HALF(w), HALF(h));
            }   
            h_gaussian->bind();

            // Apply the vertical Gaussion from pong back to blur.

            v_gaussian->bind();
            {
                apply(pong, blur, HALF(w), HALF(h), HALF(w), HALF(h));
            }   
            v_gaussian->bind();
        }

        // Apply the tonemapping to the original image using the downsampled
        // average scene color and the blurred bloom buffer.

        tonemap->bind();
        {
            if (ogl::do_hdr_bloom)
            {
                ping->bind_color(GL_TEXTURE1);
                blur->bind_color(GL_TEXTURE2);
                {
                    apply(src, dst, w, h, w, h);
                }
                blur->free_color(GL_TEXTURE2);
                ping->free_color(GL_TEXTURE1);
            }
            else
            {
                ping->bind_color(GL_TEXTURE1);
                {
                    apply(src, dst, w, h, w, h);
                }
                ping->free_color(GL_TEXTURE1);
            }
        }
        tonemap->free();

        processed = true;
    }
}

//-----------------------------------------------------------------------------

void dpy::channel::process_start()
{
    assert(src == 0);

    if (ogl::do_hdr_tonemap)
    {
        // Initialize the off-screen render target.

        src = ::glob->new_frame(w, h, GL_TEXTURE_RECTANGLE_ARB,
                                      GL_RGBA16F_ARB, true,  false);

        dst = ::glob->new_frame(w, h, GL_TEXTURE_RECTANGLE_ARB,
                                      GL_RGBA8,       false, false);

        // Initialize the static ping-pong buffers, if necessary.
        
        if (blur == 0)
            blur = ::glob->new_frame(HALF(w), HALF(h),
                                     GL_TEXTURE_RECTANGLE_ARB,
                                     GL_RGBA16F_ARB, false, false);
        if (ping == 0)
            ping = ::glob->new_frame(HALF(w), HALF(h),
                                     GL_TEXTURE_RECTANGLE_ARB,
                                     GL_RGBA16F_ARB, false, false);
        if (pong == 0)
            pong = ::glob->new_frame(HALF(w), HALF(h),
                                     GL_TEXTURE_RECTANGLE_ARB,
                                     GL_RGBA16F_ARB, false, false);

        // Initialize the static programs, if necessary.

        if (downsample == 0)
            downsample = ::glob->load_program("hdr/downsample.xml");
        if (h_gaussian == 0)
            h_gaussian = ::glob->load_program("hdr/h_gaussian.xml");
        if (v_gaussian == 0)
            v_gaussian = ::glob->load_program("hdr/v_gaussian.xml");
        if (tonemap == 0)
            tonemap    = ::glob->load_program("hdr/tonemap.xml");
    }
    else
    {
        // Initialize the off-screen render target.

        src = ::glob->new_frame(w, h, GL_TEXTURE_RECTANGLE_ARB,
                                      GL_RGB8, true, false);
    }
}

void dpy::channel::process_close()
{
    assert(src != 0);

    // Finalize the static programs, if necessary.

    if (tonemap)    ::glob->free_program(tonemap);
    if (v_gaussian) ::glob->free_program(v_gaussian);
    if (h_gaussian) ::glob->free_program(h_gaussian);
    if (downsample) ::glob->free_program(downsample);

    tonemap    = 0;
    v_gaussian = 0;
    h_gaussian = 0;
    downsample = 0;

    // Finalize the static ping-pong buffers, if necessary.

    if (pong) ::glob->free_frame(pong);
    if (ping) ::glob->free_frame(ping);
    if (blur) ::glob->free_frame(blur);

    pong = 0;
    ping = 0;
    blur = 0;

    // Finalize the off-screen render targets.

    ::glob->free_frame(dst);
    ::glob->free_frame(src);

    dst = 0;
    src = 0;
}

bool dpy::channel::process_event(app::event *E)
{
    switch (E->get_type())
    {
    case E_START: process_start(); break;
    case E_CLOSE: process_close(); break;
    }
    return false;
}

//-----------------------------------------------------------------------------
