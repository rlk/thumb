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

#include "ogl-program.hpp"
#include "ogl-texture.hpp"
#include "ogl-binding.hpp"
#include "ogl-surface.hpp"
#include "ogl-image.hpp"
#include "ogl-frame.hpp"
#include "ogl-pool.hpp"

//-----------------------------------------------------------------------------

// TODO: rework these pointers to allow auto_ptr for RAII.
// TODO: consider templating this repetition.

namespace app
{
    class glob
    {
        struct program
        {
            ogl::program *ptr;
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

        std::map<std::string, program> program_map;
        std::map<std::string, texture> texture_map;
        std::map<std::string, binding> binding_map;
        std::map<std::string, surface> surface_map;

        std::set<ogl::pool  *>  pool_set;
        std::set<ogl::image *> image_set;
        std::set<ogl::frame *> frame_set;

        GLuint vbo;
        GLuint ebo;

    public:

        glob() : vbo(0), ebo(0) { }
       ~glob();

        // Named, reference-counted GL state.

        const ogl::program *load_program(std::string, std::string) { return 0; } // HACK HACK HACK

        const ogl::program *load_program(std::string);
        const ogl::texture *load_texture(std::string, GLenum=GL_LINEAR);
        const ogl::binding *load_binding(std::string);
        const ogl::surface *load_surface(std::string);

        const ogl::program *dupe_program(const ogl::program *);
        const ogl::texture *dupe_texture(const ogl::texture *);
        const ogl::binding *dupe_binding(const ogl::binding *);
        const ogl::surface *dupe_surface(const ogl::surface *);

        void free_program(std::string);
        void free_texture(std::string);
        void free_binding(std::string);
        void free_surface(std::string);

        void free_program(const ogl::program *);
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
                              bool=false);

        void free_pool (ogl::pool  *);
        void free_image(ogl::image *);
        void free_frame(ogl::frame *);

        // Flush and reload.

        void init();
        void fini();
    };
}

//-----------------------------------------------------------------------------

extern app::glob *glob;

//-----------------------------------------------------------------------------

#endif
