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

private:

    bool mono;

    std::vector<scm_image *> images;
};

//------------------------------------------------------------------------------

#endif
