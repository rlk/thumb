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

#include "main.hpp"
#include "opengl.hpp"
#include "joint.hpp"

//-----------------------------------------------------------------------------

void ent::joint::geom_to_entity()
{
    if (geom) get_transform(current_M, geom);
}

void ent::joint::edit_init()
{
    geom = dCreateSphere(space, 0.25f);
    dGeomSetData(geom, this);
    get_default();
}

//-----------------------------------------------------------------------------

void ent::joint::play_init(dBodyID)
{
    edit_fini();
}

void ent::joint::play_fini()
{
    dJointDestroy(join);
    join = 0;
    edit_init();
}

//-----------------------------------------------------------------------------

ent::joint::joint(int f): entity(f), size(conf->get_f("joint_size"))
{
}

//-----------------------------------------------------------------------------

ent::ball::ball() : joint(data->get_obj("joint_ball.obj"))
{
}

ent::hinge::hinge() : joint(data->get_obj("joint_hinge.obj"))
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

ent::hinge2::hinge2() : joint(data->get_obj("joint_hinge2.obj"))
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

ent::slider::slider() : joint(data->get_obj("joint_slider.obj"))
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

ent::amotor::amotor() : joint(data->get_obj("joint_amotor.obj"))
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

ent::universal::universal() : joint(data->get_obj("joint_universal.obj"))
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

void ent::ball::play_init(dBodyID body)
{
    join = dJointCreateBall(world, 0);
    dJointAttach(join, body, 0);

    joint::play_init(body);
}

void ent::hinge::play_init(dBodyID body)
{
    join = dJointCreateHinge(world, 0);
    dJointAttach(join, body, 0);

    joint::play_init(body);
}

void ent::hinge2::play_init(dBodyID body)
{
    join = dJointCreateHinge2(world, 0);
    dJointAttach(join, body, 0);

    joint::play_init(body);
}

void ent::slider::play_init(dBodyID body)
{
    join = dJointCreateSlider(world, 0);
    dJointAttach(join, body, 0);

    joint::play_init(body);
}

void ent::amotor::play_init(dBodyID body)
{
    join = dJointCreateAMotor(world, 0);
    dJointAttach(join, body, 0);

    joint::play_init(body);
}

void ent::universal::play_init(dBodyID body)
{
    join = dJointCreateUniversal(world, 0);
    dJointAttach(join, body, 0);

    joint::play_init(body);
}

//-----------------------------------------------------------------------------

void ent::ball::play_join(dBodyID body1)
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

void ent::hinge::play_join(dBodyID body1)
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

void ent::hinge2::play_join(dBodyID body1)
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

void ent::slider::play_join(dBodyID body1)
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

void ent::amotor::play_join(dBodyID body1)
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

void ent::universal::play_join(dBodyID body1)
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

//-----------------------------------------------------------------------------

void ent::joint::step_prep()
{
    // Joint parameter change may require reawakening of joined bodies.

    if (body1) dBodyEnable(dJointGetBody(join, 0));
    if (body2) dBodyEnable(dJointGetBody(join, 1));
}

void ent::hinge::step_prep()
{
    std::map<int, param *>::iterator i;

    for (i = params.begin(); i != params.end(); ++i)
        dJointSetHingeParam(join, i->first, i->second->value());

    joint::step_prep();
}

void ent::hinge2::step_prep()
{
    std::map<int, param *>::iterator i;

    for (i = params.begin(); i != params.end(); ++i)
        dJointSetHinge2Param(join, i->first, i->second->value());

    joint::step_prep();
}

void ent::slider::step_prep()
{
    std::map<int, param *>::iterator i;

    for (i = params.begin(); i != params.end(); ++i)
        dJointSetSliderParam(join, i->first, i->second->value());

    joint::step_prep();
}

void ent::amotor::step_prep()
{
    std::map<int, param *>::iterator i;

    for (i = params.begin(); i != params.end(); ++i)
        dJointSetAMotorParam(join, i->first, i->second->value());

    joint::step_prep();
}

void ent::universal::step_prep()
{
    std::map<int, param *>::iterator i;

    for (i = params.begin(); i != params.end(); ++i)
        dJointSetUniversalParam(join, i->first, i->second->value());

    joint::step_prep();
}

//-----------------------------------------------------------------------------

int ent::joint::draw_prio(int flags) const
{
    if (flags & ent::flag_play)
        return 0;
    else
        return 1;
}

void ent::joint::draw_geom() const
{
    if (geom)
    {
        // Draw three rings.

        glScalef(size, size, size);

        ogl::draw_disc(1);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        ogl::draw_disc(1);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        ogl::draw_disc(1);
    }
}

//-----------------------------------------------------------------------------

mxml_node_t *ent::ball::save(mxml_node_t *parent)
{
    // Create a new ball element.

    mxml_node_t *node = mxmlNewElement(parent, "joint");

    mxmlElementSetAttr(node, "type", "ball");
    return joint::save(node);
}

mxml_node_t *ent::hinge::save(mxml_node_t *parent)
{
    // Create a new hinge element.

    mxml_node_t *node = mxmlNewElement(parent, "joint");

    mxmlElementSetAttr(node, "type", "hinge");
    return joint::save(node);
}

mxml_node_t *ent::hinge2::save(mxml_node_t *parent)
{
    // Create a new hinge2 element.

    mxml_node_t *node = mxmlNewElement(parent, "joint");

    mxmlElementSetAttr(node, "type", "hinge2");
    return joint::save(node);
}

mxml_node_t *ent::slider::save(mxml_node_t *parent)
{
    // Create a new slider element.

    mxml_node_t *node = mxmlNewElement(parent, "joint");

    mxmlElementSetAttr(node, "type", "slider");
    return joint::save(node);
}

mxml_node_t *ent::amotor::save(mxml_node_t *parent)
{
    // Create a new amotor element.

    mxml_node_t *node = mxmlNewElement(parent, "joint");

    mxmlElementSetAttr(node, "type", "amotor");
    return joint::save(node);
}

mxml_node_t *ent::universal::save(mxml_node_t *parent)
{
    // Create a new universal element.

    mxml_node_t *node = mxmlNewElement(parent, "joint");

    mxmlElementSetAttr(node, "type", "universal");
    return joint::save(node);
}

//-----------------------------------------------------------------------------

