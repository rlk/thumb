//  Copyright (C) 2005 Robert Kooima
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

#include <iostream>

#include "opengl.hpp"
#include "joint.hpp"
#include "glob.hpp"

//-----------------------------------------------------------------------------

void wrl::joint::play_init(dBodyID)
{
}

void wrl::joint::play_fini()
{
}

//-----------------------------------------------------------------------------

wrl::joint::joint(dSpaceID space, const ogl::surface *fill,
                                  const ogl::surface *line) :
    atom(fill, line), join_id(0)
{
    geom = dCreateSphere(space, 0.25f);

    dGeomSetData(geom, this);
    set_transform(current_M);
}

wrl::joint::~joint()
{
}

//-----------------------------------------------------------------------------

wrl::ball::ball(dSpaceID space) :
    joint(space, glob->load_surface("joint/joint_ball.obj"),
                 glob->load_surface("wire/wire_sphere.obj"))
{
}

wrl::hinge::hinge(dSpaceID space) :
    joint(space, glob->load_surface("joint/joint_hinge.obj"),
                 glob->load_surface("wire/wire_sphere.obj"))
{
    params[dParamVel]      = new param("dParamVel",      "0.0");
    params[dParamFMax]     = new param("dParamFMax",     "0.0");
    params[dParamCFM]      = new param("dParamCFM",      "0.0");
    params[dParamBounce]   = new param("dParamBounce",   "0.5");
    params[dParamLoStop]   = new param("dParamLoStop",  "-inf");
    params[dParamHiStop]   = new param("dParamHiStop",   "inf");
    params[dParamStopERP]  = new param("dParamStopERP",  "0.2");
    params[dParamStopCFM]  = new param("dParamStopCFM",  "0.0");
}

wrl::hinge2::hinge2(dSpaceID space) :
    joint(space, glob->load_surface("joint/joint_hinge2.obj"),
                 glob->load_surface("wire/wire_sphere.obj"))
{
    params[dParamVel]      = new param("dParamVel",      "0.0");
    params[dParamFMax]     = new param("dParamFMax",     "0.0");
    params[dParamCFM]      = new param("dParamCFM",      "0.0");
    params[dParamBounce]   = new param("dParamBounce",   "0.5");
    params[dParamLoStop]   = new param("dParamLoStop",  "-inf");
    params[dParamHiStop]   = new param("dParamHiStop",   "inf");
    params[dParamStopERP]  = new param("dParamStopERP",  "0.2");
    params[dParamStopCFM]  = new param("dParamStopCFM",  "0.0");

    params[dParamVel2]     = new param("dParamVel2",     "0.0");
    params[dParamFMax2]    = new param("dParamFMax2",    "0.0");
    params[dParamCFM2]     = new param("dParamCFM2",     "0.0");
    params[dParamBounce2]  = new param("dParamBounce2",  "0.5");
    params[dParamLoStop2]  = new param("dParamLoStop2", "-inf");
    params[dParamHiStop2]  = new param("dParamHiStop2",  "inf");
    params[dParamStopERP2] = new param("dParamStopERP2", "0.2");
    params[dParamStopCFM2] = new param("dParamStopCFM2", "0.0");

    params[dParamSuspensionERP] = new param("dParamSuspensionERP", "0.2");
    params[dParamSuspensionCFM] = new param("dParamSuspensionCFM", "0.0");
}

wrl::slider::slider(dSpaceID space) :
    joint(space, glob->load_surface("joint/joint_slider.obj"),
                 glob->load_surface("wire/wire_sphere.obj"))
{
    params[dParamVel]      = new param("dParamVel",      "0.0");
    params[dParamFMax]     = new param("dParamFMax",     "0.0");
    params[dParamCFM]      = new param("dParamCFM",      "0.0");
    params[dParamBounce]   = new param("dParamBounce",   "0.5");
    params[dParamLoStop]   = new param("dParamLoStop",  "-inf");
    params[dParamHiStop]   = new param("dParamHiStop",   "inf");
    params[dParamStopERP]  = new param("dParamStopERP",  "0.2");
    params[dParamStopCFM]  = new param("dParamStopCFM",  "0.0");
}

