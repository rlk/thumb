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

#include <etc-ode.hpp>
#include <ogl-pool.hpp>
#include <wrl-joint.hpp>

//-----------------------------------------------------------------------------

wrl::joint::joint(app::node node, std::string fill, std::string line) :
    atom(node, fill, line), join_id(0)
{
    if (node)
    {
        if (app::node n = node.find("join"))
            join_id = n.get_i();
    }

    line_scale = vec3(0.25, 0.25, 0.25);

    edit_geom = dCreateSphere(0, dReal(0.25));

    dGeomSetData(edit_geom, this);
    bGeomSetTransform(edit_geom, current_M);
}

//-----------------------------------------------------------------------------

wrl::ball::ball(app::node node) :
    joint(node, "joint/joint_ball.obj", "wire/wire_sphere.obj")
{
}

wrl::hinge::hinge(app::node node) :
    joint(node, "joint/joint_hinge.obj", "wire/wire_sphere.obj")
{
    params[dParamVel]      = new param("dParamVel",      "0.0");
    params[dParamFMax]     = new param("dParamFMax",     "0.0");
    params[dParamCFM]      = new param("dParamCFM",      "0.0");
    params[dParamBounce]   = new param("dParamBounce",   "0.5");
    params[dParamLoStop]   = new param("dParamLoStop",  "-inf");
    params[dParamHiStop]   = new param("dParamHiStop",   "inf");
    params[dParamStopERP]  = new param("dParamStopERP",  "0.2");
    params[dParamStopCFM]  = new param("dParamStopCFM",  "0.0");

    load_params(node);
}

wrl::hinge2::hinge2(app::node node) :
    joint(node, "joint/joint_hinge2.obj", "wire/wire_sphere.obj")
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

    load_params(node);
}

wrl::slider::slider(app::node node) :
    joint(node, "joint/joint_slider.obj", "wire/wire_sphere.obj")
{
    params[dParamVel]      = new param("dParamVel",      "0.0");
    params[dParamFMax]     = new param("dParamFMax",     "0.0");
    params[dParamCFM]      = new param("dParamCFM",      "0.0");
    params[dParamBounce]   = new param("dParamBounce",   "0.5");
    params[dParamLoStop]   = new param("dParamLoStop",  "-inf");
    params[dParamHiStop]   = new param("dParamHiStop",   "inf");
    params[dParamStopERP]  = new param("dParamStopERP",  "0.2");
    params[dParamStopCFM]  = new param("dParamStopCFM",  "0.0");

    load_params(node);
}

wrl::amotor::amotor(app::node node) :
    joint(node, "joint/joint_amotor.obj", "wire/wire_sphere.obj")
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

    load_params(node);
}

wrl::universal::universal(app::node node) :
    joint(node, "joint/joint_universal.obj", "wire/wire_sphere.obj")
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

    load_params(node);
}

//-----------------------------------------------------------------------------

dJointID wrl::ball::get_join(dWorldID world)
{
    return (play_join = dJointCreateBall(world, 0));
}

dJointID wrl::hinge::get_join(dWorldID world)
{
    return (play_join = dJointCreateHinge(world, 0));
}

dJointID wrl::hinge2::get_join(dWorldID world)
{
    return (play_join = dJointCreateHinge2(world, 0));
}

dJointID wrl::slider::get_join(dWorldID world)
{
    return (play_join = dJointCreateSlider(world, 0));
}

dJointID wrl::amotor::get_join(dWorldID world)
{
    return (play_join = dJointCreateAMotor(world, 0));
}

dJointID wrl::universal::get_join(dWorldID world)
{
    return (play_join = dJointCreateUniversal(world, 0));
}

//-----------------------------------------------------------------------------

void wrl::joint::play_init()
{
    if (fill) fill->set_mode(false);
}

void wrl::joint::play_fini()
{
    if (fill) fill->set_mode(true);
}

void wrl::ball::play_init()
{
    // Set ball joint geometry parameters.

    dJointSetBallAnchor(play_join, dReal(current_M[0][3]),
                                   dReal(current_M[1][3]),
                                   dReal(current_M[2][3]));
    wrl::joint::play_init();
}

void wrl::hinge::play_init()
{
    // Set hinge geometry parameters.

    dJointSetHingeAxis  (play_join, dReal(current_M[0][0]),
                                    dReal(current_M[1][0]),
                                    dReal(current_M[2][0]));
    dJointSetHingeAnchor(play_join, dReal(current_M[0][3]),
                                    dReal(current_M[1][3]),
                                    dReal(current_M[2][3]));
    wrl::joint::play_init();
}

