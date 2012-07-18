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

    void bind(GLuint) const;
    void free()       const;

    void set_texture(GLuint, int, int, long long) const;
    void clr_texture(GLuint, int)                 const;
    void set_uniform(GLuint, int,      long long) const;

    bool   page_status(long long) const;
    double page_r0    (long long) const;
    double page_r1    (long long) const;
    void   page_touch (long long, int);

    double get_r0() const;
    double get_r1() const;

    void set_channel(int c) { channel = c; }

private:

    std::vector<scm_image *> images;

    int channel;
};

typedef std::vector<scm_frame *>           scm_frame_v;
typedef std::vector<scm_frame *>::iterator scm_frame_i;

//------------------------------------------------------------------------------

#endif
