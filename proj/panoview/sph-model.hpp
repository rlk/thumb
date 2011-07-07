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

#ifndef SPH_MODEL_HPP
#define SPH_MODEL_HPP

#include <GL/glew.h>
#include <vector>

#include "sph-cache.hpp"

//------------------------------------------------------------------------------

class sph_model
{
public:

    sph_model(sph_cache&, int, int, int);
   ~sph_model();
   
    void prep(const double *, const double *, int, int);
    void draw(const double *, const double *, int);
    
    void set_zoom(double x, double y, double z, double k)
    {
        zoomv[0] = x;
        zoomv[1] = y;
        zoomv[2] = z;
        zoomk    = k;
    }
    
private:

    sph_cache& cache;

    int depth;
    int size;
    int time;

    // Zooming state.

    double zoomv[3];
    double zoomk;
    
    void zoom(double *, const double *);

    // Data structures and algorithms for handling face adaptive subdivision.

    void   draw_face(int, int,
                     double, double, double, double, int);
    void   prep_face(int, int, const double *, int, int,
                     double, double, double, double, int);
    double view_face(     int, const double *, int, int,
                     double, double, double, double);

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

    void init_program();
    void free_program();
    
    GLuint program;
    GLuint vert_shader;
    GLuint frag_shader;
    
    GLuint pos_a;
    GLuint pos_b;
    GLuint pos_c;
    GLuint pos_d;
    GLuint tex_a[8];
    GLuint tex_d[8];
    GLuint tex_t[8];
    GLuint level;
    
    // OpenGL geometry state.
    
    void init_arrays(int);
    void free_arrays();

    GLsizei count;
    GLuint  vertices;
    GLuint  elements[16];
};

//------------------------------------------------------------------------------

#endif
