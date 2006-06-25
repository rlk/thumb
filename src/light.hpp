//  Copyright (C) 2005 Robert Kooima
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

#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "opengl.hpp"
#include "image.hpp"
#include "solid.hpp"

//-----------------------------------------------------------------------------

namespace ent
{
    class light : public sphere
    {
        float fov;

        ogl::image_file lightmask;
        ogl::fbo        shadowmap;

        virtual void mult_P() const;

    public:

        light();

        void edit_init();

        int  lite_prio(bool) { return 1; }
        void lite_prep(int);
        void lite_post(int);

        int  draw_prio(bool) { return 1; }
        void draw_dark();
        void draw_lite();

        virtual mxml_node_t *save(mxml_node_t *);
    };
}

//-----------------------------------------------------------------------------

#endif
