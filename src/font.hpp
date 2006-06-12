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

#ifndef FONT_HPP
#define FONT_HPP

#include <string>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "opengl.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    struct rect
    {
        int x;
        int y;
        int w;
        int h;

        rect(int a, int b, int c, int d) : x(a), y(b), w(c), h(d) { }
    };

    class font
    {
        FT_Library library;
        FT_Face    face;

        int s;

        GLubyte *glyph(FT_Int32, int, int, int&, int&, int&, int&, int&, int&);

    public:

        font(std::string, int);
       ~font();

        void   grid(std::string, std::vector<rect>&);
        GLuint draw(std::string, int&, int&, int&, int&);

        int size() { return s; }
    };
}

//-----------------------------------------------------------------------------

#endif
