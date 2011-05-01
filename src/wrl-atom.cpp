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

#include <ogl-opengl.hpp>
#include <sys-matrix.hpp>
#include <sys-ode.hpp>

#include <wrl-atom.hpp>
#include <ogl-pool.hpp>
#include <app-glob.hpp>

//-----------------------------------------------------------------------------

wrl::atom::atom(std::string fill_name,
                std::string line_name) :
    edit_geom(0), body_id(0), name(fill_name), fill(0), line(0)
{
    load_idt(default_M);
    load_idt(current_M);

    line_scale[0] = line_scale[1] = line_scale[2] = 1.0;

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

void wrl::atom::mult_M() const
{
    // Apply the current transform as model matrix.

    glMultMatrixd(current_M);
}

void wrl::atom::mult_R() const
{
    double M[16];

    // Apply the current view rotation transform.

    M[ 0] = +current_M[ 0];
    M[ 1] = +current_M[ 4];
    M[ 2] = +current_M[ 8];
    M[ 3] = 0;
    M[ 4] = +current_M[ 1];
    M[ 5] = +current_M[ 5];
    M[ 6] = +current_M[ 9];
    M[ 7] = 0;
    M[ 8] = +current_M[ 2];
    M[ 9] = +current_M[ 6];
    M[10] = +current_M[10];
    M[11] = 0;
    M[12] = 0;
    M[13] = 0;
    M[14] = 0;
    M[15] = 1;

    glMultMatrixd(M);
}

void wrl::atom::mult_T() const
{
    // Apply the current view translation transform.

    glTranslated(-current_M[12],
                 -current_M[13],
                 -current_M[14]);
}

void wrl::atom::mult_V() const
{
    mult_R();
    mult_T();
}

void wrl::atom::mult_P() const
{
}

//-----------------------------------------------------------------------------

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

void wrl::atom::transform(const double *T)
{
    // Apply the given transformation to the current.

    double M[16];
    double I[16];

    mult_mat_mat(M, T, current_M);

    load_mat(current_M, M);
    load_mat(default_M, M);
    load_inv(I, M);

    ode_set_geom_transform(edit_geom, M);

    // Apply the current transform to the fill and line units.

    if (fill)
        fill->transform(M, I);

    if (line)
    {
        Rmul_scl_mat(M, line_scale[0], line_scale[1], line_scale[2]);

        line->transform(M, M);
    }
}

void wrl::atom::get_world(double *M) const
{
    // Return the world-aligned coordinate system centered on this atom.

    load_idt(M);

    M[12] = current_M[12];
    M[13] = current_M[13];
    M[14] = current_M[14];
}

void wrl::atom::get_local(double *M) const
{
    // Return the local-aligned coordinate system centered on this atom.

    load_mat(M, current_M);
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

    double q[4] = { 0, 0, 0, 1 };
    double p[3] = { 0, 0, 0    };

    load_idt(current_M);

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

    set_quaternion(current_M, q);

    current_M[12] = p[0];
    current_M[13] = p[1];
    current_M[14] = p[2];

    load_mat(default_M, current_M);

    // Initialize parameters.

    for (param_map::iterator i = params.begin(); i != params.end(); ++i)
        i->second->load(node);
}

void wrl::atom::save(app::node node)
{
    double q[4];

    // Add the entity transform to this element.

    get_quaternion(q, default_M);

    app::node n;

    n = app::node("rot_x"); n.set_f(q[0]); n.insert(node);
    n = app::node("rot_y"); n.set_f(q[1]); n.insert(node);
    n = app::node("rot_z"); n.set_f(q[2]); n.insert(node);
    n = app::node("rot_w"); n.set_f(q[3]); n.insert(node);

    n = app::node("pos_x"); n.set_f(default_M[12]); n.insert(node);
    n = app::node("pos_y"); n.set_f(default_M[13]); n.insert(node);
    n = app::node("pos_z"); n.set_f(default_M[14]); n.insert(node);

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
