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

#include <wrl-light.hpp>
#include <ogl-pool.hpp>

//-----------------------------------------------------------------------------

wrl::light::light(std::string fill) : sphere(fill)
{
}


wrl::d_light::d_light() : light("solid/d-light.obj")
{
    params[GL_RED]                   = new param("color_red",             "1.0");
    params[GL_GREEN]                 = new param("color_green",           "1.0");
    params[GL_BLUE]                  = new param("color_blue",            "1.0");
}

//wrl::s_light::s_light() : light("solid/s-light.obj")
wrl::s_light::s_light() : light("solid/wooden-cube.obj")
{
    params[GL_RED]                   = new param("color_red",             "1.0");
    params[GL_GREEN]                 = new param("color_green",           "1.0");
    params[GL_BLUE]                  = new param("color_blue",            "1.0");
    params[GL_CONSTANT_ATTENUATION]  = new param("attenuation_constant",  "1.0");
    params[GL_LINEAR_ATTENUATION]    = new param("attenuation_linear",    "0.0");
    params[GL_QUADRATIC_ATTENUATION] = new param("attenuation_quadratic", "0.0");
    params[GL_SPOT_EXPONENT]         = new param("spot_exponent",         "0.0");
    params[GL_SPOT_CUTOFF]           = new param("spot_cutoff",          "30.0");
}

//-----------------------------------------------------------------------------

#include <app-view.hpp>

double wrl::light::set_lighting(int light, const vec4& p,
                                           const vec4& v, int i, int m) const
{
    const GLenum L = GL_LIGHT0 + light;

    GLfloat d[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat a[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    double cutoff = 90.0;

    // Extract all lighting-oriented parameters and pass them to OpenGL.

    for (param_map::const_iterator i = params.begin(); i != params.end(); ++i)
        switch (i->first)
        {
            case GL_RED:   d[0] = GLfloat(i->second->value()); break;
            case GL_GREEN: d[1] = GLfloat(i->second->value()); break;
            case GL_BLUE:  d[2] = GLfloat(i->second->value()); break;

            case GL_SPOT_CUTOFF:

                cutoff = i->second->value();

            case GL_QUADRATIC_ATTENUATION:
            case GL_CONSTANT_ATTENUATION:
            case GL_LINEAR_ATTENUATION:
            case GL_SPOT_EXPONENT:

                glLightf(L, GLenum(i->first), GLfloat(i->second->value()));
                break;
        }

    // Diffuse and ambient colors.

    a[0] = GLfloat(i    ) / GLfloat(m);
    a[1] = GLfloat(i + 1) / GLfloat(m);

    glLightfv(L, GL_DIFFUSE, d);
    glLightfv(L, GL_AMBIENT, a);

    // Position and direction.
    const mat4 V = ::view->get_transform();

    const vec4 Vp = V * p;
    const vec4 Vv = V * v;

    GLfloat P[4] = { Vp[0], Vp[1], Vp[2], Vp[3] };
    GLfloat D[4] = { Vv[0], Vv[1], Vv[2], Vv[3] };

    glLightfv(L, GL_POSITION,       P);
    glLightfv(L, GL_SPOT_DIRECTION, D);

    return cutoff;
}

//-----------------------------------------------------------------------------

void wrl::light::play_init()
{
    // if (fill) fill->set_mode(false);

    sphere::play_init();
}

void wrl::light::play_fini()
{
    // if (fill) fill->set_mode(true);

    sphere::play_fini();
}

//-----------------------------------------------------------------------------

void wrl::d_light::save(app::node node)
{
    app::node n("light");

    n.set_s("type", "d-light");
    n.insert(node);
    solid::save(n);
}

void wrl::s_light::save(app::node node)
{
    app::node n("light");

    n.set_s("type", "s-light");
    n.insert(node);
    solid::save(n);
}

//-----------------------------------------------------------------------------
