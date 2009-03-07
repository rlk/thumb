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

#ifndef APP_GLOB_HPP
#define APP_GLOB_HPP

#include <map>
#include <set>
#include <string>

#include "ogl-opengl.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class uniform;
    class program;
    class process;
    class texture;
    class binding;
    class surface;
    class image;
    class frame;
    class pool;
}

//-----------------------------------------------------------------------------

namespace app
{
    class glob
    {
        struct uniform
        {
            ogl::uniform *ptr;
            int           ref;
        };

        struct program
        {
            ogl::program *ptr;
            int           ref;
        };

        struct process
        {
            ogl::process *ptr;
            int           ref;
        };

        struct texture
        {
            ogl::texture *ptr;
            int           ref;
        };

        struct binding
        {
            ogl::binding *ptr;
            int           ref;
        };

        struct surface
        {
            ogl::surface *ptr;
            int           ref;
        };

        std::map<std::string, uniform> uniform_map;
        std::map<std::string, program> program_map;
        std::map<std::string, process> process_map;
        std::map<std::string, texture> texture_map;
        std::map<std::string, binding> binding_map;
        std::map<std::string, surface> surface_map;

        std::set<ogl::pool  *>  pool_set;
        std::set<ogl::image *> image_set;
        std::set<ogl::frame *> frame_set;

    public:

        glob() { }
       ~glob();

        const ogl::program *load_program(std::string, std::string) { return 0; } // HACK HACK HACK

        // Named, reference-counted GL state.

              ogl::uniform *load_uniform(const std::string&, GLsizei);
        const ogl::program *load_program(const std::string&);
        const ogl::process *load_process(const std::string&, int=0);
        const ogl::texture *load_texture(const std::string&,
                                         const std::string&);
        const ogl::binding *load_binding(const std::string&,
                                         const std::string&);
        const ogl::surface *load_surface(const std::string&, bool);

              ogl::uniform *dupe_uniform(      ogl::uniform *);
        const ogl::program *dupe_program(const ogl::program *);
        const ogl::process *dupe_process(const ogl::process *);
        const ogl::texture *dupe_texture(const ogl::texture *);
        const ogl::binding *dupe_binding(const ogl::binding *);
        const ogl::surface *dupe_surface(const ogl::surface *);

        void free_uniform(const std::string&);
        void free_program(const std::string&);
        void free_process(const std::string&);
        void free_texture(const std::string&);
        void free_binding(const std::string&);
        void free_surface(const std::string&);

        void free_uniform(      ogl::uniform *);
        void free_program(const ogl::program *);
        void free_process(const ogl::process *);
        void free_texture(const ogl::texture *);
        void free_binding(const ogl::binding *);
        void free_surface(const ogl::surface *);

        // Anonymous GL state.

        ogl::pool  *new_pool ();
        ogl::image *new_image(GLsizei,
                              GLsizei,
                              GLenum=GL_TEXTURE_2D,
                              GLenum=GL_RGBA8,
                              GLenum=GL_RGBA,
                              GLenum=GL_UNSIGNED_BYTE);
        ogl::frame *new_frame(GLsizei,
                              GLsizei,
                              GLenum=GL_TEXTURE_2D,
                              GLenum=GL_RGBA8,
                              bool=true,
                              bool=true,
                              bool=false);

        void free_pool (ogl::pool  *);
        void free_image(ogl::image *);
        void free_frame(ogl::frame *);

        // Render prep, state flush, and state reload.

        void prep();
        void init();
        void fini();
    };
}

//-----------------------------------------------------------------------------

extern app::glob *glob;

//-----------------------------------------------------------------------------

#endif
