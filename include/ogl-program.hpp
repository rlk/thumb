//  Copyright (C) 2007-2011 Robert Kooima
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

#include <etc-vector.hpp>
#include <ogl-opengl.hpp>
#include <app-file.hpp>

//-----------------------------------------------------------------------------

namespace ogl
{
    class uniform;
    class process;
}

//-----------------------------------------------------------------------------

namespace ogl
{
    class program
    {
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

        bool discards() const { return discard; }

        void uniform(std::string, int)                     const;
        void uniform(std::string, double)                  const;
        void uniform(std::string, const vec2&)             const;
        void uniform(std::string, const vec3&)             const;
        void uniform(std::string, const vec4&)             const;
        void uniform(std::string, const mat3&, bool=false) const;
        void uniform(std::string, const mat4&, bool=false) const;

        static const program *current;

    private:

        typedef std::map<      std::string,    GLenum> texture_map;
        typedef std::map<const ogl::process *, GLenum> process_map;
        typedef std::map<      ogl::uniform *, GLint>  uniform_map;

        std::string name;

        GLhandleARB vert;
        GLhandleARB frag;
        GLhandleARB prog;

        texture_map textures;
        process_map processes;
        uniform_map uniforms;

        bool bindable;
        bool discard;

        bool program_log(GLhandleARB, const std::string&);
        bool  shader_log(GLhandleARB, const std::string&);

        GLhandleARB compile(GLenum, const std::string&,
                                    const std::string&);

        std::string load(const std::string&);

        void init_attributes(app::node);
        void init_textures  (app::node);
        void init_processes (app::node);
        void init_uniforms  (app::node);
    };
}

//-----------------------------------------------------------------------------

#endif
