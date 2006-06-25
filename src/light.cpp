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
#include "main.hpp"

//-----------------------------------------------------------------------------

ent::light::light() :

    fov(60),
    lightmask("lightmask_basic.png"),
    shadowmap(GL_RGBA8, GL_DEPTH_COMPONENT24, 1024, 1024)
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
    float y = float(sin(RAD(fov) / 2));
    float z = float(cos(RAD(fov) / 2));

    glFrustum(-y, +y, -y, +y, z, z * 10);
}

//-----------------------------------------------------------------------------

void ent::light::lite_prep(int pass)
{
    // Render to the light's shadow map.

    shadowmap.push_frame(true);

    // Set up the GL state for shadow map rendering.

    glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);

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

    // Revert the GL state.

    glColorMask(1, 1, 1, 1);

    glPopAttrib();

    shadowmap.pop_frame();
    shadowmap.bind_depth(GL_TEXTURE1);
    lightmask.bind      (GL_TEXTURE2);

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

void ent::light::draw_dark()
{
    const GLfloat b[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    const GLfloat c[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    const GLfloat w[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glPushMatrix();
    {
        float ny = float(sin(RAD(fov) / 2));
        float nz = float(cos(RAD(fov) / 2));

        float vy = ny * 0.5f;
        float vz = nz * 0.5f;

        mult_M();

        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  c);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, b);

        glBegin(GL_TRIANGLES);
        {
            glColor3f(0.5, 0.5, 0.5);

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

        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  w);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, w);

        glBegin(GL_QUADS);
        {
            glNormal3f(  0,   0,   1);
            glVertex3f(-vy, -vy, -vz);
            glVertex3f(-vy, +vy, -vz);
            glVertex3f(+vy, +vy, -vz);
            glVertex3f(+vy, -vy, -vz);
        }
        glEnd();
    }
    glPopMatrix();
}

void ent::light::draw_lite()
{
    draw_dark();
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
