//  Copyright (C) 2013 Robert Kooima
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

#ifndef WRL_LIGHT_HPP
#define WRL_LIGHT_HPP

#include <ogl-opengl.hpp>
#include <wrl-solid.hpp>

//-----------------------------------------------------------------------------

namespace wrl
{
    class light : public sphere
    {
    public:

        light(std::string);

        virtual void play_init();
        virtual void play_fini();

        virtual double cache_light(int, const vec4&, const vec4&, int, int);
        virtual void   apply_light(int) const;
        virtual bool     has_light();

        virtual void load(app::node);

    protected:

        // OpenGL lighting parameter cache

        GLfloat     ambient[4];
        GLfloat     diffuse[4];
        GLfloat    position[4];
        GLfloat   direction[4];
        GLfloat attenuation[3];
        GLfloat    exponent;
        GLfloat      cutoff;

    };

    //-------------------------------------------------------------------------

    class d_light : public light
    {
    public:

        d_light();

        virtual d_light *clone() const { return new d_light(*this); }
        virtual int   priority() const { return -2; }

        virtual void save(app::node);
    };

    //-------------------------------------------------------------------------

    class s_light : public light
    {
    public:

        s_light();

        virtual s_light *clone() const { return new s_light(*this); }
        virtual int   priority() const { return -1; }

        virtual void save(app::node);
    };
}

//-----------------------------------------------------------------------------

#endif
