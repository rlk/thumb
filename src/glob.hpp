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

#ifndef GLOB_HPP
#define GLOB_HPP

#include <map>
#include <set>

#include "program.hpp"
#include "texture.hpp"
#include "geodata.hpp"
#include "image.hpp"
#include "frame.hpp"

//-----------------------------------------------------------------------------

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

        struct geodata
        {
            ogl::geodata *ptr;
            int           ref;
        };

        std::map<std::string, program> program_map;
        std::map<std::string, texture> texture_map;
        std::map<std::string, geodata> geodata_map;

        std::set<ogl::image *> image_set;
        std::set<ogl::frame *> frame_set;

    public:

       ~glob();

        // Named, reference-counted GL state.

        const ogl::program *load_program(std::string);
        const ogl::texture *load_texture(std::string);
        const ogl::geodata *load_geodata(std::string);

        void free_program(std::string);
        void free_texture(std::string);
        void free_geodata(std::string);

        void free_program(const ogl::program *);
        void free_texture(const ogl::texture *);
        void free_geodata(const ogl::geodata *);

        void dupe_program(const ogl::program *);
        void dupe_texture(const ogl::texture *);
        void dupe_geodata(const ogl::geodata *);

        // Anonymous GL state.

        ogl::image *new_image(GLsizei, GLsizei, GLenum, GLenum);
        ogl::frame *new_frame(GLsizei, GLsizei, GLenum, GLenum, GLenum);

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
