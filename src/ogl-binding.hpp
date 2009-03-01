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

#include "app-serial.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class pool;
    class node;
    class frame;
    class program;
    class process;
    class texture;
}

//-----------------------------------------------------------------------------

namespace ogl
{
    class binding
    {
        typedef std::map<GLenum, const ogl::texture *> unit_texture;
        typedef std::map<GLenum, const ogl::process *> unit_process;
        typedef std::map<GLenum,       ogl::frame   *> unit_frame;

        // Shared binding attributes.

        static std::vector<ogl::frame *> shadow;

        static bool init_shadow();

        // Local binding attributes.

        std::string name;

        const ogl::program *depth_program;  // Depth mode shader program
        unit_texture        depth_texture;  // Depth mode texture bindings
        unit_process        depth_process;  // Depth mode process bindings
        unit_frame          depth_c_frame;  // Depth mode color frame bindings
        unit_frame          depth_d_frame;  // Depth mode depth frame bindings

        const ogl::program *color_program;  // Color mode shader program
        unit_texture        color_texture;  // Color mode texture bindings
        unit_process        color_process;  // Color mode process bindings
        unit_frame          color_c_frame;  // Color mode color frame bindings
        unit_frame          color_d_frame;  // Color mode depth frame bindings

        const ogl::program *init_program(app::node, unit_texture&,
                                                    unit_process&,
                                                    unit_frame&,
                                                    unit_frame&);

    public:

        static int  shadow_count();
        static bool bind_shadow(int);
        static void free_shadow(int);

        static void prep_irradiance(const ogl::program *);

        // Local binding methods.

        const std::string& get_name() const { return name; }

        binding(std::string);
       ~binding();

        bool depth_eq(const binding *) const;
        bool color_eq(const binding *) const;
        bool opaque() const;

        bool bind(bool) const;
    };
}

//-----------------------------------------------------------------------------

#endif
