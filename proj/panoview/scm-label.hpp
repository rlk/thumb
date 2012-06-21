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

#ifndef SCM_LABEL_HPP
#define SCM_LABEL_HPP

#include <string>
#include <vector>

#include <GL/glew.h>

#include "type.h"
#include "glsl.h"

//-----------------------------------------------------------------------------

class scm_label
{
public:

    scm_label(const void *, size_t,
              const void *, size_t);
   ~scm_label();

    void draw();

private:

    static const int strmax = 64;

    struct label
    {
        char  str[strmax];
        float lat;
        float lon;
        float dia;
    };

    void parse(const void *, size_t);
    void apply(label *);

    font *label_font;
    line *label_line;
    glsl *label_glsl;

    GLuint vbo;
    GLuint vert;
    GLuint frag;
    GLuint prog;

    std::vector<label> labels;
};

//-----------------------------------------------------------------------------

#endif
