//  Copyright (C) 2009 Robert Kooima
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

#ifndef OGL_TERRAIN_HPP
#define OGL_TERRAIN_HPP

#include <string>

#include "ogl-opengl.hpp"
#include "ogl-range.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    struct head_s;
    struct page_s;
    struct vert_s;

    class terrain
    {
        std::string name;
        std::string data;

        const uint8_t *buff;

        const head_s *head;
        const page_s *page;
        const vert_s *vert;

        double s;
        double h;
        double M[16];
        double I[16];

    public:

        const std::string& get_name() const { return name; }

        terrain(std::string);
       ~terrain();

        ogl::range view(                const double *, int) const;
        void       draw(const double *, const double *, int) const;

        void init();
        void fini();
    };
}

//-----------------------------------------------------------------------------

#endif
