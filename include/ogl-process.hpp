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

#ifndef OGL_PROCESS_HPP
#define OGL_PROCESS_HPP

#include <string>

#include "ogl-opengl.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class pool;
    class node;
    class frame;
    class binding;
}

//-----------------------------------------------------------------------------

namespace ogl
{
    class process
    {
    protected:

        std::string name;

        static int count;

        // Render to clipspace.

        static ogl::pool *clip_pool;
        static ogl::node *clip_node;

        static void init_clip();
        static void fini_clip();

        // Render to cubemap.

        static ogl::pool *cube_pool;
        static ogl::node *cube_node[6];

        static void init_cube();
        static void fini_cube();
        static void proc_cube(ogl::frame *);

    public:

        const std::string& get_name() const { return name; }

        process(const std::string&);

        virtual void draw(const ogl::binding *) const { };

        virtual void bind_frame() const { }
        virtual void free_frame() const { }
        virtual void bind(GLenum) const { }

        virtual void init() { }
        virtual void fini() { }

        virtual ~process();
    };
}

//-----------------------------------------------------------------------------

#endif
