//  Copyright (C) 2007 Robert Kooima
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

#ifndef OGL_BINDING_HPP
#define OGL_BINDING_HPP

#include <vector>
#include <string>
#include <map>

//-----------------------------------------------------------------------------

namespace ogl
{
    class frame;
    class program;
    class texture;
}

//-----------------------------------------------------------------------------

namespace ogl
{
    class binding
    {
        typedef std::map<GLenum, const ogl::texture *> unit_texture;
        typedef std::map<GLenum, const ogl::frame   *> unit_frame;

        static std::vector<ogl::frame *> shadow;
        
        static double split_depth[4];
        static double world_light[3];

        std::string name;

        const ogl::program *depth_program;
        const ogl::program *color_program;

        unit_texture depth_texture;
        unit_texture color_texture;
        unit_frame   light_texture;

        GLenum cull_mode;

        static bool init_shadow();

    public:

        static double split(int, double, double);
        static void   light(const double *);

        static int  shadow_count();

        static bool bind_shadow_frame(int);
        static bool bind_shadow_color(int, GLenum=GL_TEXTURE0);
        static bool bind_shadow_depth(int, GLenum=GL_TEXTURE0);

        static void draw_shadow_color(int);

        static void free_shadow_frame(int);
        static void free_shadow_color(int);
        static void free_shadow_depth(int);

        const std::string& get_name() const { return name; }

        binding(std::string);
       ~binding();

        bool depth_eq(const binding *) const;
        bool color_eq(const binding *) const;
        bool opaque() const;

        bool bind(bool) const;

        void init();
        void fini();
    };
}

//-----------------------------------------------------------------------------

#endif
