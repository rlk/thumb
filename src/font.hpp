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
#include "image.hpp"
#include "rect.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    //-------------------------------------------------------------------------
    // Typeset texture glyph map element.

    class glyph
    {
        int   x0, x1;
        int   y0, y1;
        float s0, s1;
        float t0, t1;

    public:

        glyph(int, int, int, int, int);

        bool find(int, int) const;
        void draw(int, int) const;

        int L() const { return x0; }
        int R() const { return x1; }
    };

    typedef std::vector<glyph> glyph_v;

    //-------------------------------------------------------------------------
    // Typeset texture.

    class text
    {
        ogl::image *data;
        
        int x, y;
        int inner_w;
        int inner_h;
        int outer_w;
        int outer_h;

        glyph_v map;

    public:

        text(int, int);
       ~text();

        int w() const { return inner_w; }
        int h() const { return inner_h; }
        int n() const { return int(map.size()); }

        void move(int, int);
        int  find(int, int) const;
        int  curs(int)      const;
        void draw(int)      const;
        void draw()         const;
        void bind()         const;


        void add(int, int);
        void set(const GLubyte *);
    };

    //-------------------------------------------------------------------------
    // Typesetter.

    class font
    {
        std::string filename;
        FT_Library  library;
        FT_Face     face;

        int s;

        GLubyte *glyph(FT_Int32, int, int, int&, int&, int&, int&, int&, int&);

    public:

        font(std::string, int);
       ~font();

        text *render(std::string);

        int size() { return s; }
    };
}

//-----------------------------------------------------------------------------

#endif
