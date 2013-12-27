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
    ambient[0]     =  0;
    ambient[1]     =  0;
    ambient[2]     =  0;
    ambient[3]     =  1;

    diffuse[0]     =  0;
    diffuse[1]     =  0;
    diffuse[2]     =  0;
    diffuse[3]     =  1;

    position[0]    =  0;
    position[1]    =  0;
    position[2]    =  1;
    position[3]    =  0;

    direction[0]   =  0;
    direction[1]   =  0;
    direction[2]   = -1;
    direction[3]   =  0;

    attenuation[0] =  1;
    attenuation[1] =  0;
    attenuation[2] =  0;

    exponent       =  0;
    cutoff         = 90;
}


wrl::d_light::d_light() : light("solid/d-light.obj")
{
    params[GL_RED]                   = new param("color_red",             "1.0");
    params[GL_GREEN]                 = new param("color_green",           "1.0");
    params[GL_BLUE]                  = new param("color_blue",            "1.0");
}

wrl::s_light::s_light() : light("solid/s-light.obj")
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

bool wrl::light::has_light()
{
    return (params[GL_RED  ]->value() > 0 ||
            params[GL_GREEN]->value() > 0 ||
            params[GL_BLUE ]->value() > 0);
}

void wrl::light::apply_light(int light) const
{
    GLenum L = GL_LIGHT0 + light;

    glPushMatrix();
    {
        glLoadIdentity();
        glLightfv(L, GL_DIFFUSE,               diffuse);
        glLightfv(L, GL_AMBIENT,               ambient);
        glLightfv(L, GL_POSITION,              position);
        glLightfv(L, GL_SPOT_DIRECTION,        direction);
        glLightf (L, GL_SPOT_EXPONENT,         exponent);
        glLightf (L, GL_SPOT_CUTOFF,           cutoff);
        glLightf (L, GL_CONSTANT_ATTENUATION,  attenuation[0]);
        glLightf (L, GL_LINEAR_ATTENUATION,    attenuation[1]);
        glLightf (L, GL_QUADRATIC_ATTENUATION, attenuation[2]);
    }
    glPopMatrix();
}

#include <app-view.hpp>

double wrl::light::cache_light(int light, const vec4& p,
                                          const vec4& v, int i, int m)
{
    // Extract all lighting-oriented parameters.

    for (param_map::const_iterator c = params.begin(); c != params.end(); ++c)
    {
        const GLfloat v = GLfloat(c->second->value());

        switch (c->first)
        {
            case GL_RED:                       diffuse[0] = v; break;
            case GL_GREEN:                     diffuse[1] = v; break;
            case GL_BLUE:                      diffuse[2] = v; break;
            case GL_CONSTANT_ATTENUATION:  attenuation[0] = v; break;
            case GL_LINEAR_ATTENUATION:    attenuation[1] = v; break;
            case GL_QUADRATIC_ATTENUATION: attenuation[2] = v; break;
            case GL_SPOT_EXPONENT:            exponent    = v; break;
            case GL_SPOT_CUTOFF:                cutoff    = v; break;
        }
    }

    // The ambient color is used to convey split parameters.

    ambient[0] = GLfloat(i    ) / GLfloat(m);
    ambient[1] = GLfloat(i + 1) / GLfloat(m);

    // Compute the eye-space position and direction.

    const mat4 V  = ::view->get_transform();
    const vec4 Vp = V * p;
    const vec4 Vv = V * v;

    position[0]  = GLfloat(Vp[0]);
    position[1]  = GLfloat(Vp[1]);
    position[2]  = GLfloat(Vp[2]);
    position[3]  = GLfloat(Vp[3]);

    direction[0] = GLfloat(Vv[0]);
    direction[1] = GLfloat(Vv[1]);
    direction[2] = GLfloat(Vv[2]);
    direction[3] = GLfloat(Vv[3]);

    // Feed these values to OpenGL and return the spot field-of-view.

    apply_light(light);

    return double(cutoff);
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

void wrl::light::load(app::node node)
{
    atom::load(node); // Skip solid::load.
}

void wrl::d_light::save(app::node node)
{
    app::node n("light");

    n.set_s("type", "d-light");
    n.insert(node);
    atom::save(n);
}

void wrl::s_light::save(app::node node)
{
    app::node n("light");

    n.set_s("type", "s-light");
    n.insert(node);
    atom::save(n);
}

//-----------------------------------------------------------------------------
