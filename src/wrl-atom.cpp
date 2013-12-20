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

#include <ogl-opengl.hpp>
#include <etc-ode.hpp>

#include <wrl-atom.hpp>
#include <ogl-pool.hpp>
#include <app-glob.hpp>

//-----------------------------------------------------------------------------

wrl::atom::atom(std::string fill_name,
                std::string line_name) :
    edit_geom(0), body_id(0), name(fill_name), fill(0), line(0)
{
    line_scale = vec3(1, 1, 1);

    if (fill_name.size()) fill = new ogl::unit(fill_name);
    if (line_name.size()) line = new ogl::unit(line_name);
}

wrl::atom::atom(const atom& that) : fill(0), line(0)
{
    param_map::const_iterator i;

    // Copy that atom.

    *this = that;

    // Duplicate ODE state.

    edit_geom = ode_dupe_geom(0, that.edit_geom);

    dGeomSetData(edit_geom, this);

    // Duplicate GL state.

    if (that.fill) fill = new ogl::unit(*that.fill);
    if (that.line) line = new ogl::unit(*that.line);

    // Flush and clone each parameter separately.

    params.clear();

    for (i = that.params.begin(); i != that.params.end(); ++i)
        params[i->first] = new param(*i->second);
}

wrl::atom::~atom()
{
    // Destroy ODE state.

    dGeomDestroy(edit_geom);

    // Delete GL state.

    if (line) delete line;
    if (fill) delete fill;

    // Delete all parameters.

    for (param_map::iterator i = params.begin(); i != params.end(); ++i)
        delete i->second;
}

//-----------------------------------------------------------------------------

void wrl::atom::live(dSpaceID space) const
{
    dSpaceAdd(space, edit_geom);
}

void wrl::atom::dead(dSpaceID space) const
{
    dSpaceRemove(space, edit_geom);
}

//-----------------------------------------------------------------------------
#include <app-view.hpp>

double wrl::atom::set_lighting(int light, int i, int m, int w) const
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
    const mat4 M = ::view->get_transform();
    // const mat4 T = transpose(inverse(M));

    const vec4 p = M * vec4( wvector(current_M), w);
    const vec4 v = M * vec4(-yvector(current_M), 0);

    GLfloat P[4] = { p[0], p[1], p[2], p[3] };
    GLfloat V[4] = { v[0], v[1], v[2], v[2] };

    glLightfv(L, GL_POSITION,       P);
    glLightfv(L, GL_SPOT_DIRECTION, V);

    return cutoff;
}

void wrl::atom::get_surface(dSurfaceParameters& s)
{
    // Merge this atom's surface parameters with the given structure.

    param_map::iterator i;

    if ((i = params.find(wrl::param::mu))       != params.end())
        s.mu       = std::min(s.mu,       dReal(i->second->value()));

    if ((i = params.find(wrl::param::bounce))   != params.end())
        s.bounce   = std::max(s.bounce,   dReal(i->second->value()));

    if ((i = params.find(wrl::param::soft_erp)) != params.end())
        s.soft_erp = std::min(s.soft_erp, dReal(i->second->value()));

    if ((i = params.find(wrl::param::soft_cfm)) != params.end())
        s.soft_cfm = std::max(s.soft_cfm, dReal(i->second->value()));
}

//-----------------------------------------------------------------------------

void wrl::atom::transform(const mat4& T)
{
    // Apply the given transformation to the current.

    const mat4 M = T * current_M;
    const mat4 N = M * scale(line_scale);

    current_M = M;
    default_M = M;

    ode_set_geom_transform(edit_geom, M);

    // Apply the current transform to the fill and line units.

    if (fill)
        fill->transform(M, inverse(M));

    if (line)
        line->transform(N, inverse(N));
}

mat4 wrl::atom::get_world() const
{
    // Return the world-aligned coordinate system centered on this atom.

    mat4 M;

    M[0][3] = current_M[0][3];
    M[1][3] = current_M[1][3];
    M[2][3] = current_M[2][3];

    return M;
}

mat4 wrl::atom::get_local() const
{
    return current_M;
}

//-----------------------------------------------------------------------------

void wrl::atom::set_param(int key, std::string& expr)
{
    // Allow only valid parameters as initialized by the entity constructor.

    if (params.find(key) != params.end())
        params[key]->set(expr);
}

bool wrl::atom::get_param(int key, std::string& expr)
{
    // Return false to indicate an invalid parameter was requested.

    if (params.find(key) != params.end())
    {
        params[key]->get(expr);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------

void wrl::atom::load(app::node node)
{
    app::node n;

    quat q;
    vec3 p;

    // Initialize the transform and body mappings.

    if ((n = node.find("rot_x"))) q[0] = n.get_f();
    if ((n = node.find("rot_y"))) q[1] = n.get_f();
    if ((n = node.find("rot_z"))) q[2] = n.get_f();
    if ((n = node.find("rot_w"))) q[3] = n.get_f();

    if ((n = node.find("pos_x"))) p[0] = n.get_f();
    if ((n = node.find("pos_y"))) p[1] = n.get_f();
    if ((n = node.find("pos_z"))) p[2] = n.get_f();

    if ((n = node.find("body"))) body_id = n.get_i();

    // Compute and apply the transform.

    current_M = mat4(mat3(q));

    current_M[0][3] = p[0];
    current_M[1][3] = p[1];
    current_M[2][3] = p[2];

    default_M = current_M;

    // Initialize parameters.

    for (param_map::iterator i = params.begin(); i != params.end(); ++i)
        i->second->load(node);
}

void wrl::atom::save(app::node node)
{
    const vec3 x = xvector(default_M);
    const vec3 y = yvector(default_M);
    const vec3 z = zvector(default_M);
    const vec3 p = wvector(default_M);

    const quat q(mat3(x, y, z));

    // Add the entity transform to this element.

    app::node n;

    n = app::node("rot_x"); n.set_f(q[0]); n.insert(node);
    n = app::node("rot_y"); n.set_f(q[1]); n.insert(node);
    n = app::node("rot_z"); n.set_f(q[2]); n.insert(node);
    n = app::node("rot_w"); n.set_f(q[3]); n.insert(node);

    n = app::node("pos_x"); n.set_f(p[0]); n.insert(node);
    n = app::node("pos_y"); n.set_f(p[1]); n.insert(node);
    n = app::node("pos_z"); n.set_f(p[2]); n.insert(node);

    if (body_id)
    {
        n = app::node("body");
        n.set_i(body_id);
        n.insert(node);
    }

    // Add entity parameters to this element.

    for (param_map::iterator i = params.begin(); i != params.end(); ++i)
        i->second->save(node);
}

//-----------------------------------------------------------------------------