void wrl::hinge2::play_init()
{
    // Set hinge2 geometry parameters.

    dJointSetHinge2Axis2 (play_join, dReal(current_M[0][0]),
                                     dReal(current_M[1][0]),
                                     dReal(current_M[2][0]));
    dJointSetHinge2Axis1 (play_join, dReal(current_M[0][1]),
                                     dReal(current_M[1][1]),
                                     dReal(current_M[2][1]));
    dJointSetHinge2Anchor(play_join, dReal(current_M[0][3]),
                                     dReal(current_M[1][3]),
                                     dReal(current_M[2][3]));
    wrl::joint::play_init();
}

void wrl::slider::play_init()
{
    // Set slider geometry parameters.

    dJointSetSliderAxis(play_join, dReal(current_M[0][3]),
                                   dReal(current_M[1][3]),
                                   dReal(current_M[2][3]));
    wrl::joint::play_init();
}

void wrl::amotor::play_init()
{
    int a = dJointGetBody(play_join, 0) ? 1 : 0;
    int b = dJointGetBody(play_join, 1) ? 2 : 0;

    // Set angular motor geometry parameters.

    if (a != b)
    {
        dJointSetAMotorMode(play_join, dAMotorEuler);
        dJointSetAMotorAxis(play_join, 0, a, dReal(current_M[0][0]),
                                             dReal(current_M[1][0]),
                                             dReal(current_M[2][0]));
        dJointSetAMotorAxis(play_join, 2, b, dReal(current_M[0][3]),
                                             dReal(current_M[1][3]),
                                             dReal(current_M[2][3]));
    }
    wrl::joint::play_init();
}

void wrl::universal::play_init()
{
    // Set universal joint geometry parameters.

    dJointSetUniversalAxis1 (play_join, dReal(current_M[0][0]),
                                        dReal(current_M[1][0]),
                                        dReal(current_M[2][0]));
    dJointSetUniversalAxis2 (play_join, dReal(current_M[0][1]),
                                        dReal(current_M[1][1]),
                                        dReal(current_M[2][1]));
    dJointSetUniversalAnchor(play_join, dReal(current_M[0][3]),
                                        dReal(current_M[1][3]),
                                        dReal(current_M[2][3]));
    wrl::joint::play_init();
}

//-----------------------------------------------------------------------------

void wrl::joint::step_init()
{
    // Joint parameter change may require reawakening of joined bodies.

    dBodyID body0 = dJointGetBody(play_join, 0);
    dBodyID body1 = dJointGetBody(play_join, 1);

    if (body0) dBodyEnable(body0);
    if (body1) dBodyEnable(body1);
}

void wrl::hinge::step_init()
{
    for (param_map::iterator i = params.begin(); i != params.end(); ++i)
        dJointSetHingeParam(play_join, i->first, i->second->value());

    joint::step_init();
}

void wrl::hinge2::step_init()
{
    for (param_map::iterator i = params.begin(); i != params.end(); ++i)
        dJointSetHinge2Param(play_join, i->first, i->second->value());

    joint::step_init();
}

void wrl::slider::step_init()
{
    for (param_map::iterator i = params.begin(); i != params.end(); ++i)
        dJointSetSliderParam(play_join, i->first, i->second->value());

    joint::step_init();
}

void wrl::amotor::step_init()
{
    for (param_map::iterator i = params.begin(); i != params.end(); ++i)
        dJointSetAMotorParam(play_join, i->first, i->second->value());

    joint::step_init();
}

void wrl::universal::step_init()
{
    for (param_map::iterator i = params.begin(); i != params.end(); ++i)
        dJointSetUniversalParam(play_join, i->first, i->second->value());

    joint::step_init();
}

//-----------------------------------------------------------------------------

void wrl::joint::save(app::node node)
{
    if (join_id)
    {
        app::node n("join");
        n.set_i(join_id);
        n.insert(node);
    }
    atom::save(node);
}

void wrl::ball::save(app::node node)
{
    // Create a new ball element.

    app::node n("joint");

    n.set_s("type", "ball");
    n.insert(node);
    joint::save(n);
}

void wrl::hinge::save(app::node node)
{
    // Create a new hinge element.

    app::node n("joint");

    n.set_s("type", "hinge");
    n.insert(node);
    joint::save(n);
}

void wrl::hinge2::save(app::node node)
{
    // Create a new hinge2 element.

    app::node n("joint");

    n.set_s("type", "hinge2");
    n.insert(node);
    joint::save(n);
}

void wrl::slider::save(app::node node)
{
    // Create a new slider element.

    app::node n("joint");

    n.set_s("type", "slider");
    n.insert(node);
    joint::save(n);
}

void wrl::amotor::save(app::node node)
{
    // Create a new amotor element.

    app::node n("joint");

    n.set_s("type", "amotor");
    n.insert(node);
    joint::save(n);
}

void wrl::universal::save(app::node node)
{
    // Create a new universal element.

    app::node n("joint");

    n.set_s("type", "universal");
    n.insert(node);
    joint::save(n);
}

//-----------------------------------------------------------------------------

