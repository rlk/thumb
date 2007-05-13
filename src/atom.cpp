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

#include "atom.hpp"
#include "glob.hpp"

//-----------------------------------------------------------------------------

wrl::atom::atom(const ogl::surface *fill,
                const ogl::surface *line) : geom(0), fill(fill), line(line)
{
    load_idt(default_M);
    load_idt(current_M);
}

wrl::atom::atom(const atom& that)
{
    // Copy that atom.

    *this = that;

    // Duplicate ODE state.

    // Duplicate GL state.

    glob->dupe_surface(fill);
    glob->dupe_surface(line);
}

wrl::atom::~atom()
{
    dGeomDestroy(geom);

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

void wrl::atom::get_transform(float M[16])
{
    if (geom)
    {
        const dReal *p = dGeomGetPosition(geom);
        const dReal *R = dGeomGetRotation(geom);

        M[ 0] = float(R[ 0]);
        M[ 1] = float(R[ 4]);
        M[ 2] = float(R[ 8]);
        M[ 3] = 0.0f;

        M[ 4] = float(R[ 1]);
        M[ 5] = float(R[ 5]);
        M[ 6] = float(R[ 9]);
        M[ 7] = 0.0f;

        M[ 8] = float(R[ 2]);
        M[ 9] = float(R[ 6]);
        M[10] = float(R[10]);
        M[11] = 0.0f;

        M[12] = float(p[ 0]);
        M[13] = float(p[ 1]);
        M[14] = float(p[ 2]);
        M[15] = 1.0f;
    }
}

void wrl::atom::set_transform(float M[16])
{
    if (geom)
    {
        dMatrix3 R;

        R[ 0] = (dReal) M[ 0];
        R[ 1] = (dReal) M[ 4];
        R[ 2] = (dReal) M[ 8];
        R[ 3] = (dReal) 0.0f;

        R[ 4] = (dReal) M[ 1];
        R[ 5] = (dReal) M[ 5];
        R[ 6] = (dReal) M[ 9];
        R[ 7] = (dReal) 0.0f;

        R[ 8] = (dReal) M[ 2];
        R[ 9] = (dReal) M[ 6];
        R[10] = (dReal) M[10];
        R[11] = (dReal) 0.0f;

        dGeomSetRotation(geom, R);
        dGeomSetPosition(geom, (dReal) M[12],
                               (dReal) M[13],
                               (dReal) M[14]);
    }
}

//-----------------------------------------------------------------------------

void wrl::atom::set_default()
{
    memcpy(default_M, current_M, 16 * sizeof (float));
}

void wrl::atom::get_default()
{
    memcpy(current_M, default_M, 16 * sizeof (float));

    set_transform(current_M);
}

void wrl::atom::get_surface(dSurfaceParameters& s)
{
    // Merge this atom's surface parameters with the given structure.
}

//-----------------------------------------------------------------------------

void wrl::atom::mult_world(const float M[16])
{
    // Apply the given transformation in world space using left-composition.

    mult_mat_mat(current_M, M, current_M);

    set_transform(current_M);
}

void wrl::atom::mult_local(const float M[16])
{
    // Apply the given transformation in local space using right-composition.

    mult_mat_mat(current_M, M, current_M);

    set_transform(current_M);
}

//-----------------------------------------------------------------------------

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

void wrl::atom::draw_foci(dGeomID focus) const
{
    // Hilite the focused atom in heavy yellow.

    if (geom == focus)
    {
        glColor3f(1.0f, 1.0f, 0.0f);

        glLineWidth(3.0f);

        draw_line();
    }
}

void wrl::atom::draw_stat() const
{
    // Hilite dynamic atoms in green and static atoms in red.

    if (dGeomGetBody(geom))
        glColor3f(0.0f, 1.0f, 0.0f);
    else
        glColor3f(1.0f, 0.0f, 0.0f);

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
    }

    set_quaternion(current_M, q);

    current_M[12] = p[0];
    current_M[13] = p[1];
    current_M[14] = p[2];

    set_default();

    // Initialize parameters.
/*
    std::map<int, param *>::iterator i;

    for (i = params.begin(); i != params.end(); ++i)
        i->second->load(node);
*/
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
/*
    if (body1) mxmlNewInteger(mxmlNewElement(node, "body1"), body1);
    if (body2) mxmlNewInteger(mxmlNewElement(node, "body2"), body2);

    std::map<int, param *>::iterator i;

    for (i = params.begin(); i != params.end(); ++i)
        i->second->save(node);
*/
    return node;
}

//-----------------------------------------------------------------------------
