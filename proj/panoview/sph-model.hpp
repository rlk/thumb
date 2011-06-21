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
    GLuint program;
    GLuint vert_shader;
    GLuint frag_shader;
    
public:

    sph_model();
   ~sph_model();
   
    void draw(const double *, const double *, int w, int h);
    
private:
};

//------------------------------------------------------------------------------

#endif