wrl::amotor::amotor(dSpaceID space) :
    joint(space, glob->load_surface("joint/joint_amotor.obj"),
                 glob->load_surface("wire/wire_sphere.obj"))
{
    params[dParamVel]      = new param("dParamVel",      "0.0");
    params[dParamFMax]     = new param("dParamFMax",     "0.0");
    params[dParamCFM]      = new param("dParamCFM",      "0.0");
    params[dParamBounce]   = new param("dParamBounce",   "0.5");
    params[dParamLoStop]   = new param("dParamLoStop",  "-inf");
    params[dParamHiStop]   = new param("dParamHiStop",   "inf");
    params[dParamStopERP]  = new param("dParamStopERP",  "0.2");
    params[dParamStopCFM]  = new param("dParamStopCFM",  "0.0");

    params[dParamVel2]     = new param("dParamVel2",     "0.0");
    params[dParamFMax2]    = new param("dParamFMax2",    "0.0");
    params[dParamCFM2]     = new param("dParamCFM2",     "0.0");
    params[dParamBounce2]  = new param("dParamBounce2",  "0.5");
    params[dParamLoStop2]  = new param("dParamLoStop2", "-inf");
    params[dParamHiStop2]  = new param("dParamHiStop2",  "inf");
    params[dParamStopERP2] = new param("dParamStopERP2", "0.2");
    params[dParamStopCFM2] = new param("dParamStopCFM2", "0.0");

    params[dParamVel3]     = new param("dParamVel3",     "0.0");
    params[dParamFMax3]    = new param("dParamFMax3",    "0.0");
    params[dParamCFM3]     = new param("dParamCFM3",     "0.0");
    params[dParamBounce3]  = new param("dParamBounce3",  "0.5");
    params[dParamLoStop3]  = new param("dParamLoStop3", "-inf");
    params[dParamHiStop3]  = new param("dParamHiStop3",  "inf");
    params[dParamStopERP3] = new param("dParamStopERP3", "0.2");
    params[dParamStopCFM3] = new param("dParamStopCFM3", "0.0");
}

wrl::universal::universal(dSpaceID space) :
    joint(space, glob->load_surface("joint/joint_universal.obj"),
                 glob->load_surface("wire/wire_sphere.obj"))
{
    params[dParamVel]      = new param("dParamVel",      "0.0");
    params[dParamFMax]     = new param("dParamFMax",     "0.0");
    params[dParamCFM]      = new param("dParamCFM",      "0.0");
    params[dParamBounce]   = new param("dParamBounce",   "0.5");
    params[dParamLoStop]   = new param("dParamLoStop",  "-inf");
    params[dParamHiStop]   = new param("dParamHiStop",   "inf");
    params[dParamStopERP]  = new param("dParamStopERP",  "0.2");
    params[dParamStopCFM]  = new param("dParamStopCFM",  "0.0");

    params[dParamVel2]     = new param("dParamVel2",     "0.0");
    params[dParamFMax2]    = new param("dParamFMax2",    "0.0");
    params[dParamCFM2]     = new param("dParamCFM2",     "0.0");
    params[dParamBounce2]  = new param("dParamBounce2",  "0.5");
    params[dParamLoStop2]  = new param("dParamLoStop2", "-inf");
    params[dParamHiStop2]  = new param("dParamHiStop2",  "inf");
    params[dParamStopERP2] = new param("dParamStopERP2", "0.2");
    params[dParamStopCFM2] = new param("dParamStopCFM2", "0.0");
}

