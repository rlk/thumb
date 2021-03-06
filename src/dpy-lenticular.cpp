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

#include <SDL.h>
#include <SDL_keyboard.h>

#include <etc-vector.hpp>
#include <app-glob.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <ogl-program.hpp>
#include <dpy-channel.hpp>
#include <dpy-lenticular.hpp>

//-----------------------------------------------------------------------------

dpy::lenticular::lenticular(app::node p) :
    display(p),

    channels(p.get_i("channels", 1)),

    pitch(100.0),
    angle(  0.0),
    thick(  0.1),
    shift(  0.0),
    debug(  1.0),
    quality(1.0),

    program(0)
{
    int i;

    // Find a frustum definition, or create a default.  Clone the frustum and
    // create a default slice for each channel.

    for (i = 0; i < channels; ++i)
        if (app::node n = p.find("frustum"))
        {
            frust.push_back(new app::calibrated_frustum(n));
            slice.push_back(slice_param(0.85));
        }
        else
        {
            frust.push_back(new app::calibrated_frustum(n));
            slice.push_back(slice_param(0.85));
        }

    // Configure the array.

    if ((array = p.find("array")))
    {
        pitch   = array.get_f("pitch", 100.0);
        angle   = array.get_f("angle",   0.0);
        thick   = array.get_f("thick",   0.1);
        shift   = array.get_f("shift",   0.0);
        quality = array.get_f("quality", 1.0);
    }

    // Configure the slices.

    for (app::node n = p.find("slice"); n; n = p.next(n, "slice"))

        if ((i = n.get_i("index")) < int(slice.size()))
        {
            slice[i].cycle = n.get_f("cycle");
            slice[i].step0 = n.get_f("step0");
            slice[i].step1 = n.get_f("step1");
            slice[i].step2 = n.get_f("step2");
            slice[i].step3 = n.get_f("step3");
            slice[i].depth = n.get_f("depth");
        }
}

dpy::lenticular::~lenticular()
{
    assert(channels && int(frust.size()) == channels);

    for (int i = 0; i < channels; ++i)
        delete frust[i];
}

//-----------------------------------------------------------------------------

int dpy::lenticular::get_frusc() const
{
    return int(frust.size());
}

void dpy::lenticular::get_frusv(app::frustum **frusv) const
{
    assert(channels && int(frust.size()) == channels);

    // Add my frustums to the list.

    for (int i = 0; i < channels; ++i)
        frusv[i] = frust[i];
}

void dpy::lenticular::prep(int chanc, const dpy::channel *const *chanv)
{
    assert(channels && int(frust.size()) == channels);

    // Apply the channel view positions to the frustums.

    for (int i = 0; i < channels && i < chanc; ++i)
        frust[i]->set_eye(chanv[i]->get_eye() * debug);
}

void dpy::lenticular::draw(int chanc, const dpy::channel *const *chanv, int frusi)
{
    int i;

    // Draw the scene to the off-screen buffers.

    for (i = 0; i < chanc && i < channels; ++i)
    {
        chanv[i]->bind(quality);
        {
            ::host->draw(frusi + i, frust[i], i);
        }
        chanv[i]->free();
    }

    // Draw the off-screen buffers to the screen.

    for (i = 0; i < chanc && i < channels; ++i)
        chanv[i]->bind_color(GL_TEXTURE0 + i);

    program->bind();
    {
        apply_uniforms();

        glViewport(viewport[0], viewport[1],
                   viewport[2], viewport[3]);
        glBegin(GL_QUADS);
        {
            glVertex2i(-1, -1);
            glVertex2i(+1, -1);
            glVertex2i(+1, +1);
            glVertex2i(-1, +1);
        }
        glEnd();
    }
    program->free();
}

void dpy::lenticular::test(int chanc, const dpy::channel *const *chanv, int index)
{
    int i;

    // Draw the scene to the off-screen buffers.

    for (i = 0; i < chanc && i < channels; ++i)
    {
        chanv[i]->bind();
        chanv[i]->test();
        chanv[i]->free();
    }

    // Draw the off-screen buffers to the screen.

    for (i = 0; i < chanc && i < channels; ++i)
        chanv[i]->bind_color(GL_TEXTURE0 + i);

    program->bind();
    {
        apply_uniforms();

        glViewport(viewport[0], viewport[1],
                   viewport[2], viewport[3]);
        glBegin(GL_QUADS);
        {
            glVertex2i(-1, -1);
            glVertex2i(+1, -1);
            glVertex2i(+1, +1);
            glVertex2i(-1, +1);
        }
        glEnd();
    }
    program->free();
}

//-----------------------------------------------------------------------------

bool dpy::lenticular::pointer_to_3D(app::event *E, int x, int y)
{
    // Let the frustum project the pointer into space.

    assert(!frust.empty());

    double s = double(x - viewport[0]) / viewport[2];
    double t = double(y - viewport[1]) / viewport[3];

    if (0.0 <= s && s < 1.0 && 0.0 <= t && t < 1.0)
        return frust[0]->pointer_to_3D(E, s, t);
    else
        return false;
}

bool dpy::lenticular::process_start(app::event *E)
{
    // Initialize the shader.

    if ((program = ::glob->load_program("lenticular.xml")))
    {
    }

    return false;
}

bool dpy::lenticular::process_close(app::event *E)
{
    // Finalize the shader.

    ::glob->free_program(program);

    program = 0;

    return false;
}

