#include <ode/ode.h>

#include "opengl.hpp"
#include "matrix.hpp"
#include "solid.hpp"
#include "main.hpp"

//-----------------------------------------------------------------------------

ent::solid::solid(int f) : entity(f), tran(0)
{
    params[friction] = new solid_param_friction;
    params[density]  = new solid_param_density;
    params[bounce]   = new solid_param_bounce;
    params[soft_erp] = new solid_param_soft_erp;
    params[soft_cfm] = new solid_param_soft_cfm;
}

ent::solid::solid(const solid& that)
{
    std::map<int, solid_param *>::const_iterator i;

    // Copy this object.  This is C++ black magic.

    *this = that;

    // Flush and clone each parameter separately.

    params.clear();

    for (i = that.params.begin(); i != that.params.end(); ++i)
        params[i->first] = i->second->clone();
}

ent::solid::~solid()
{
    // Delete all parameters.

    std::map<int, solid_param *>::iterator i;
    
    for (i = params.begin(); i != params.end(); ++i)
        delete i->second;
}

//-----------------------------------------------------------------------------

void ent::solid::geom_to_entity()
{
    if (geom && tran)
    {
        float tran_M[16];
        float geom_M[16];

        get_transform(tran_M, tran);
        get_transform(geom_M, geom);

        mult_mat_mat(current_M, tran_M, geom_M);
    }
}

//-----------------------------------------------------------------------------

void ent::solid::use_param(dSurfaceParameters& surface)
{
    std::map<int, solid_param *>::iterator i;

    for (i = params.begin(); i != params.end(); ++i)
        i->second->apply(surface);
}

void ent::solid::set_param(int key, std::string& expr)
{
    // Allow only valid parameters as initialized by the entity constructor.

    if (params.find(key) != params.end())
        params[key]->set(expr);
}

bool ent::solid::get_param(int key, std::string& expr)
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

void ent::box::edit_init()
{
    float bound[6];

    if (file >= 0)
    {
        obj_bound_file(file, bound);

        geom = dCreateBox(space, bound[3] - bound[0],
                                 bound[4] - bound[1],
                                 bound[5] - bound[2]);
        dGeomSetData(geom, this);
        entity_to_geom();
    }
}

void ent::sphere::edit_init()
{
    float bound[6];

    if (file >= 0)
    {
        obj_bound_file(file, bound);

        geom = dCreateSphere(space, (bound[3] - bound[0]) / 2.0f);

        dGeomSetData(geom, this);
        entity_to_geom();
    }
}

void ent::capsule::edit_init()
{
    float bound[6];

    if (file >= 0)
    {
        obj_bound_file(file, bound);

        float length = (bound[5] - bound[2]);
        float radius = (bound[3] - bound[0]) / 2.0f;

        geom = dCreateCCylinder(space, radius, length - radius - radius);

        dGeomSetData(geom, this);
        entity_to_geom();
    }
}

void ent::plane::edit_init()
{
    geom = dCreatePlane(space, 0.0f, 1.0f, 0.0f, 0.0f);

    dGeomSetData(geom, this);
}

//-----------------------------------------------------------------------------

void ent::solid::play_init(dBodyID body)
{
    if (body)
    {
        dMass total;

        // Transform the mass to align with the entity.

        dMatrix3 R;

        R[ 0] = (dReal) current_M[ 0];
        R[ 1] = (dReal) current_M[ 4];
        R[ 2] = (dReal) current_M[ 8];
        R[ 3] = (dReal) 0.0f;

        R[ 4] = (dReal) current_M[ 1];
        R[ 5] = (dReal) current_M[ 5];
        R[ 6] = (dReal) current_M[ 9];
        R[ 7] = (dReal) 0.0f;

        R[ 8] = (dReal) current_M[ 2];
        R[ 9] = (dReal) current_M[ 6];
        R[10] = (dReal) current_M[10];
        R[11] = (dReal) 0.0f;
    
        dMassRotate   (&mass, R);
        dMassTranslate(&mass, current_M[12],
                              current_M[13],
                              current_M[14]);

        // Accumulate the geom's mass with the body's existing mass.

        dBodyGetMass(body, &total);
        dMassAdd(&total, &mass);
        dBodySetMass(body, &total);
    }
}

