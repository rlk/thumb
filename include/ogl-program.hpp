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

#ifndef OGL_PROGRAM_HPP
#define OGL_PROGRAM_HPP

#include <string>
#include <map>

#include "app-file.hpp"
#include "ogl-opengl.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class uniform;
}

//-----------------------------------------------------------------------------

namespace ogl
{
    class program
    {
        typedef std::map<ogl::uniform *, GLint>  uniform_map;
        typedef std::map<std::string,    GLenum> sampler_map;

        std::string name;

        GLhandleARB vert;
        GLhandleARB frag;
        GLhandleARB prog;

        uniform_map uniforms;
        sampler_map samplers;

        bool bindable;

        bool log(GLhandleARB, const std::string&);

        GLhandleARB compile(GLenum, const std::string&,
                                    const std::string&);

        std::string load(const std::string&);

        void init_attributes(app::node);
        void init_uniforms  (app::node);
        void init_samplers  (app::node);

    public:

        const std::string& get_name() const { return name; }

        program(std::string);
       ~program();

        void prep() const;
        void bind() const;
        void free() const;

        void init();
        void fini();

        GLenum unit(std::string) const;

        void uniform(std::string, int)                            const;
        void uniform(std::string, double)                         const;
        void uniform(std::string, double, double)                 const;
        void uniform(std::string, double, double, double)         const;
        void uniform(std::string, double, double, double, double) const;
        void uniform(std::string, const double *, bool=false)     const;

        static const program *current;
    };
}

//-----------------------------------------------------------------------------

#endif
