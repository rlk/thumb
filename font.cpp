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

#include <stdexcept>

#include "font.hpp"

//-----------------------------------------------------------------------------

app::font::font(std::string filename, int size) : s(size)
{
    // Initialize the font library and font face.

    if (FT_Init_FreeType(&library))
        throw std::runtime_error("Failure starting FreeType2");

    if (FT_New_Face(library, filename.c_str(), 0, &face))
        throw std::runtime_error("Failure loading font file");

    if (FT_Set_Pixel_Sizes(face, 0, size))
        throw std::runtime_error("Failure setting font size");
}

app::font::~font()
{
    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

//-----------------------------------------------------------------------------

static int next_power_of_2(int n)
{
    int m = 1;

    while (m < n)
        m <<= 1;

    return m;
}

static int utf8(std::string& text, int& i)
{
    if ((text[i] & 0x80) == 0x00)
    {
        int a = text[i++];

        return a;
    }
    if ((text[i] & 0xE0) == 0xC0)
    {
        int a = text[i++] & 0x1F;
        int b = text[i++] & 0x3F;

        return (a << 6) | b;
    }
    if ((text[i] & 0xF0) == 0xE0)
    {
        int a = text[i++] & 0x0F;
        int b = text[i++] & 0x3F;
        int c = text[i++] & 0x3F;

        return (a << 12) | (b << 6) | c;
    }
    if ((text[i] & 0xF8) == 0xF0)
    {
        int a = text[i++] & 0x07;
        int b = text[i++] & 0x3F;
        int c = text[i++] & 0x3F;
        int d = text[i++] & 0x3F;

        return (a << 18) | (b << 12) | (c << 8) | d;
    }
    return 0;
}

GLubyte *app::font::glyph(FT_Int32 flag, int curr, int prev,
                          int& x, int& y, int& w, int& h, int& a, int& k)
{
    FT_Vector kern;

    // Look up the current pair of characters.

    FT_UInt L = FT_Get_Char_Index(face, prev);
    FT_UInt R = FT_Get_Char_Index(face, curr);

    // Get the kerning and glyph sizes.

    FT_Get_Kerning(face, L, R, FT_KERNING_DEFAULT, &kern);
    FT_Load_Glyph (face, R, flag);

    // Convert these values to pixels and return the buffer.

    x = face->glyph->metrics.horiBearingX >> 6;
    y = face->glyph->metrics.horiBearingY >> 6;
    w = face->glyph->metrics.width        >> 6;
    h = face->glyph->metrics.height       >> 6;
    a = face->glyph->advance.x            >> 6;
    k = kern.x                            >> 6;

    return (GLubyte *) face->glyph->bitmap.buffer;
}

void app::font::grid(std::string text, std::vector<rect>& v)
{
    int n = int(text.length());

    int x, y;
    int w, h;
    int a, k;
    int c;

    int curr = 0;
    int prev = 0;

    int px = 0;
    int py = y = -face->size->metrics.descender >> 6;

    // Compute the location of each glyph of the given string.

    for (px = 0, c = 0; c < n; )
    {
        curr = utf8(text, c);
        glyph(FT_LOAD_DEFAULT, curr, prev, x, y, w, h, a, k);
        prev = curr;

        v.push_back(rect(px + x, py + y, w, h));
        px += k + a;
    }
}

GLuint app::font::draw(std::string text, int& inner_w, int& inner_h,
                                         int& outer_w, int& outer_h)
{
    int n = int(text.length());

    int c;
    int x, y;
    int w, h;
    int a, k;
    int i, j;

    int px = 0;
    int py = y = -face->size->metrics.descender >> 6;

    int curr = 0;
    int prev = 0;

    GLubyte *src;
    GLubyte *dst;
    GLubyte *pix;

    // Compute the total size of the string as rendered in this font.

    for (px = 0, c = 0; c < n; )
    {
        curr = utf8(text, c);
        src  = glyph(FT_LOAD_DEFAULT, curr, prev, x, y, w, h, a, k);
        prev = curr;

        px  += k + a;
    }

    inner_w = px;
    inner_h = face->size->metrics.height >> 6;

    // Allocate an RGBA texture buffer to render the text.

    outer_w = next_power_of_2(inner_w);
    outer_h = next_power_of_2(inner_h);

    dst = new GLubyte[outer_w * outer_h * 4];

    memset(dst, 0, outer_w * outer_h * 4);

    // Render the text to the buffer.

    for (px = 0, c = 0; c < n; )
    {
        curr = utf8(text, c);
        src  = glyph(FT_LOAD_RENDER, curr, prev, x, y, w, h, a, k);
        prev = curr;

        px  += k;

        for (i = 0; i < h; ++i)
            for (j = 0; j < w; ++j)
            {
                pix = dst + 4 * ((py + y - i) * outer_w + (px + x + j));

                pix[0] = 0xFF;
                pix[1] = 0xFF;
                pix[2] = 0xFF;
                pix[3] = (GLubyte) (pix[3] + src[i * w + j]);
            }

        px += a;
    }

    // Create a texture object using this buffer.

    GLuint o = 0;

    glGenTextures(1, &o);
    glBindTexture(GL_TEXTURE_2D, o);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, outer_w, outer_h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, dst);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    delete [] dst;

    return o;
}

//-----------------------------------------------------------------------------
