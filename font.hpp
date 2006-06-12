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
