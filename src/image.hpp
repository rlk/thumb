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

#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <string>

#include "opengl.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class image
    {
    protected:

        GLsizei width;
        GLsizei height;
        GLenum  target;
        GLenum  format;
        GLuint  texture;

    public:

        image();
       ~image();

        void bind(GLenum) const;
        void draw()       const;
    };

    class image_file : public image
    {
    public:
        image_file(std::string);
    };

    class image_snap : public image
    {
    public:
        image_snap();
    };
}

//-----------------------------------------------------------------------------

#endif
