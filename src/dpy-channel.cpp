//  Copyright (C) 2007-2011 Robert Kooima
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

#include <etc-vector.hpp>
#include <app-default.hpp>
#include <app-glob.hpp>
#include <app-conf.hpp>
#include <app-event.hpp>
#include <ogl-frame.hpp>
#include <ogl-program.hpp>
#include <dpy-channel.hpp>

#define HALF(i) ((i + 1) / 2)

//-----------------------------------------------------------------------------

const ogl::program *dpy::channel::downsample_avg = 0;
const ogl::program *dpy::channel::downsample_max = 0;
const ogl::program *dpy::channel::h_gaussian     = 0;
const ogl::program *dpy::channel::v_gaussian     = 0;
const ogl::program *dpy::channel::tonemap        = 0;
const ogl::program *dpy::channel::bloom          = 0;

ogl::frame *dpy::channel::blur = 0;
ogl::frame *dpy::channel::ping = 0;
ogl::frame *dpy::channel::pong = 0;

//-----------------------------------------------------------------------------

dpy::channel::channel(app::node n) : src(0), dst(0)
{
    const std::string unit = n.get_s("unit");

    double scale = scale_to_meters(unit.empty() ? "ft" : unit);

    // Extract the configuration.

    v[0] = p[0] = n.get_f("x") * scale;
    v[1] = p[1] = n.get_f("y") * scale;
    v[2] = p[2] = n.get_f("z") * scale;

    c[0] = GLubyte(n.get_f("r", 1.0) * 0xFF);
    c[1] = GLubyte(n.get_f("g", 1.0) * 0xFF);
    c[2] = GLubyte(n.get_f("b", 1.0) * 0xFF);
    c[3] = GLubyte(n.get_f("a", 1.0) * 0xFF);

    w = n.get_i("w", DEFAULT_PIXEL_WIDTH);
    h = n.get_i("h", DEFAULT_PIXEL_HEIGHT);

    // Optionally compute X from the inter-pupilary distance.

    int    ii = n.get_i("i");
    int    in = n.get_i("n");
    double id = n.get_f("d");

    if (in) v[0] = p[0] = 0.5 * id * (ii - 0.5 * (in - 1.0));
}

dpy::channel::~channel()
{
}

void dpy::channel::set_head(const vec3& p, const quat& q)
{
    // Cache the view position in the head's coordinate system.

    this->p = p + mat3(q) * v;
}

//-----------------------------------------------------------------------------