bool dpy::lenticular::process_key(app::event *E)
{
    const bool d = E->data.key.d;
    const int  k = E->data.key.k;
    const int  m = E->data.key.m;

    bool b = false;

    if (d && m & KMOD_CTRL)
    {
        if (m & KMOD_SHIFT)
        {
            if      (k == SDL_SCANCODE_LEFT)  { angle -= 0.00500; b = true; }
            else if (k == SDL_SCANCODE_RIGHT) { angle += 0.00500; b = true; }
            else if (k == SDL_SCANCODE_DOWN)  { pitch -= 0.01000; b = true; }
            else if (k == SDL_SCANCODE_UP)    { pitch += 0.01000; b = true; }
        }
        else
        {
            if (m & KMOD_CAPS)
            {
                if      (k == SDL_SCANCODE_LEFT)  { shift -= 0.00010; b = true; }
                else if (k == SDL_SCANCODE_RIGHT) { shift += 0.00010; b = true; }
                else if (k == SDL_SCANCODE_DOWN)  { thick += 0.00100; b = true; }
                else if (k == SDL_SCANCODE_UP)    { thick -= 0.00100; b = true; }
            }
            else
            {
                if      (k == SDL_SCANCODE_LEFT)  { shift -= 0.00001; b = true; }
                else if (k == SDL_SCANCODE_RIGHT) { shift += 0.00001; b = true; }
                else if (k == SDL_SCANCODE_DOWN)  { thick += 0.00010; b = true; }
                else if (k == SDL_SCANCODE_UP)    { thick -= 0.00010; b = true; }

                else if (k == SDL_SCANCODE_PAGEUP)   { debug = 50.0; b = true; }
                else if (k == SDL_SCANCODE_PAGEDOWN) { debug =  1.0; b = true; }
            }
        }
    }

    if (b)
    {
        // Update the values in the XML node.

        array.set_f("pitch", pitch);
        array.set_f("angle", angle);
        array.set_f("thick", thick);
        array.set_f("shift", shift);

        return true;
    }

    return false;
}

bool dpy::lenticular::process_event(app::event *E)
{
    assert(E);

    // Do the local startup or shutdown.

    switch (E->get_type())
    {
    case E_KEY:   if (process_key  (E)) return true; else break;
    case E_START: if (process_start(E)) return true; else break;
    case E_CLOSE: if (process_close(E)) return true; else break;
    }

    // Let the frustums handle the event.

    std::vector<app::frustum *>::iterator i;

    for (i = frust.begin(); i != frust.end(); ++i)
        if ((*i)->process_event(E))
            return true;

    return false;
}

//-----------------------------------------------------------------------------

vec4 dpy::lenticular::calc_transform(const vec3& u) const
{
    // Compute the debug-scaled pitch and optical thickness.

    const double p = pitch / debug;
    const double t = thick * debug;

    // Compute the pitch and shift reduction due to optical thickness.

    double pp =     p * (u[2] - t) / u[2];
    double ss = shift * (u[2] - t) / u[2];

    // Compute the parallax due to optical thickness.

    double dx = t * u[0] / u[2] - ss;
    double dy = t * u[1] / u[2];

    // Compose the line screen transformation matrix.

    mat4 M = scale(vec3(pp, pp, 1))
           * yrotation(to_radians(-angle))
           * translation(vec3(dx, dy, 0));

    // We only need to transform X, so return the first row.

    return M[0];
}

void dpy::lenticular::apply_uniforms() const
{
    static const std::string index[] = {
        "[0]",  "[1]",  "[2]",  "[3]",  "[4]",  "[5]",  "[6]",  "[7]",
        "[8]",  "[9]", "[10]", "[11]", "[12]", "[13]", "[14]", "[15]"
    };

    const double w = frust[0]->get_width();
    const double h = frust[0]->get_height();
    const double d = w / (3 * viewport[2]);

    // TODO: cache these uniform locations (in process_start).

    program->uniform("eyes",    channels);
    program->uniform("quality", quality);
    program->uniform("offset",  vec3(-d, 0, d));
    program->uniform("corner",  vec2(viewport[0], viewport[1]));
    program->uniform("size",    vec4(w * 0.5, h * 0.5, 0.0, 1.0));

    for (int i = 0; i < channels; ++i)
    {
        // Calculate the transform coefficients.

        const vec3 *c = frust[i]->get_corners();
        const vec3  p = frust[i]->get_eye();

        const vec3 m = mix(c[1], c[2], 0.5);
        const vec3 x = normal(c[1] - c[0]);
        const vec3 y = normal(c[2] - c[0]);
        const vec3 z = normal(cross(x, y));

        vec4 v = calc_transform(mat3(x, y, z) * (p - m));

        // Calculate the linescreen function edges.

        const double e0 = 0.0;
        const double e6 = 1.0;
        const double e1 =      slice[i].step0;
        const double e5 =      slice[i].cycle;
        const double e2 = e1 + slice[i].step1;
        const double e4 = e5 - slice[i].step3;
        const double e3 = e4 - slice[i].step2;

        // Set all uniform values.

        program->uniform("coeff" + index[i], v);
        program->uniform("edge0" + index[i], vec3(e0, e0, e0));
        program->uniform("edge1" + index[i], vec3(e1, e1, e1));
        program->uniform("edge2" + index[i], vec3(e2, e2, e2));
        program->uniform("edge3" + index[i], vec3(e3, e3, e3));
        program->uniform("edge4" + index[i], vec3(e4, e4, e4));
        program->uniform("edge5" + index[i], vec3(e5, e5, e5));
        program->uniform("edge6" + index[i], vec3(e6, e6, e6));
        program->uniform("depth" + index[i], vec3(-slice[i].depth,
                                                  -slice[i].depth,
                                                  -slice[i].depth));
    }
}

//-----------------------------------------------------------------------------
