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
#include <app-data.hpp>

//-----------------------------------------------------------------------------

// Locate a wire frame object for the named solid.

static std::string line_from_fill(std::string fill)
{
    std::string::size_type p;
    std::string line = fill;

    if ((p = line.find("solid/")) != std::string::npos)
    {
        line.replace(p, 5, "wire");

        if (::data->find(line))
            return line;
    }
    return fill;
}

//-----------------------------------------------------------------------------

wrl::atom::atom(app::node node, std::string _fill_name,
                                std::string _line_name, bool center) :
    edit_geom(0),
    body_id(0),
    fill_name(_fill_name),
    line_name(_line_name),
    fill(0),
    line(0),
    line_scale(1, 1, 1)
{
    // Load the named file and line units.

    if (!fill_name.empty() &
         line_name.empty()) line_name = line_from_fill(fill_name);

    if (!fill_name.empty()) fill = new ogl::unit(fill_name, center);
    if (!line_name.empty()) line = new ogl::unit(line_name, center);

    fill->merge_bound(fill_bound);
    line->merge_bound(line_bound);

    // Initialize the transform and body mappings.

    if (node)
    {
        app::node n;

        quat q;
        vec3 p;

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
}
#if 0
wrl::atom::atom(const atom& that) : fill(0), line(0)
{
    param_map::const_iterator i;

    // Copy that atom.

    *this = that;

    // Duplicate ODE state.

    edit_geom = that.new_geom(0);
    if (edit_geom) dGeomSetData(edit_geom, this);

    // Duplicate GL state.

    if (that.fill) fill = new ogl::unit(*that.fill);
    if (that.line) line = new ogl::unit(*that.line);

    // Flush and clone each parameter separately.

    params.clear();

    for (i = that.params.begin(); i != that.params.end(); ++i)
        params[i->first] = new param(*i->second);
}
#endif

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

void wrl::atom::get_surface(dSurfaceParameters& s) const
{
    // Merge this atom's surface parameters with the given structure.

    param_map::const_iterator i;

    if ((i = params.find(wrl::param::mu))       != params.end())
        s.mu       = std::min(s.mu,       dReal(i->second->value()));

    if ((i = params.find(wrl::param::bounce))   != params.end())
        s.bounce   = std::max(s.bounce,   dReal(i->second->value()));

    if ((i = params.find(wrl::param::soft_erp)) != params.end())
        s.soft_erp = std::min(s.soft_erp, dReal(i->second->value()));

    if ((i = params.find(wrl::param::soft_cfm)) != params.end())
        s.soft_cfm = std::max(s.soft_cfm, dReal(i->second->value()));
}

double wrl::atom::get_lighting(vec2& brightness) const
{
    brightness = vec2(0.0, 0.0);
    return 0.0;
}

//-----------------------------------------------------------------------------

void wrl::atom::transform(const mat4& T)
{
    // Apply the given transformation to the current.

    const mat4 M = T * current_M;
    const mat4 N = M * scale(line_scale);

    current_M = M;
    default_M = M;

    bGeomSetTransform(edit_geom, M);

    // Apply the current transform to the fill and line units.

    if (fill) fill->transform(M, inverse(M));
    if (line) line->transform(N, inverse(N));
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
