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

#ifndef SCM_FRAME_HPP
#define SCM_FRAME_HPP

#include <vector>

#include "scm-image.hpp"

//------------------------------------------------------------------------------

class scm_frame
{
public:

    scm_frame();

    void add_image(scm_image *p) { images.push_back(p); }

    void bind(int, GLuint) const;
    void free(int)         const;

    void set(GLuint, int, int, long long) const;
    void clr(GLuint, int)                 const;

    bool   page_status(long long) const;
    double page_r0    (long long) const;
    double page_r1    (long long) const;
    void   page_touch (long long, int);

    double get_r0() const;
    double get_r1() const;

private:

    std::vector<scm_image *> images;
};

typedef std::vector<scm_frame *>           scm_frame_v;
typedef std::vector<scm_frame *>::iterator scm_frame_i;

//------------------------------------------------------------------------------

#endif
