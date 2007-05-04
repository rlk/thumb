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
#include "texture.hpp"
#include "solid.hpp"

//-----------------------------------------------------------------------------

namespace ent
{
    class light : public sphere
    {
    protected:

        float fov;

        const ogl::texture *lightmask;

        virtual void mult_P() const;

    public:

        virtual light *clone() const { return new light(*this); }
        light();

        void edit_init();

        int  lite_prio(bool) { return 1; }
        void lite_prep(int);
        void lite_post(int);

        int  draw_prio(bool);
        void draw();

        virtual mxml_node_t *save(mxml_node_t *);
    };
}

//-----------------------------------------------------------------------------

#endif
