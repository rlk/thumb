//  Copyright (C) 2008 Robert Kooima
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

#ifndef UNI_SLIDES_HPP
#define UNI_SLIDES_HPP

#include <list>

#include "app-serial.hpp"
#include "ogl-texture.hpp"
#include "app-frustum.hpp"

//-----------------------------------------------------------------------------

namespace uni
{
    // Single slide

    struct slide
    {
        const ogl::texture *texture;

        double M[16];

    public:

        slide(app::node);
       ~slide();

        void draw(const double *) const;
    };

    typedef std::list<slide *>                 slide_l;
    typedef std::list<slide *>::iterator       slide_i;
    typedef std::list<slide *>::const_iterator slide_c;

    // Set of slides

    class slides
    {
        double M[16];
        double I[16];

        slide_l all;

        app::file file;

        app::frustum_v frusta;

    public:

        slides(std::string);
       ~slides();

        void view(app::frustum_v&);
        void draw(int i) const;
    };
}

//-----------------------------------------------------------------------------

#endif
