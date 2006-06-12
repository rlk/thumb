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

void ent::joint::set_param(int key, std::string& expr)
{
    // Allow only valid parameters as initialized by the entity constructor.

    if (params.find(key) != params.end())
        params[key]->set(expr);
}

bool ent::joint::get_param(int key, std::string& expr)
{
    // Return false to indicate a non-valid parameter was requested.

    if (params.find(key) != params.end())
    {
        params[key]->get(expr);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------

ent::joint::joint(int f): entity(f), size(conf->get_f("joint_size"))
{
}

ent::joint::joint(const joint& that)
{
    std::map<int, joint_param *>::const_iterator i;

    // Copy this object.  This is C++ black magic.

    *this = that;

    // Flush and clone each parameter separately.

    params.clear();

    for (i = that.params.begin(); i != that.params.end(); ++i)
        params[i->first] = i->second->clone();
}

ent::joint::~joint()
{
    // Delete all parameters.

    std::map<int, joint_param *>::iterator i;
    
    for (i = params.begin(); i != params.end(); ++i)
        delete i->second;
}

//-----------------------------------------------------------------------------

ent::ball::ball() : joint(data->get_obj("joint_ball.obj"))
{
}

ent::hinge::hinge() : joint(data->get_obj("joint_hinge.obj"))
{
    params[velocity]  = new joint_param_velocity;
    params[force]     = new joint_param_force;
    params[cfm]       = new joint_param_cfm;
    params[bounce]    = new joint_param_bounce;
    params[lo_stop]   = new joint_param_lo_stop;
    params[hi_stop]   = new joint_param_hi_stop;
    params[stop_erp]  = new joint_param_stop_erp;
    params[stop_cfm]  = new joint_param_stop_cfm;
}

ent::hinge2::hinge2() : joint(data->get_obj("joint_hinge2.obj"))
{
    params[velocity]  = new joint_param_velocity;
    params[force]     = new joint_param_force;
    params[cfm]       = new joint_param_cfm;
    params[bounce]    = new joint_param_bounce;
    params[lo_stop]   = new joint_param_lo_stop;
    params[hi_stop]   = new joint_param_hi_stop;
    params[stop_erp]  = new joint_param_stop_erp;
    params[stop_cfm]  = new joint_param_stop_cfm;
    params[susp_erp]  = new joint_param_stop_erp;
    params[susp_cfm]  = new joint_param_stop_cfm;

    params[velocity2] = new joint_param_velocity(1);
    params[force2]    = new joint_param_force   (1);
    params[cfm2]      = new joint_param_cfm     (1);
    params[bounce2]   = new joint_param_bounce  (1);
    params[lo_stop2]  = new joint_param_lo_stop (1);
    params[hi_stop2]  = new joint_param_hi_stop (1);
    params[stop_erp2] = new joint_param_stop_erp(1);
    params[stop_cfm2] = new joint_param_stop_cfm(1);
}

ent::slider::slider() : joint(data->get_obj("joint_slider.obj"))
{
    params[velocity]  = new joint_param_velocity;
    params[force]     = new joint_param_force;
    params[cfm]       = new joint_param_cfm;
    params[bounce]    = new joint_param_bounce;
    params[lo_stop]   = new joint_param_lo_stop;
    params[hi_stop]   = new joint_param_hi_stop;
    params[stop_erp]  = new joint_param_stop_erp;
    params[stop_cfm]  = new joint_param_stop_cfm;
}

ent::amotor::amotor() : joint(data->get_obj("joint_amotor.obj"))
{
    params[velocity]  = new joint_param_velocity;
    params[force]     = new joint_param_force;
    params[cfm]       = new joint_param_cfm;
    params[bounce]    = new joint_param_bounce;
    params[lo_stop]   = new joint_param_lo_stop;
    params[hi_stop]   = new joint_param_hi_stop;
    params[stop_erp]  = new joint_param_stop_erp;
    params[stop_cfm]  = new joint_param_stop_cfm;

    params[velocity2] = new joint_param_velocity(1);
    params[force2]    = new joint_param_force   (1);
    params[cfm2]      = new joint_param_cfm     (1);
    params[bounce2]   = new joint_param_bounce  (1);
    params[lo_stop2]  = new joint_param_lo_stop (1);
    params[hi_stop2]  = new joint_param_hi_stop (1);
    params[stop_erp2] = new joint_param_stop_erp(1);
    params[stop_cfm2] = new joint_param_stop_cfm(1);

    params[velocity3] = new joint_param_velocity(2);
    params[force3]    = new joint_param_force   (2);
    params[cfm3]      = new joint_param_cfm     (2);
    params[bounce3]   = new joint_param_bounce  (2);
    params[lo_stop3]  = new joint_param_lo_stop (2);
    params[hi_stop3]  = new joint_param_hi_stop (2);
    params[stop_erp3] = new joint_param_stop_erp(2);
    params[stop_cfm3] = new joint_param_stop_cfm(2);
}

ent::universal::universal() : joint(data->get_obj("joint_universal.obj"))
{
    params[velocity]  = new joint_param_velocity;
    params[force]     = new joint_param_force;
    params[cfm]       = new joint_param_cfm;
    params[bounce]    = new joint_param_bounce;
    params[lo_stop]   = new joint_param_lo_stop;
    params[hi_stop]   = new joint_param_hi_stop;
    params[stop_erp]  = new joint_param_stop_erp;
    params[stop_cfm]  = new joint_param_stop_cfm;

    params[velocity2] = new joint_param_velocity(1);
    params[force2]    = new joint_param_force   (1);
    params[cfm2]      = new joint_param_cfm     (1);
    params[bounce2]   = new joint_param_bounce  (1);
    params[lo_stop2]  = new joint_param_lo_stop (1);
    params[hi_stop2]  = new joint_param_hi_stop (1);
    params[stop_erp2] = new joint_param_stop_erp(1);
    params[stop_cfm2] = new joint_param_stop_cfm(1);
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

void ent::hinge::use_param()
{
    std::map<int, joint_param *>::iterator i;

    for (i = params.begin(); i != params.end(); ++i)
        i->second->apply(dJointSetHingeParam, join);
}

void ent::hinge2::use_param()
{
    std::map<int, joint_param *>::iterator i;

    for (i = params.begin(); i != params.end(); ++i)
        i->second->apply(dJointSetHinge2Param, join);
}

void ent::slider::use_param()
{
    std::map<int, joint_param *>::iterator i;

    for (i = params.begin(); i != params.end(); ++i)
        i->second->apply(dJointSetSliderParam, join);
}

void ent::amotor::use_param()
{
    std::map<int, joint_param *>::iterator i;

    for (i = params.begin(); i != params.end(); ++i)
        i->second->apply(dJointSetAMotorParam, join);
}

void ent::universal::use_param()
{
    std::map<int, joint_param *>::iterator i;

    for (i = params.begin(); i != params.end(); ++i)
        i->second->apply(dJointSetUniversalParam, join);
}

//-----------------------------------------------------------------------------

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

void ent::joint::draw_fill() const
{
    // Draw the object associated with this joint.

    if (geom)
    {
        glPushMatrix();
        {
            mult_M();
            glScalef(size, size, size);

            obj_draw_file(file);
        }
        glPopMatrix();
    }
}

//-----------------------------------------------------------------------------

void ent::joint::load(mxml_node_t *node)
{
    std::map<int, joint_param *>::iterator i;

    // Initialize parameters using the given element.

    for (i = params.begin(); i != params.end(); ++i)
        i->second->load(node);

    entity::load(node);
}

mxml_node_t *ent::joint::save(mxml_node_t *node)
{
    std::map<int, joint_param *>::iterator i;

    // Add parameter attributes to this element.

    for (i = params.begin(); i != params.end(); ++i)
        i->second->save(node);

    return entity::save(node);
}

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

