#include "main.hpp"
#include "opengl.hpp"
#include "earth.hpp"

//-----------------------------------------------------------------------------

ent::earth::earth(float k) : free(data->get_obj("earth.obj")), dist(k)
{
}

void ent::earth::draw_fill() const
{
    glPushMatrix();
    {
        glScalef(dist, dist, dist);
        obj_draw_file(file);
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------
