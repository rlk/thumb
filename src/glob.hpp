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

#include "opengl.hpp"
#include "shader.hpp"
#include "image.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    class glob
    {
        std::map<std::string, ogl::shader *> shader_map;
        std::map<std::string, ogl::image  *>  image_map;

    public:

       ~glob();

        const ogl::shader *get_shader(std::string);
        const ogl::image  *get_image (std::string);
    };
}

//-----------------------------------------------------------------------------

#endif