void ent::solid::play_tran(dBodyID body)
{
    if (body)
    {
        const dReal *p = dBodyGetPosition(body);

        // Create a transform geom and reposition the enclosed geom.

        tran = dCreateGeomTransform(space);
        dGeomTransformSetGeom(tran, geom);
        dGeomTransformSetInfo(tran, 0);

        dGeomSetBody(tran, body);
        dGeomSetPosition(geom, current_M[12] - p[0],
                               current_M[13] - p[1],
                               current_M[14] - p[2]);

        dSpaceRemove(space, geom);
    }
}

void ent::solid::play_fini()
{
    if (tran)
    {
        // Delete the transform geom and revert the transform.

        dSpaceAdd(space, geom);
        dGeomDestroy(tran);
        get_default();

        tran = 0;
    }
}

//-----------------------------------------------------------------------------

void ent::box::play_init(dBodyID body)
{
    if (body)
    {
        dVector3 v;

        // Compute the mass of this box.

        dGeomBoxGetLengths(geom, v);
        dMassSetBox(&mass, params[density]->value(), v[0], v[1], v[2]);

        solid::play_init(body);
    }
}

void ent::sphere::play_init(dBodyID body)
{
    if (body)
    {
        dReal r;

        // Compute the mass of this sphere.

        r = dGeomSphereGetRadius(geom);
        dMassSetSphere(&mass, params[density]->value(), r);

        solid::play_init(body);
    }
}

void ent::capsule::play_init(dBodyID body)
{
    if (body)
    {
        dReal r;
        dReal l;

        // Compute the mass of this sphere.

        dGeomCCylinderGetParams(geom, &r, &l);
        dMassSetCappedCylinder(&mass, params[density]->value(), 3, r, l);

        solid::play_init(body);
    }
}

//-----------------------------------------------------------------------------

void ent::box::draw_geom() const
{
    if (geom)
    {
        dVector3 len;

        dGeomBoxGetLengths(geom, len);

        // Draw a wire box.

        glScalef(float(len[0] / 2.0f),
                 float(len[1] / 2.0f),
                 float(len[2] / 2.0f));

        ogl::draw_cube();
    }
}

void ent::sphere::draw_geom() const
{
    if (geom)
    {
        float rad = float(dGeomSphereGetRadius(geom));

        // Draw three rings.

        glScalef(rad, rad, rad);

        ogl::draw_disc(1);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        ogl::draw_disc(1);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        ogl::draw_disc(1);
    }
}

void ent::capsule::draw_geom() const
{
}

//-----------------------------------------------------------------------------

void ent::solid::load(mxml_node_t *node)
{
    std::map<int, solid_param *>::iterator i;

    mxml_node_t *name;

    // Load the OBJ file.

    if ((name = mxmlFindElement(node, node, "file", 0, 0, MXML_DESCEND)))
        file = data->get_obj(std::string(name->child->value.text.string));

    // Initialize parameters using the given element.

    for (i = params.begin(); i != params.end(); ++i)
        i->second->load(node);

    entity::load(node);
}

mxml_node_t *ent::solid::save(mxml_node_t *node)
{
    std::map<int, solid_param *>::iterator i;

    // Add the OBJ file reference.

    if (file >= 0)
    {
        std::string name = data->get_relative(obj_get_file_name(file));
        mxmlNewText(mxmlNewElement(node, "file"), 0, name.c_str());
    }

    // Add parameters elements.

    for (i = params.begin(); i != params.end(); ++i)
        i->second->save(node);

    return entity::save(node);
}

mxml_node_t *ent::box::save(mxml_node_t *parent)
{
    // Create a new box element.

    mxml_node_t *node = mxmlNewElement(parent, "geom");

    mxmlElementSetAttr(node, "class", "box");
    return solid::save(node);
}

mxml_node_t *ent::sphere::save(mxml_node_t *parent)
{
    // Create a new sphere element.

    mxml_node_t *node = mxmlNewElement(parent, "geom");

    mxmlElementSetAttr(node, "class", "sphere");
    return solid::save(node);
}

mxml_node_t *ent::capsule::save(mxml_node_t *parent)
{
    // Create a new capsule element.

    mxml_node_t *node = mxmlNewElement(parent, "geom");

    mxmlElementSetAttr(node, "class", "capsule");
    return solid::save(node);
}

//-----------------------------------------------------------------------------

