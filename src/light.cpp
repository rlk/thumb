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
#include "light.hpp"
#include "glob.hpp"
#include "view.hpp"

//-----------------------------------------------------------------------------

ent::light::light() :
    fov(60),
    lightmask(glob->load_texture("lightmask_basic.png"))
{
}

void ent::light::edit_init()
{
    geom = dCreateSphere(space, 0.5f);
    dGeomSetData(geom, this);
    get_default();
}

//-----------------------------------------------------------------------------

void ent::light::mult_P() const
{
    float y = float(sin(RAD(fov) / 2)) * 0.5f;
    float z = float(cos(RAD(fov) / 2)) * 0.5f;

    glFrustum(-y, +y, -y, +y, z, z * 100);
}

//-----------------------------------------------------------------------------

void ent::light::lite_prep(int pass)
{
    // Set up the GL state for shadow map rendering.

    glColorMask(0, 0, 0, 0);

    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);

    glPolygonOffset(4.0f, 4.0f);

    // Setup the transform of the light's view.

    glMatrixMode(GL_PROJECTION);
    {
        glLoadIdentity();
        mult_P();
    }
    glMatrixMode(GL_MODELVIEW);
    {
        glLoadIdentity();
        mult_V();
    }
}

void ent::light::lite_post(int pass)
{
    // Compose a matrix for later using in mapping this shadow map.

    glActiveTextureARB(GL_TEXTURE1);
    glMatrixMode(GL_TEXTURE);
    {
        glLoadIdentity();

        view->mult_S();
        mult_P();
        mult_V();
        view->mult_M();
    }
    glMatrixMode(GL_MODELVIEW);
    glActiveTextureARB(GL_TEXTURE0);

    // TODO: this violates the GL state discipline

    lightmask->bind(GL_TEXTURE2
);

    // Revert the GL state.

    glColorMask(1, 1, 1, 1);

    // Add the light source parameters to the GL state.

    float P[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    float D[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    view->apply();
    mult_M();

    glEnable (GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, P);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  D);
}

//-----------------------------------------------------------------------------

int ent::light::draw_prio(bool edit)
{
    if (edit)
        return 1;
    else
        return 0;
}

void ent::light::draw()
{
    const GLfloat b[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    const GLfloat w[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glPushAttrib(GL_ENABLE_BIT);
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

        glEnable(GL_TEXTURE_2D);

        lightmask->bind(GL_TEXTURE0);
        {
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
        lightmask->free();
    }
    glPopMatrix();
    glPopAttrib();
}

//-----------------------------------------------------------------------------

mxml_node_t *ent::light::save(mxml_node_t *parent)
{
    // Create a new light element.

    mxml_node_t *node = mxmlNewElement(parent, "geom");

    mxmlElementSetAttr(node, "class", "light");
    return solid::save(node);
}

//-----------------------------------------------------------------------------
