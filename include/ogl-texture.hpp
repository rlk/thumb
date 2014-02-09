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

#ifndef OGL_TEXTURE_HPP
#define OGL_TEXTURE_HPP

#include <vector>
#include <string>
#include <map>

#include <etc-vector.hpp>
#include <ogl-opengl.hpp>

//-----------------------------------------------------------------------------

namespace ogl
{
    class texture
    {
        std::string name;

        GLuint object;
        GLsizei w;
        GLsizei h;
        GLsizei c;

        void load_png(const void *, size_t, std::vector<GLubyte>&);
        void load_jpg(const void *, size_t, std::vector<GLubyte>&); // TODO

        void load_img(std::string, std::map<int, vec4>&);
        void load_opt(std::string, std::map<int, vec4>&);
        void load_prm(std::string);

    public:

        const std::string& get_name() const { return name; }

        texture(std::string);
       ~texture();

        void bind(GLenum=GL_TEXTURE0) const;
        void free(GLenum=GL_TEXTURE0) const;

        void init();
        void fini();

        bool opaque() const { return (c == 1 || c == 3); }
    };
}

//-----------------------------------------------------------------------------

#endif
