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

#ifndef SCM_TASK_HPP
#define SCM_TASK_HPP

#include <GL/glew.h>
#include <tiffio.h>

#include "scm-item.hpp"

//------------------------------------------------------------------------------

struct scm_task : public scm_item
{
    scm_task()                   : scm_item(    ), u(0), d(false), p(0) { }
    scm_task(int f, long long i) : scm_item(i, f), u(0), d(false), p(0) { }
    scm_task(int f, long long i, uint64 o, GLuint u, GLsizei s);

    uint64 o;
    GLuint u;
    bool   d;
    void  *p;

    void make_texture(GLuint, uint32, uint32, uint16, uint16, uint16);
    void load_texture(TIFF *, uint32, uint32, uint16, uint16, uint16);
    void dump_texture();
};

//------------------------------------------------------------------------------

#endif
