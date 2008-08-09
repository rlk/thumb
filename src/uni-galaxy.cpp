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

#include "matrix.hpp"
#include "uni-galaxy.hpp"
#include "app-data.hpp"
#include "app-user.hpp"

#include "util.hpp"

//=============================================================================

const void *uni::star::read(const void *buf)
{
    const GLubyte *b = (const GLubyte *) buf;
    const GLfloat *f = (const GLfloat *) buf;

    col[0] = b[0];
    col[1] = b[1];
    col[2] = b[2];
    col[3] = b[3];

    pos[0] = ntohf(f[1]);
    pos[1] = ntohf(f[2]);
    pos[2] = ntohf(f[3]);
    mag    = ntohf(f[4]);

    return (const void *)(f + 5);
}

//=============================================================================

const void *uni::node::read(const void *buf)
{
    const float *f = (const float *) buf;
    const int   *i = (const int   *) buf;

    bound[0] = (double) ntohf(f[1]);
    bound[1] = (double) ntohf(f[2]);
    bound[2] = (double) ntohf(f[3]);
    bound[3] = (double) ntohf(f[4]);
    bound[4] = (double) ntohf(f[5]);
    bound[5] = (double) ntohf(f[6]);

    star0 = ntohi(i[7]);
    starc = ntohi(i[8]);
    nodeL = ntohi(i[9]);
    nodeR = ntohi(i[10]);

    return (const void *)(i + 11);
}

void uni::node::draw() const
{
    glDrawArrays(GL_POINTS, star0, starc);
}

//=============================================================================

uni::galaxy::galaxy(const char *filename) : magnitude(75.0f)
{
    int N_num = 0;
    int S_num = 0;

    // Load the galaxy data file.

    size_t      len;
    const void *buf = (const void *) ::data->load(filename, &len);

    if (buf)
    {
        // Parse the number of nodes and stars.

        N_num = ntohi(((int *) buf)[0]);
        S_num = ntohi(((int *) buf)[1]);

        buf = (int *) buf + 2;

        // Preallocate node and star storage.

        N.reserve(N_num); N.resize(N_num);
        S.reserve(S_num); S.resize(S_num);

        // Parse each node and each star in turn.

        for (node_i i = N.begin(); i != N.end(); ++i) buf = i->read(buf);
        for (star_i i = S.begin(); i != S.end(); ++i) buf = i->read(buf);
    }

    ::data->free(filename);

    // Initialize the vertex buffer object.

    glGenBuffersARB(1, &buffer);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, 20 * S_num,
                    &S.front(), GL_STATIC_DRAW_ARB);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    // Initialize the program.

    starprog = ::glob->load_program("glsl/star.vert", "glsl/star.frag");
}

uni::galaxy::~galaxy()
{
    ::glob->free_program(starprog);

    if (buffer) glDeleteBuffersARB(1, &buffer);
}

//-----------------------------------------------------------------------------

void uni::galaxy::view(app::frustum_v& frusta)
{
    load_mat(M, ::user->get_I());
    load_mat(I, ::user->get_M());

    while (!this->frusta.empty())
    {
        delete this->frusta.back();
        this->frusta.pop_back();
    }

    // Apply the transform to the frusta.

    for (int i = 0; i < int(frusta.size()); ++i)
    {
        app::frustum *frust = new app::frustum(*(frusta[i]));

        frust->calc_view_planes(I, M);

        this->frusta.push_back(frust);
    }
}

void uni::galaxy::draw(int i) const
{
    // Bind the VBO.  Enable and attach the vertex arrays.

    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableVertexAttribArrayARB(6);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer);

    glColorPointer          (4,    GL_UNSIGNED_BYTE, 20, (GLvoid *)  0);
    glVertexPointer         (3,    GL_FLOAT,         20, (GLvoid *)  4);
    glVertexAttribPointerARB(6, 1, GL_FLOAT,      0, 20, (GLvoid *) 16);

    // Apply the projection.

    frusta[i]->calc_projection(1.0, 1e16);
    frusta[i]->draw();

    // Draw, beginning with the root node of the hierarchy.

    glPushAttrib(GL_ENABLE_BIT);
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        glEnable(GL_POINT_SPRITE_ARB);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        glPushMatrix();
        {
            const double k = 1e11;

            glLoadMatrixd(M);
            glScalef(k, k, k);

            starprog->bind();
            {
                const double *p = frusta[i]->get_view_pos();

                starprog->uniform("Multiplier", magnitude);
                starprog->uniform("Position", p[0] / k, p[1] / k, p[2] / k);
                N[0].draw();
            }
            starprog->free();
        }
        glPopMatrix();
    }
    glPopAttrib();

    // Disable the vertex arrays and unbind the VBO.

    glDisableVertexAttribArrayARB(6);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    // Unbind the VBO and EBO.

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

//=============================================================================
