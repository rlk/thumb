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

#include "opengl.hpp"
#include "matrix.hpp"
#include "odeutil.hpp"

#include "atom.hpp"
#include "glob.hpp"

//-----------------------------------------------------------------------------

wrl::atom::atom(const ogl::surface *fill,
                const ogl::surface *line) :
    edit_geom(0), body_id(0), fill(fill), line(line)
{
    load_idt(default_M);
    load_idt(current_M);
}

wrl::atom::atom(const atom& that)
{
    param_map::const_iterator i;

    // Copy that atom.

    *this = that;

    // Duplicate ODE state.

    edit_geom = ode_dupe_geom(dGeomGetSpace(that.edit_geom), that.edit_geom);

    // Duplicate GL state.

    glob->dupe_surface(fill);
    glob->dupe_surface(line);

    // Flush and clone each parameter separately.

    params.clear();

    for (i = that.params.begin(); i != that.params.end(); ++i)
        params[i->first] = new param(*i->second);
}

wrl::atom::~atom()
{
    dGeomDestroy(edit_geom);

    glob->free_surface(fill);
    glob->free_surface(line);
}

//-----------------------------------------------------------------------------

void wrl::atom::mult_M() const
{
    // Apply the current transform as model matrix.

    glMultMatrixf(current_M);
}

void wrl::atom::mult_R() const
{
    float M[16];

    // Apply the current view rotation transform.

    M[ 0] = +current_M[ 0];
    M[ 1] = +current_M[ 4];
    M[ 2] = +current_M[ 8];
    M[ 3] = 0.0f;
    M[ 4] = +current_M[ 1];
    M[ 5] = +current_M[ 5];
    M[ 6] = +current_M[ 9];
    M[ 7] = 0.0f;
    M[ 8] = +current_M[ 2];
    M[ 9] = +current_M[ 6];
    M[10] = +current_M[10];
    M[11] = 0.0f;
    M[12] = 0.0f;
    M[13] = 0.0f;
    M[14] = 0.0f;
    M[15] = 1.0f;

    glMultMatrixf(M);
}

void wrl::atom::mult_T() const
{
    // Apply the current view translation transform.

    glTranslatef(-current_M[12],
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

void wrl::atom::transform(const float M[16])
{
    // Apply the given transformation.

    float T[16];

    mult_mat_mat(T, M, current_M);

    load_mat(current_M, T);
    load_mat(default_M, T);

    ode_set_geom_transform(edit_geom, T);
}

void wrl::atom::get_world(float M[16]) const
{
    // Return the world-aligned coordinate system centered on this atom.

    load_idt(M);

    M[12] = current_M[12];
    M[13] = current_M[13];
    M[14] = current_M[14];
}

void wrl::atom::get_local(float M[16]) const
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

void wrl::atom::draw_foci(dGeomID focus) const
{
    if (focus)
    {
        const atom *a = (const atom *) dGeomGetData(focus);

        if (a == this)
        {
            // Hilite the focused atom in heavy yellow.

            glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
            glLineWidth(3.0f);

            draw_line();
        }
        else if (a->body() && a->body() == body())
        {
            // Highlight the body of the focus in light yellow.

            glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
            glLineWidth(1.0f);

            draw_line();
        }
        else if (a->join() && a->join() == body())
        {
            // Highlight the join target of the focus in light magenta.

            glColor4f(1.0f, 0.0f, 1.0f, 0.5f);
            glLineWidth(1.0f);

            draw_line();
        }
    }
}

void wrl::atom::draw_stat() const
{
    // Hilite dynamic atoms in green and static atoms in red.

    if (body_id)
        glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
    else
        glColor4f(1.0f, 0.0f, 0.0f, 0.5f);

    glLineWidth(1.0f);

    draw_line();
}

//-----------------------------------------------------------------------------

void wrl::atom::draw_fill() const
{
    if (fill)
    {
        glPushMatrix();
        {
            mult_M();
            fill->draw(DRAW_OPAQUE | DRAW_UNLIT);
        }
        glPopMatrix();
    }
}

void wrl::atom::draw_line() const
{
    if (line)
    {
        glPushMatrix();
        {
            mult_M();
            line->draw(DRAW_OPAQUE | DRAW_UNLIT);
        }
        glPopMatrix();
    }
}

//-----------------------------------------------------------------------------

void wrl::atom::load(mxml_node_t *node)
{
    mxml_node_t *n;

    load_idt(current_M);

    float p[3] = { 0, 0, 0    };
    float q[4] = { 0, 0, 0, 1 };

    // Initialize the transform and body mappings.

    for (n = node->child; n; 
         n = mxmlFindElement(n, node, 0, 0, 0, MXML_NO_DESCEND))
    {
        std::string name(n->value.element.name);

        if      (name == "rot_x") q[0] = float(n->child->value.real);
        else if (name == "rot_y") q[1] = float(n->child->value.real);
        else if (name == "rot_z") q[2] = float(n->child->value.real);
        else if (name == "rot_w") q[3] = float(n->child->value.real);

        else if (name == "pos_x") p[0] = float(n->child->value.real);
        else if (name == "pos_y") p[1] = float(n->child->value.real);
        else if (name == "pos_z") p[2] = float(n->child->value.real);

        else if (name == "body") body_id = n->child->value.integer;
    }

    set_quaternion(current_M, q);

    current_M[12] = p[0];
    current_M[13] = p[1];
    current_M[14] = p[2];

    load_mat(default_M, current_M);

    ode_set_geom_transform(edit_geom, current_M);

    // Initialize parameters.

    param_map::iterator i;

    for (i = params.begin(); i != params.end(); ++i)
        i->second->load(node);
}

mxml_node_t *wrl::atom::save(mxml_node_t *node)
{
    float q[4];

    // Add the entity transform to this element.

    get_quaternion(q, default_M);

    mxmlNewReal(mxmlNewElement(node, "rot_x"), q[0]);
    mxmlNewReal(mxmlNewElement(node, "rot_y"), q[1]);
    mxmlNewReal(mxmlNewElement(node, "rot_z"), q[2]);
    mxmlNewReal(mxmlNewElement(node, "rot_w"), q[3]);

    mxmlNewReal(mxmlNewElement(node, "pos_x"), default_M[12]);
    mxmlNewReal(mxmlNewElement(node, "pos_y"), default_M[13]);
    mxmlNewReal(mxmlNewElement(node, "pos_z"), default_M[14]);

    // Add entity parameters to this element.

    if (body_id) mxmlNewInteger(mxmlNewElement(node, "body"), body_id);

    param_map::iterator i;

    for (i = params.begin(); i != params.end(); ++i)
        i->second->save(node);

    return node;
}

//-----------------------------------------------------------------------------
