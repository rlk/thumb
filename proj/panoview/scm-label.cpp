//  Copyright (C) 2005-2012 Robert Kooima
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

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <GL/glew.h>

#include "scm-label.hpp"
#include "math3d.h"
#include "type.h"
#include "glsl.h"

//------------------------------------------------------------------------------

static double clamp(double k, double a, double b)
{
    if      (k < a) return a;
    else if (k > b) return b;
    else            return k;
}

struct point
{
    float v[3];
    float t[2];
};

struct matrix
{
    double M[16];

    matrix()
    {
        midentity(M);
    }

    void rotatey(double a)
    {
        double A[16], B[16];
        mrotatey(A, a);
        mmultiply(B, M, A);
        mcpy(M, B);
    }

    void rotatex(double a)
    {
        double A[16], B[16];
        mrotatex(A, a);
        mmultiply(B, M, A);
        mcpy(M, B);
    }

    void rotatez(double a)
    {
        double A[16], B[16];
        mrotatez(A, a);
        mmultiply(B, M, A);
        mcpy(M, B);
    }

    void translate(double x, double y, double z)
    {
        double A[16], B[16], v[3] = { x, y, z };
        mtranslate(A, v);
        mmultiply(B, M, A);
        mcpy(M, B);
    }

    void scale(double k)
    {
        double A[16], B[16], v[3] = { k, k, k };
        mscale(A, v);
        mmultiply(B, M, A);
        mcpy(M, B);
    }
};

struct circle
{
    point p[4];

    circle(matrix& M)
    {
        const double s = 0.5;

        p[0].v[0] = M.M[0] * (-s) + M.M[4] * (-s) + M.M[12];
        p[0].v[1] = M.M[1] * (-s) + M.M[5] * (-s) + M.M[13];
        p[0].v[2] = M.M[2] * (-s) + M.M[6] * (-s) + M.M[14];
        p[0].t[0] = 0;
        p[0].t[1] = 0;

        p[1].v[0] = M.M[0] * ( s) + M.M[4] * (-s) + M.M[12];
        p[1].v[1] = M.M[1] * ( s) + M.M[5] * (-s) + M.M[13];
        p[1].v[2] = M.M[2] * ( s) + M.M[6] * (-s) + M.M[14];
        p[1].t[0] = 0;
        p[1].t[1] = 1;

        p[2].v[0] = M.M[0] * ( s) + M.M[4] * ( s) + M.M[12];
        p[2].v[1] = M.M[1] * ( s) + M.M[5] * ( s) + M.M[13];
        p[2].v[2] = M.M[2] * ( s) + M.M[6] * ( s) + M.M[14];
        p[2].t[0] = 1;
        p[2].t[1] = 1;

        p[3].v[0] = M.M[0] * (-s) + M.M[4] * ( s) + M.M[12];
        p[3].v[1] = M.M[1] * (-s) + M.M[5] * ( s) + M.M[13];
        p[3].v[2] = M.M[2] * (-s) + M.M[6] * ( s) + M.M[14];
        p[3].t[0] = 1;
        p[3].t[1] = 0;
    }
};

//------------------------------------------------------------------------------

// Parse the label definition file.

void scm_label::parse(const void *data_ptr, size_t data_len)
{
    const char *dat = (const char *) data_ptr;
    label L;
    int   n;

    while (sscanf(dat, "\"%63[^\"]\",%f,%f,%f,%f,%c\n%n",
                       L.str, &L.lat, &L.lon, &L.dia, &L.rad, &L.typ, &n) > 5)
    {
        L.dia /= 1737.4;

        labels.push_back(L);

        dat += n;
    }
}

//------------------------------------------------------------------------------

const char *vert_txt =                        \
    "void main() { "                          \
        "gl_TexCoord[0] = gl_MultiTexCoord0;" \
        "gl_FrontColor  = gl_Color;"          \
        "gl_Position    = ftransform();"      \
    "}";

const char *frag_txt =                        \
    "void main() { "                          \
        "float d = distance(gl_TexCoord[0].xy, vec2(0.5)); " \
        "vec2  f = fwidth(gl_TexCoord[0].xy); "\
        "float l = min(f.x, f.y); "\
        "float k = 1.0 - smoothstep(0.0, l, abs(0.5 - l - d));"\
        "gl_FragColor = vec4(gl_Color.rgb, gl_Color.a * k);"      \
    "}";


scm_label::scm_label(const void *data_ptr, size_t data_len,
                     const void *font_ptr, size_t font_len) : label_line(0)
{
    // Initialize the font.

    label_font = font_create(font_ptr, font_len, 64, 1.0);

    // Initialize the GLSL.

    vert = glsl_init_shader(GL_VERTEX_SHADER,   vert_txt);
    frag = glsl_init_shader(GL_FRAGMENT_SHADER, frag_txt);
    prog = glsl_init_program(vert, frag);

    // Parse the data file into labels, creating strings, matrices and circles.

    std::vector<char *> strv;
    std::vector<matrix> matv;
    std::vector<circle> cirv;

    parse(data_ptr, data_len);

    for (int i = 0; i < int(labels.size()); ++i)
    {
        int len = line_length(labels[i].str, label_font);

        double d = labels[i].dia;
        matrix M;

        M.rotatey  ( radians(labels[i].lon));
        M.rotatex  (-radians(labels[i].lat));
        M.translate(0.0, 0.0, sqrt(1.0 - d * d / 4.0));
        M.scale    (d);
        circle C(M);

        M.scale(0.001 * clamp(d, 0.0005, 0.5) / d);
        M.translate(-len / 2.0, 0.0, 0.0);

        strv.push_back(labels[i].str);
        cirv.push_back(C);
        matv.push_back(M);
    }

    // Typeset the labels.

    if (!strv.empty())
        label_line = line_layout(strv.size(), &strv.front(), NULL,
                                               matv.front().M, label_font);

    // Create a VBO for the circles.

    size_t sz = sizeof (point);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * sz * cirv.size(),
                                          &cirv.front(), GL_STATIC_DRAW);
}

scm_label::~scm_label()
{
    glDeleteBuffers(1, &vbo);

    line_delete(label_line);
    font_delete(label_font);

    glDeleteProgram(prog);
    glDeleteShader(frag);
    glDeleteShader(vert);
}

//------------------------------------------------------------------------------

void scm_label::draw()
{
    size_t sz = sizeof (point);

    glPushAttrib(GL_ENABLE_BIT);
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4f(1.0f, 0.5f, 0.0f, 0.5f);

        glUseProgram(prog);

        glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
        {
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glVertexPointer  (3, GL_FLOAT, sz, (GLvoid *) 0);
            glTexCoordPointer(2, GL_FLOAT, sz, (GLvoid *) 12);

            glDrawArrays(GL_QUADS, 0, labels.size() * 4);
        }
        glPopClientAttrib();

        glUseProgram(0);

        line_render(label_line);
    }
    glPopAttrib();
}

//------------------------------------------------------------------------------
