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

//------------------------------------------------------------------------------

class sph_model
{
public:

    sph_model();
   ~sph_model();
   
    void draw(const double *, const double *, int w, int h);
    
private:

    struct face
    {
        double a[3];
        double b[3];
        double c[3];
        double d[3];
        
        void   divide(face&, face&, face&, face&);
        double measure(const double *, int, int);
    };

    void draw_face(face&, const double *, int, int, int);

    void init_program();
    void free_program();
    
    void init_arrays(int);
    void free_arrays();

    GLuint  program;
    GLuint  vert_shader;
    GLuint  frag_shader;
    
    GLuint  corner_a;
    GLuint  corner_b;
    GLuint  corner_c;
    GLuint  corner_d;
    
    GLuint  vertices;
    GLuint  elements;    
    GLsizei count;
};

//------------------------------------------------------------------------------

#endif