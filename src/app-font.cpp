//  Copyright (C) 2005-2011 Robert Kooima
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
#include <cassert>

#include <app-font.hpp>
#include <app-data.hpp>
#include <app-glob.hpp>
#include <etc-math.hpp>

//-----------------------------------------------------------------------------

app::glyph::glyph(int x, int w, int inner_h, int outer_w, int outer_h) :

    x0(x), x1(x + w),
    y0(0), y1(inner_h),

    s0(float(x0) / float(outer_w)),
    s1(float(x1) / float(outer_w)),
    t0(float(y0) / float(outer_h)),
    t1(float(y1) / float(outer_h))
{
}

bool app::glyph::find(int x, int y) const
{
    // Return true if the given point falls within this glyph.

    return (x0 <= x && x < x1 && y0 <= y && y < y1);
}

void app::glyph::draw(int x, int y) const
{
    // Draw an individual textured glyph rectangle.

    glBegin(GL_QUADS);
    {
        glTexCoord2f(s0, t0); glVertex2i(x + x0, y + y0);
        glTexCoord2f(s1, t0); glVertex2i(x + x1, y + y0);
        glTexCoord2f(s1, t1); glVertex2i(x + x1, y + y1);
        glTexCoord2f(s0, t1); glVertex2i(x + x0, y + y1);
    }
    glEnd();
}

//-----------------------------------------------------------------------------

app::text::text(int w, int h) :
    data(0),
    x(0), y(0),
    inner_w(w),
    inner_h(h),
    outer_w(0),
    outer_h(0)
{
    outer_w = next_pow2(inner_w);
    outer_h = next_pow2(inner_h);

    if (outer_w && outer_h)
        data = ::glob->new_image(outer_w, outer_h);
}

app::text::~text()
{
    if (data) ::glob->free_image(data);
}

void app::text::move(int x, int y)
{
    this->x = x;
    this->y = y;
}

int app::text::find(int x, int y) const
{
    // Find and return the index of the glyph containing the given point.

    for (int i = 0; i < int(map.size()); ++i)
        if (map[i].find(x - this->x, y - this->y))
            return i;

    // Return negative to indicate failure.

    return -1;
}

int app::text::curs(int i) const
{
    // Return the cursor position at the given glyph index.

    if (map.empty())
        return x;
    else
    {
        if (i < int(map.size()))
            return x + map[i].L();
        else
            return x + map.back().R();
    }
}

void app::text::draw(int i) const
{
    // Draw only the requested glyph.

    if (data)
    {
        data->bind();
        {
            map[i].draw(x, y);
        }
        data->free();
    }
}

void app::text::draw() const
{
    if (data)
    {
        const float s = float(inner_w) / float(outer_w);
        const float t = float(inner_h) / float(outer_h);
    
        // Draw the entire textured string at once.
    
        data->bind();
        {
            glBegin(GL_QUADS);
            {
                glTexCoord2f(0, 0); glVertex2i(x,           y);
                glTexCoord2f(s, 0); glVertex2i(x + inner_w, y);
                glTexCoord2f(s, t); glVertex2i(x + inner_w, y + inner_h);
                glTexCoord2f(0, t); glVertex2i(x,           y + inner_h);
            }
            glEnd();
        }
        data->free();
    }
}

void app::text::add(int x, int w)
{
    // Append a new glyph map object.

    map.push_back(glyph(x, w, inner_h, outer_w, outer_h));
}

void app::text::set(const GLubyte *p)
{
    // Copy the given glyph image to the data object.

    if (data)
    {
        data->zero();
        data->blit(p, 0, 0, inner_w, inner_h);
    }
}

//-----------------------------------------------------------------------------

app::font::font(std::string filename, int size) : filename(filename), s(size)
{
    size_t     len;
    const void *ptr = ::data->load(filename, &len);

    // Initialize the font library and font face.

    if (FT_Init_FreeType(&library))
        throw std::runtime_error("Failure starting FreeType2");

    if (FT_New_Memory_Face(library, (const FT_Byte *) ptr, len, 0, &face))
        throw std::runtime_error("Failure loading font file");

    if (FT_Set_Pixel_Sizes(face, 0, size))
        throw std::runtime_error("Failure setting font size");
}

app::font::~font()
{
    ::data->free(filename);

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

//-----------------------------------------------------------------------------

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

app::text *app::font::render(std::string text)
{
    int i, n = int(text.length());

    int x, w, a, W, X = 0;
    int y, h, k, H, Y = -face->size->metrics.descender >> 6;

    int curr = 0;
    int prev = 0;

    GLubyte   *src;
    GLubyte   *dst;
    app::text *T;

    // Compute the total size of the string as rendered in this font.

    for (X = 0, i = 0; i < n; X += k + a)
    {
        curr = utf8(text, i);
        src  = glyph(FT_LOAD_DEFAULT, curr, prev, x, y, w, h, a, k);
        prev = curr;
    }

    W = X;
    H = face->size->metrics.height >> 6;

    // Allocate a buffer to accumulate glyph renderings.

    memset((dst = new GLubyte[W * H * 4]), 0, W * H * 4);

    // Allocate a text object to recieve the image.

    T = new app::text(W, H);

    // Render the text.

    for (X = 0, i = 0; i < n; X += k + a)
    {
        curr = utf8(text, i);
        src  = glyph(FT_LOAD_RENDER, curr, prev, x, y, w, h, a, k);
        prev = curr;

        T->add(X + x + k, w);

        for (int r = 0; r < h; ++r)
        {
            for (int c = 0; c < w; ++c)
            {
                GLubyte *pix = dst + 4 * ((Y + y - r) * W + (X + x + k + c));

                pix[0] = 0xFF;
                pix[1] = 0xFF;
                pix[2] = 0xFF;
                pix[3] = (GLubyte) (pix[3] + src[r * w + c]);
            }
        }
    }

    T->set(dst);

    delete [] dst;

    return T;
}

//-----------------------------------------------------------------------------