//-----------------------------------------------------------------------------
/*
void wrl::ball::play_init(dBodyID body)
{
    dJointAttach(join, body, 0);

    joint::play_init(body);
}

void wrl::hinge::play_init(dBodyID body)
{
    dJointAttach(join, body, 0);

    joint::play_init(body);
}

void wrl::hinge2::play_init(dBodyID body)
{
    dJointAttach(join, body, 0);

    joint::play_init(body);
}

void wrl::slider::play_init(dBodyID body)
{
    dJointAttach(join, body, 0);

    joint::play_init(body);
}

void wrl::amotor::play_init(dBodyID body)
{
    dJointAttach(join, body, 0);

    joint::play_init(body);
}

void wrl::universal::play_init(dBodyID body)
{
    dJointAttach(join, body, 0);

    joint::play_init(body);
}
*/
//-----------------------------------------------------------------------------
/*
void wrl::ball::play_join(dBodyID body1)
{
    dBodyID body0 = dJointGetBody(join, 0);

    if (body0 || body1)
    {
        const float *M = current_M;

        // Set ball joint geometry parameters.

        dJointAttach       (join, body0, body1);
        dJointSetBallAnchor(join, M[12], M[13], M[14]);
    }
}

void wrl::hinge::play_join(dBodyID body1)
{
    dBodyID body0 = dJointGetBody(join, 0);

    if (body0 || body1)
    {
        const float *M = current_M;

        // Set hinge geometry parameters.

        dJointAttach        (join, body0, body1);
        dJointSetHingeAxis  (join, M[ 0], M[ 1], M[ 2]);
        dJointSetHingeAnchor(join, M[12], M[13], M[14]);
    }
}

void wrl::hinge2::play_join(dBodyID body1)
{
    dBodyID body0 = dJointGetBody(join, 0);

    if (body0 || body1)
    {
        const float *M = current_M;

        // Set hinge2 geometry parameters.

        dJointAttach         (join, body0, body1);
        dJointSetHinge2Axis2 (join, M[ 0], M[ 1], M[ 2]);
        dJointSetHinge2Axis1 (join, M[ 4], M[ 5], M[ 6]);
        dJointSetHinge2Anchor(join, M[12], M[13], M[14]);
    }
}

void wrl::slider::play_join(dBodyID body1)
{
    dBodyID body0 = dJointGetBody(join, 0);

    if (body0 || body1)
    {
        const float *M = current_M;

        // Set slider geometry parameters.

        dJointAttach       (join, body0, body1);
        dJointSetSliderAxis(join, M[ 8], M[ 9], M[10]);
    }
}

void wrl::amotor::play_join(dBodyID body1)
{
    dBodyID body0 = dJointGetBody(join, 0);

    if (body0 || body1)
    {
        const float *M = current_M;

        int a = body0 ? 1 : 0;
        int b = body1 ? 2 : 0;

        // Set angular motor geometry parameters.

        dJointAttach       (join, body0, body1);
        dJointSetAMotorMode(join, dAMotorEuler);
        dJointSetAMotorAxis(join, 0, a, M[ 0], M[ 1], M[ 2]);
        dJointSetAMotorAxis(join, 2, b, M[ 8], M[ 9], M[10]);
    }
}

void wrl::universal::play_join(dBodyID body1)
{
    dBodyID body0 = dJointGetBody(join, 0);

    if (body0 || body1)
    {
        const float *M = current_M;

        // Set universal joint geometry parameters.

        dJointAttach            (join, body0, body1);
        dJointSetUniversalAxis1 (join, M[ 0], M[ 1], M[ 2]);
        dJointSetUniversalAxis2 (join, M[ 4], M[ 5], M[ 6]);
        dJointSetUniversalAnchor(join, M[12], M[13], M[14]);
    }
}
*/
//-----------------------------------------------------------------------------
/*
void wrl::joint::step_init()
{
    // Joint parameter change may require reawakening of joined bodies.
    // TODO: do this only when necessary.

    if (body1) dBodyEnable(dJointGetBody(join, 0));
    if (body2) dBodyEnable(dJointGetBody(join, 1));
}

void wrl::hinge::step_init()
{
    for (param_map::iterator i = params.begin(); i != params.end(); ++i)
        dJointSetHingeParam(join, i->first, i->second->value());

    joint::step_init();
}

void wrl::hinge2::step_init()
{
    for (param_map::iterator i = params.begin(); i != params.end(); ++i)
        dJointSetHinge2Param(join, i->first, i->second->value());

    joint::step_init();
}

void wrl::slider::step_init()
{
    for (param_map::iterator i = params.begin(); i != params.end(); ++i)
        dJointSetSliderParam(join, i->first, i->second->value());

    joint::step_init();
}

void wrl::amotor::step_init()
{
    for (param_map::iterator i = params.begin(); i != params.end(); ++i)
        dJointSetAMotorParam(join, i->first, i->second->value());

    joint::step_init();
}

void wrl::universal::step_init()
{
    for (param_map::iterator i = params.begin(); i != params.end(); ++i)
        dJointSetUniversalParam(join, i->first, i->second->value());

    joint::step_init();
}
*/
//-----------------------------------------------------------------------------

void wrl::joint::draw_line() const
{
    float r = float(dGeomSphereGetRadius(geom));

    // Draw a wire sphere.

    glPushMatrix();
    {
        mult_M();

        glScalef(r, r, r);

        line->draw(DRAW_OPAQUE | DRAW_UNLIT);
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------

void wrl::joint::load(mxml_node_t *node)
{
    mxml_node_t *join;

    if ((join = mxmlFindElement(node, node, "join", 0, 0, MXML_DESCEND)))
        join_id = join->child->value.integer;

    atom::load(node);
}

mxml_node_t *wrl::joint::save(mxml_node_t *node)
{
    if (join_id) mxmlNewInteger(mxmlNewElement(node, "join"), join_id);
    return atom::save(node);
}

mxml_node_t *wrl::ball::save(mxml_node_t *parent)
{
    // Create a new ball element.

    mxml_node_t *node = mxmlNewElement(parent, "joint");

    mxmlElementSetAttr(node, "type", "ball");
    return joint::save(node);
}

mxml_node_t *wrl::hinge::save(mxml_node_t *parent)
{
    // Create a new hinge element.

    mxml_node_t *node = mxmlNewElement(parent, "joint");

    mxmlElementSetAttr(node, "type", "hinge");
    return joint::save(node);
}

mxml_node_t *wrl::hinge2::save(mxml_node_t *parent)
{
    // Create a new hinge2 element.

    mxml_node_t *node = mxmlNewElement(parent, "joint");

    mxmlElementSetAttr(node, "type", "hinge2");
    return joint::save(node);
}

mxml_node_t *wrl::slider::save(mxml_node_t *parent)
{
    // Create a new slider element.

    mxml_node_t *node = mxmlNewElement(parent, "joint");

    mxmlElementSetAttr(node, "type", "slider");
    return joint::save(node);
}

mxml_node_t *wrl::amotor::save(mxml_node_t *parent)
{
    // Create a new amotor element.

    mxml_node_t *node = mxmlNewElement(parent, "joint");

    mxmlElementSetAttr(node, "type", "amotor");
    return joint::save(node);
}

mxml_node_t *wrl::universal::save(mxml_node_t *parent)
{
    // Create a new universal element.

    mxml_node_t *node = mxmlNewElement(parent, "joint");

    mxmlElementSetAttr(node, "type", "universal");
    return joint::save(node);
}

//-----------------------------------------------------------------------------