void dpy::channel::test() const
{
    // Clear the bound buffer to the calibration color.

    glClearColor(c[0], c[1], c[2], c[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}

void dpy::channel::bind(double q) const
{
    // Bind the off-screen render target.

    assert(src);
    src->bind(q);
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

    if (ogl::do_hdr_tonemap || ogl::do_hdr_bloom)
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

    if (ogl::do_hdr_tonemap || ogl::do_hdr_bloom)
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

void dpy::channel::proc() const
{
    // Bloom the frame buffer.

    if (ogl::do_hdr_bloom)
    {
        int w2 = HALF(w), w4 = HALF(w2);
        int h2 = HALF(h), h4 = HALF(h2);

        // Max-downsample the image to produce the un-blurred bloom.

        downsample_max->bind();
        {
            apply(src,  ping, w,  h,  w2, h2);
            apply(ping, blur, w2, h2, w4, w4);
        }
        downsample_max->free();

        // Apply the horizontal Gaussion from blur to ping.

        h_gaussian->bind();
        {
            apply(blur, ping, w4, h4, w4, h4);
        }
        h_gaussian->free();

        // Apply the vertical Gaussion from ping back to blur.

        v_gaussian->bind();
        {
            apply(ping, blur, w4, h4, w4, h4);
        }
        v_gaussian->free();
    }

    // Compute the average color using downsampling.

    if (ogl::do_hdr_tonemap)
    {
        downsample_avg->bind();
        {
            int W = w;
            int H = h;

            // Downsample the source framebuffer to the ping buffer.

            if (W > 1 || W > 1)
            {
                apply(src, ping, W, H, HALF(W), HALF(H));
                W = HALF(W);
                H = HALF(H);
            }

            // Subsequent downsamplings work from ping to pong, and swap.

            while (W > 1 || H > 1)
            {
                apply(ping, pong, W, H, HALF(W), HALF(H));
                W = HALF(W);
                H = HALF(H);

                ogl::frame *temp;

                temp = ping;
                ping = pong;
                pong = temp;
            }

            // The average framebuffer color is now in pixel (0,0) of ping.
        }
        downsample_avg->free();
    }

    if (ogl::do_hdr_tonemap)
    {
        // Tone-map the original image using the downsampled average
        // scene color and (optionally) the blurred bloom buffer.

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
    }
    else if (ogl::do_hdr_bloom)
    {
        // Sum the original image with the blurred bloom buffer.

        bloom->bind();
        {
            blur->bind_color(GL_TEXTURE1);
            {
                apply(src, dst, w, h, w, h);
            }
            blur->free_color(GL_TEXTURE1);
        }
        bloom->free();
    }
}

//-----------------------------------------------------------------------------

void dpy::channel::process_start()
{
    assert(src == 0);

    if (ogl::do_hdr_tonemap || ogl::do_hdr_bloom)
    {
        int w2 = HALF(w), w4 = HALF(w2);
        int h2 = HALF(h), h4 = HALF(h2);

        // Initialize the off-screen render target.

        src = ::glob->new_frame(w, h, GL_TEXTURE_RECTANGLE,
                                GL_RGBA16F, true, true,  false);

        dst = ::glob->new_frame(w, h, GL_TEXTURE_RECTANGLE,
                                GL_RGBA8,   true, false, false);

        // Initialize the static ping-pong buffers, if necessary.

        if (blur == 0)
            blur = ::glob->new_frame(w4, h4, GL_TEXTURE_RECTANGLE,
                                     GL_RGBA16F, true, false, false);
        if (ping == 0)
            ping = ::glob->new_frame(w2, h2, GL_TEXTURE_RECTANGLE,
                                     GL_RGBA16F, true, false, false);
        if (pong == 0)
            pong = ::glob->new_frame(w2, h2, GL_TEXTURE_RECTANGLE,
                                     GL_RGBA16F, true, false, false);

        // Initialize the static programs, if necessary.

        if (downsample_avg == 0)
            downsample_avg = ::glob->load_program("hdr/downsample-avg.xml");
        if (downsample_max == 0)
            downsample_max = ::glob->load_program("hdr/downsample-avg.xml");
//          downsample_max = ::glob->load_program("hdr/downsample-max.xml");
        if (h_gaussian     == 0)
            h_gaussian     = ::glob->load_program("hdr/h-gaussian.xml");
        if (v_gaussian     == 0)
            v_gaussian     = ::glob->load_program("hdr/v-gaussian.xml");
        if (tonemap        == 0)
            tonemap        = ::glob->load_program("hdr/tonemap.xml");
        if (bloom          == 0)
            bloom          = ::glob->load_program("hdr/bloom.xml");
    }
    else
    {
        // Initialize the off-screen render target.

        src = ::glob->new_frame(w, h, GL_TEXTURE_RECTANGLE,
                                GL_RGB8, true, true, false);
    }
}

void dpy::channel::process_close()
{
    assert(src != 0);

    // Finalize the static programs, if necessary.

    if (bloom)          ::glob->free_program(bloom);
    if (tonemap)        ::glob->free_program(tonemap);
    if (v_gaussian)     ::glob->free_program(v_gaussian);
    if (h_gaussian)     ::glob->free_program(h_gaussian);
    if (downsample_avg) ::glob->free_program(downsample_avg);
    if (downsample_max) ::glob->free_program(downsample_max);

    bloom          = 0;
    tonemap        = 0;
    v_gaussian     = 0;
    h_gaussian     = 0;
    downsample_avg = 0;
    downsample_max = 0;

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
