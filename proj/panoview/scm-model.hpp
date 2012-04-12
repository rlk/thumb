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

#ifndef SCM_MODEL_HPP
#define SCM_MODEL_HPP

#include <GL/glew.h>
#include <vector>

#include "scm-cache.hpp"

//------------------------------------------------------------------------------

class scm_model
{
public:

    scm_model(scm_cache&, const char *, const char *,
                          int, int, int);//, double, double);
   ~scm_model();

    int  tick() { return time++; }
    void prep(const double *, const double *, int, int);
    void draw(const double *, const double *, const int *, int,
                                              const int *, int,
                                              const int *, int);

    void dump_face(const double *, int, int,
                     double, double, double, double, int);
    double wiew_face(const double *, double, double, int, int,
                     double, double, double, double, int, int);
    void pwep_face(const double *, int, int, const int *, int,
                                             const int *, int, int);
    void pwep(const double *, const double *, int, int, const int *, int,
                                                        const int *, int);
    void dwaw(const double *, const double *, int, int, const int *, int,
                                                        const int *, int,
                                                        const int *, int);

    void set_fade(double k);
    void set_zoom(double x, double y, double z, double k)
    {
        zoomv[0] = x;
        zoomv[1] = y;
        zoomv[2] = z;
        zoomk    = k;
    }

    // double get_r0() const { return r0; }
    // double get_r1() const { return r1; }

private:

    GLuint stack[8];

    scm_cache& cache;

    int    time;
    int    size;
    int    depth;
    // double r0;
    // double r1;

    // Zooming state.

    double zoomv[3];
    double zoomk;

    void zoom(double *, const double *);

    // Data structures and algorithms for handling face adaptive subdivision.

    struct page;

    page *qv;
    int   qn;
    int   qm;

    // Data structures and algorithms for handling face adaptive subdivision.

    void   draw_face(const int *, int,
                     const int *, int,
                     const int *, int,
                     double, double, double, double, int, int);
    void   prep_face(const double *, int, int,
                     double, double, double, double, int, int, int);
    double view_face(const double *, double, int, int,
                     double, double, double, double, int);
    void   dump_face(const double *, double, int, int,
                     double, double, double, double, int);

    typedef unsigned char byte;

    enum
    {
        s_halt = 0,
        s_pass = 1,
        s_draw = 2
    };

    std::vector<byte> status;

    GLfloat age(int);

    // OpenGL programmable processing state

    static const int max_texture_image_units = 64;

    void init_program(const char *, const char *);
    void free_program();

    GLuint program;
    GLuint vert_shader;
    GLuint frag_shader;

    std::vector<GLuint> u_tex_a;
    std::vector<GLuint> u_tex_d;
    std::vector<GLuint> u_v_age;
    std::vector<GLuint> u_f_age;
    std::vector<GLuint> u_v_img;
    std::vector<GLuint> u_f_img;

    GLuint u_level;
    GLuint u_fader;
    GLuint u_zoomk;
    GLuint u_zoomv;
    GLuint u_faceM;

    // OpenGL geometry state.

    void init_arrays(int);
    void free_arrays();

    GLsizei count;
    GLuint  vertices;
    GLuint  elements[16];
};

//------------------------------------------------------------------------------

#endif
