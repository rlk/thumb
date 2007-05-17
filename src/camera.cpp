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

#include "matrix.hpp"
#include "opengl.hpp"
#include "camera.hpp"
#include "view.hpp"

//-----------------------------------------------------------------------------

ent::camera::camera() : fov(60)
{
}

void ent::camera::edit_init()
{
    geom = dCreateSphere(space, 0.5f);
    dGeomSetData(geom, this);
//  get_default();
}

//-----------------------------------------------------------------------------

void ent::camera::draw()
{
    const GLfloat b[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    const GLfloat w[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glPushMatrix();
    {
        float ny = float(sin(RAD(fov) / 2));
        float nz = float(cos(RAD(fov) / 2));

        float vy = ny * 0.5f;
        float vz = nz * 0.5f;

        mult_M();

        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, b);

        glBegin(GL_TRIANGLES);
        {
            glNormal3f(-nz,   0, +ny);
            glVertex3f(  0,   0,   0);
            glVertex3f(-vy, +vy, -vz);
            glVertex3f(-vy, -vy, -vz);

            glNormal3f(  0, +nz, +ny);
            glVertex3f(  0,   0,   0);
            glVertex3f(+vy, +vy, -vz);
            glVertex3f(-vy, +vy, -vz);

            glNormal3f(+nz,   0, +ny);
            glVertex3f(  0,   0,   0);
            glVertex3f(+vy, -vy, -vz);
            glVertex3f(+vy, +vy, -vz);

            glNormal3f(  0, -nz, +ny);
            glVertex3f(  0,   0,   0);
            glVertex3f(-vy, -vy, -vz);
            glVertex3f(+vy, -vy, -vz);
        }
        glEnd();

        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, w);

        glBegin(GL_QUADS);
        {
            glNormal3f(  0,   0,   1);
            glTexCoord2i(0, 0);
            glVertex3f(-vy, -vy, -vz);
            glTexCoord2i(1, 0);
            glVertex3f(-vy, +vy, -vz);
            glTexCoord2i(1, 1);
            glVertex3f(+vy, +vy, -vz);
            glTexCoord2i(0, 1);
            glVertex3f(+vy, -vy, -vz);
        }
        glEnd();
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------

mxml_node_t *ent::camera::save(mxml_node_t *parent)
{
    // Create a new camera element.

    mxml_node_t *node = mxmlNewElement(parent, "geom");

    mxmlElementSetAttr(node, "class", "camera");
    return solid::save(node);
}

//-----------------------------------------------------------------------------
