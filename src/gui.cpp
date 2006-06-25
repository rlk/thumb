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

#include <algorithm>
#include <SDL.h>

#include "main.hpp"
#include "util.hpp"
#include "gui.hpp"

//-----------------------------------------------------------------------------
// Basic widget.

gui::widget::widget() : area(0, 0, 0, 0)
{
    just = 0;

    is_enabled = false;
    is_pressed = false;
    is_invalid = false;
    is_varied  = false;
}

gui::widget::~widget()
{
    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
        delete (*i);

    child.clear();
}

gui::widget *gui::widget::click(int, int, bool d)
{
    is_pressed = d;
    return 0;
}

void gui::widget::value(std::string)
{
}

std::string gui::widget::value() const
{
    return "";
}

void gui::widget::laydn(int x, int y, int w, int h)
{
    area.x = x;
    area.y = y;
    area.w = w;
    area.h = h;
}

void gui::widget::back_color(const widget *focus) const
{
    // Set the background color.

    if      (is_pressed)    glColor4f(0.5f, 0.5f, 0.5f, 0.7f);
    else if (this == focus) glColor4f(0.3f, 0.3f, 0.3f, 0.7f);
    else if (is_enabled)    glColor4f(0.1f, 0.1f, 0.1f, 0.7f);
    else                    glColor4f(0.1f, 0.1f, 0.1f, 0.7f);
}

void gui::widget::fore_color() const
{
    // Set the foreground color.

    if (is_enabled)
        glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    else
        glColor4f(0.0f, 0.0f, 0.0f, 0.2f);
}

//-----------------------------------------------------------------------------
// Leaf widget.

gui::widget *gui::leaf::enter(int x, int y)
{
    // If the point is within this widget, return this.

    return (is_enabled && area.test(x, y)) ? this : 0;
}

void gui::leaf::draw(const widget *focus, const widget *input) const
{
    back_color(focus);

    // Draw the background.

    glBegin(GL_QUADS);
    {
        glVertex2i(area.L(), area.T());
        glVertex2i(area.L(), area.B());
        glVertex2i(area.R(), area.B());
        glVertex2i(area.R(), area.T());
    }
    glEnd();
}

//-----------------------------------------------------------------------------
// Tree widget.

bool gui::tree::exp_w() const
{
    bool b = false;

    // A group is horizontally expanding if any child is.

    for (widget_v::const_iterator i = child.begin(); i != child.end(); ++i)
        b |= (*i)->exp_w();

    return b;
}

bool gui::tree::exp_h() const
{
    bool b = false;

    // A group is vertically expanding if any child is.

    for (widget_v::const_iterator i = child.begin(); i != child.end(); ++i)
        b |= (*i)->exp_h();

    return b;
}

gui::widget *gui::tree::enter(int x, int y)
{
    widget *w;

    // Search all child nodes for the widget containing the given point.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
        if ((w = (*i)->enter(x, y)))
            return w;

    return 0;
}

void gui::tree::show()
{
    // Show all child nodes.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
        (*i)->show();
}

void gui::tree::hide()
{
    // Hide all child nodes.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
        (*i)->hide();
}

void gui::tree::draw(const widget *focus, const widget *input) const
{
    // Draw all child nodes.

    for (widget_v::const_iterator i = child.begin(); i != child.end(); ++i)
        (*i)->draw(focus, input);
}

gui::tree::~tree()
{
    // Delete all child nodes.

    for (widget_v::const_iterator i = child.begin(); i != child.end(); ++i)
        delete (*i);

    child.clear();
}

//-----------------------------------------------------------------------------
// Basic string widget.

gui::string::string(app::font *f, std::string s, int j,
                    GLubyte r, GLubyte g, GLubyte b) :

    text(0), font(f), str(lang->get(s))
{
    color[0] = r;
    color[1] = g;
    color[2] = b;
    just     = j;

    update();

    // Pad the string.

    area.w = text->w() + f->size() * 2;
    area.h = text->h() + f->size();
}

void gui::string::text_color() const
{
    // Set the text color.

    if      (is_varied)  glColor4ub(0xFF, 0xFF, 0x40, 0xC0);
    else if (is_invalid) glColor4ub(0xFF, 0x20, 0x20, 0xC0);
    else                 glColor3ubv(color);
}

void gui::string::dotext() const
{
    // Draw the string.

    glPushAttrib(GL_ENABLE_BIT);
    {
        glEnable(GL_TEXTURE_2D);

        text_color();
        text->draw();
    }
    glPopAttrib();
}

void gui::string::update()
{
    if (text) delete text;

    // Render the new string texture.

    text = font->render(str);

    // Set the justification of the new string.

    int jx = (area.w - text->w()) / 2;
    int jy = (area.h - text->h()) / 2;

    if (just > 0) jx = area.w - text->w() - 4;
    if (just < 0) jx = 4;

    text->move(area.x + jx, area.y + jy);
}

void gui::string::value(std::string s)
{
    str = lang->get(s);
    update();
}

std::string gui::string::value() const
{
    return str;
}

void gui::string::laydn(int x, int y, int w, int h)
{
    leaf::laydn(x, y, w, h);

    // Set the justification of the string object.

    int jx = (area.w - text->w()) / 2;
    int jy = (area.h - text->h()) / 2;

    if (just > 0) jx = area.w - text->w() - 4;
    if (just < 0) jx = 4;

    text->move(area.x + jx, area.y + jy);
}

void gui::string::draw(const widget *focus, const widget *input) const
{
    leaf::draw(focus, input);
    dotext();
}

//-----------------------------------------------------------------------------
// Pushbutton widget.

gui::button::button(std::string text) : string(sans, text)
{
    is_enabled = true;
}

void gui::button::draw(const widget *focus, const widget *input) const
{
    // Draw the background.

    leaf::draw(focus, input);

    // Draw the button shape.

    glBegin(GL_QUADS);
    {
        fore_color();

        glVertex2i(area.L() + 2, area.T() + 2);
        glVertex2i(area.L() + 2, area.B() - 2);
        glVertex2i(area.R() - 2, area.B() - 2);
        glVertex2i(area.R() - 2, area.T() + 2);
    }
    glEnd();

    // Draw the text.

    dotext();
}

gui::widget *gui::button::click(int x, int y, bool d)
{
    widget *input = string::click(x, y, d);

    // Activate the button on mouse up.

    if (d == false) apply();
    return input;
}

//-----------------------------------------------------------------------------
// Bitmap widget.

gui::bitmap::bitmap() : string(mono, "0123456789ABCDEFGHIJKLMNOPQRSTUV")
{
    bits       = 0;
    is_enabled = true;
}

void gui::bitmap::draw(const widget *focus, const widget *input) const
{
    leaf::draw(focus, input);

    // Draw the string.

    glPushAttrib(GL_ENABLE_BIT);
    {
        glEnable(GL_TEXTURE_2D);

        text->bind();

        glBegin(GL_QUADS);
        {
            int i;
            int b;

            for (i = 0, b = 1; i < text->n(); ++i, b <<= 1)
            {
                if (bits & b)
                    text_color();
                else
                    glColor3f(0.0f, 0.0f, 0.0f);

                text->draw(i);
            }
        }
        glEnd();
    
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glPopAttrib();
}

gui::widget *gui::bitmap::click(int x, int y, bool d)
{
    if (d == false)
    {
        int i;

        if ((i = text->find(x, y)) >= 0)
        {
            // If shift, set all bits to the toggle of bit i.

            if (SDL_GetModState() & KMOD_SHIFT)
            {
                if ((bits & (1 << i)))
                    bits = 0x00000000;
                else
                    bits = 0xFFFFFFFF;
            }

            // If control, toggle all bits.

            else if (SDL_GetModState() & KMOD_CTRL)
                for (int j = 0; j < text->n(); ++j)
                    bits = bits ^ (1 << j);

            // Otherwise toggle bit i.

            else bits = bits ^ (1 << i);

            return this;
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
// Editable text widget.

std::string gui::editor::clip;

gui::editor::editor(std::string t) : string(mono, t)
{
    is_enabled = true;

    si = 0;
    sc = 0;

    just = -1;
}

void gui::editor::update()
{
    is_varied = false;

    string::update();

    // Make sure the cursor position is sane.

    si = MIN(si, text->n());
    sc = 0;
}

void gui::editor::grow_select(int sd)
{
    int sn = int(str.length());
    int sj = si;

    // Extend the selection.

    if (sd > 0)
    {
        sc += sd;
        sc  = MIN(sc, sn - si);
    }
    else
    {
        si += sd;
        si  = MAX(si, 0);
        sc += (sj - si);
    }
}

void gui::editor::move_select(int sd)
{
    int sn = int(str.length());

    // Move the cursor.

    if (sc)
    {
        if (sd > 0)
            si += sc + sd - 1;
        else
            si +=      sd + 1;
    }
    else
        si += sd;

    sc = 0;

    si = MAX(si, 0);
    si = MIN(si, sn);
}

void gui::editor::draw(const widget *focus, const widget *input) const
{
    leaf::draw(focus, input);

    glBegin(GL_QUADS);
    {
        // Draw the editor shape.

        fore_color();

        glVertex2i(area.L() + 2, area.T() + 2);
        glVertex2i(area.L() + 2, area.B() - 2);
        glVertex2i(area.R() - 2, area.B() - 2);
        glVertex2i(area.R() - 2, area.T() + 2);

        // Draw the selection / cursor.

        if (this == input)
        {
            int L = text->curs(0) - 1;
            int R = text->curs(0) + 1;

            if (sc)
            {
                L = text->curs(si     );
                R = text->curs(si + sc);
            }
            else if (si)
            {
                L = text->curs(si) - 1;
                R = text->curs(si) + 1;
            }

            glColor4ub(0xFF, 0xC0, 0x40, 0x80);

            glVertex2i(L, area.T() + 3);
            glVertex2i(L, area.B() - 3);
            glVertex2i(R, area.B() - 3);
            glVertex2i(R, area.T() + 3);
        }
    }
    glEnd();

    // Draw the text.

    dotext();
}

gui::widget *gui::editor::click(int x, int y, bool d)
{
    int sp = text->find(x, y);

    leaf::click(x, y, d);

    if (sp >= 0)
    {
        if (d)
        {
            si = sp;
            sc = 0;
        }
    }
    else
    {
        if (d)
        {
            si = int(str.length());
            sc = 0;
        }
    }
    return this;
}

void gui::editor::point(int x, int y)
{
    int sp = text->find(x, y);

    if (sp >= 0)
    {
        if (sp > si)
            sc = sp - si;
        else
        {
            sc = si + sc - sp;
            si = sp;
        }
    }
    else sc = int(str.length()) - si;
}

void gui::editor::keybd(int k, int c)
{
    int n = int(str.length());
    int s = SDL_GetModState() & KMOD_SHIFT;

    switch (k)
    {
    // Handle cursor and selection keys.

    case SDLK_LEFT:  if (s) grow_select(-1); else move_select(-1); return;
    case SDLK_RIGHT: if (s) grow_select(+1); else move_select(+1); return;
    case SDLK_HOME:  if (s) grow_select(-n); else move_select(-n); return;
    case SDLK_END:   if (s) grow_select(+n); else move_select(+n); return;

    case SDLK_RETURN:
        update();
        return;

   // Handle deletion to the left.

    case SDLK_BACKSPACE:
        if (sc)
            str.erase(si, sc);
        else if (si > 0)
            str.erase(--si, 1);

        sc = 0;
        update();
        return;

    // Handle deletion to the right.

    case SDLK_DELETE:
        if (sc)
            str.erase(si, sc);
        else
            str.erase(si, 1);

        sc = 0;
        update();
        return;
    }

    // Handle text copy.

    if (c == 3 && sc > 0)
        clip = str.substr(si, sc);

    // Handle text paste.

    if (c == 22 && clip.length() > 0)
    {
        str.replace(si, sc, clip);
        si = si + int(clip.length());
        sc = 0;
        update();
    }

    // Handle text insertion.

    if (32 <= c && c < 127)
    {
        str.replace(si, sc, std::string(1, c));
        si = si + 1;
        sc = 0;
        update();
    }
}

gui::editor::~editor()
{
    if (text) delete text;
}

//-----------------------------------------------------------------------------
// Vertically scrolling panel.

void gui::scroll::layup()
{
    // Find the widest child width.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
    {
        (*i)->layup();

        area.w = MAX((*i)->get_w(),  get_w());
        area.h =    ((*i)->get_h() + get_h());
    }

    // Include the scrollbar width.  Height is arbitrary.

    area.w += scroll_w;

    child_h = area.h;
    area.h  = 0;
}

void gui::scroll::laydn(int x, int y, int w, int h)
{
    int c = 0, excess = MAX(h - child_h, 0);

    widget::laydn(x, y, w, h);

    // Count the number of expanding children.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
        if ((*i)->exp_h())
            c++;

    // Give children requested vertical space.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
    {
        int dy;

        if ((*i)->exp_h())
            (*i)->laydn(x, y, w - scroll_w, (dy = (excess / c)));
        else
            (*i)->laydn(x, y, w - scroll_w, (dy = (*i)->get_h()));

        y += dy;
    }
}

gui::widget *gui::scroll::click(int x, int y, bool d)
{
    widget *input = widget::click(x, y, d);

    // Focus only lands here when the pointer is in the scrollbar.

    point(x, y);
    return input;
}

void gui::scroll::point(int x, int y)
{
    int thumb_h;
    int m;

    // We know the pointer is in the scrollbar, so scroll.

    if ((m = MAX(child_h - area.h, 0)) > 0)
    {
        thumb_h = (area.h - 4) * area.h / child_h;

        child_d = m * (y - (area.y + thumb_h / 2 + 2)) /
                           (area.h - thumb_h - 4);

        if (child_d > m) child_d = m;
        if (child_d < 0) child_d = 0;
    }
}

gui::widget *gui::scroll::enter(int x, int y)
{
    // Check for a hit on the scrollbar, or search the child list.

    if (is_enabled)
    {
        rect C(area.x,                     area.y, area.w - scroll_w, area.h);
        rect S(area.x + area.w - scroll_w, area.y,          scroll_w, area.h);

        if (C.test(x, y)) return tree::enter(x, y + child_d);
        if (S.test(x, y)) return this;
    }
    return 0;
}

void gui::scroll::draw(const widget *focus, const widget *input) const
{
    int thumb_h = area.h - 4;
    int thumb_y = area.y + 2;
    int m;

    // Compute the size and position of the scroll thumb.

    if ((m = MAX(child_h - area.h, 0)) > 0)
    {
        thumb_h = thumb_h *  area.h / child_h;
        thumb_y = thumb_y + (area.h - thumb_h - 4) * child_d / m;
    }

    // Draw the scroll bar.

    glPushAttrib(GL_ENABLE_BIT);
    {
        glDisable(GL_TEXTURE_2D);

        glBegin(GL_QUADS);
        {
            back_color(focus);

            glVertex2i(area.R() - scroll_w, area.T());
            glVertex2i(area.R() - scroll_w, area.B());
            glVertex2i(area.R(),            area.B());
            glVertex2i(area.R(),            area.T());
        }
        glEnd();

        // Draw the scroll thumb.

        glBegin(GL_QUADS);
        {
            fore_color();

            glVertex2i(area.R() - scroll_w + 2, thumb_y);
            glVertex2i(area.R() - scroll_w + 2, thumb_y + thumb_h);
            glVertex2i(area.R()            - 2, thumb_y + thumb_h);
            glVertex2i(area.R()            - 2, thumb_y);
        }
        glEnd();
    }
    glPopAttrib();

    // Draw the children.

    glPushAttrib(GL_ENABLE_BIT);
    glPushMatrix();
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(area.x, conf->get_i("window_h") - area.y - area.h,
                  area.w - scroll_w, area.h);

        glTranslatef(0, float(-child_d), 0);
        tree::draw(focus, input);
    }
    glPopMatrix();
    glPopAttrib();
}

//-----------------------------------------------------------------------------
// File selection list.

void gui::finder::enlist()
{
    // List all files and subdirectories in the current directory.

    strvec dirs;
    strvec regs;

    dir.get(dirs, regs, ext);

    std::sort(dirs.begin(), dirs.end());
    std::sort(regs.begin(), regs.end());

    // Add a new file button for each file and subdirectory.

    for (strvec::iterator i = dirs.begin(); i != dirs.end(); ++i)
        add(new finder_dir(*i, this));

    for (strvec::iterator i = regs.begin(); i != regs.end(); ++i)
        add(new finder_reg(*i, this));

    add(new gui::filler());
}

void gui::finder::update()
{
    // Delete all child nodes.

    for (widget_v::const_iterator i = child.begin(); i != child.end(); ++i)
        delete (*i);

    child.clear();

    // Create the new child list.

    enlist();

    // Find the total height of all children.

    child_d = 0;
    child_h = 0;

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
        child_h += (*i)->get_h();

    // Lay out this new widget hierarchy.

    laydn(area.x, area.y, area.w, area.h);
}

void gui::finder::set_dir(std::string& name)
{
    dir.set(name);
    update();
    conf->set_s(key, dir.cwd());
}

void gui::finder::set_reg(std::string& name)
{
    file = name;

    if (state)
        state->value(dir.cwd() + file);
}

void gui::finder::show()
{
    // Refresh the directory listing on show.

    update();
    scroll::show();
}

//-----------------------------------------------------------------------------
// File selection list elements.

gui::finder_elt::finder_elt(std::string& t, gui::finder *f) :
    string(mono, t, -1, 0xFF, 0xFF, 0xFF), state(f)
{
    is_enabled = true;

    area.w = text->w() + mono->size() * 2;
    area.h = text->h() + mono->size() / 2;
}

void gui::finder_elt::draw(const widget *focus, const widget *input) const
{
    //Draw the background.

    leaf::draw(focus, input);

    // Draw the finder element shape.

    glBegin(GL_QUADS);
    {
        fore_color();

        glVertex2i(area.L(), area.T());
        glVertex2i(area.L(), area.B());
        glVertex2i(area.R(), area.B());
        glVertex2i(area.R(), area.T());
    }
    glEnd();

    // Draw the string.

    dotext();
}

gui::widget *gui::finder_elt::click(int x, int y, bool d)
{
    widget *input = string::click(x, y, d);

    // Activate the finder element on mouse up.

    if (d == false) apply();
    return input;
}

//-----------------------------------------------------------------------------
// Horizontal group of equally-sized widgets.

void gui::harray::layup()
{
    // Find the widest child width and the highest child height.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
    {
        (*i)->layup();

        area.w = MAX((*i)->get_w(), get_w());
        area.h = MAX((*i)->get_h(), get_h());
    }

    // Total width is the widest child width times the child count.

    area.w *= int(child.size());
}

void gui::harray::laydn(int x, int y, int w, int h)
{
    int n = int(child.size()), c = 0;

    widget::laydn(x, y, w, h);

    // Distribute horizontal space evenly to all children.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i, ++c)
    {
        int x0 = x + (c    ) * w / n;
        int x1 = x + (c + 1) * w / n;

        (*i)->laydn(x0, y, x1 - x0, h);
    }
}

//-----------------------------------------------------------------------------
// Vertical group of equally sized widgets.

void gui::varray::layup()
{
    // Find the widest child width and the highest child height.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
    {
        (*i)->layup();

        area.w = MAX((*i)->get_w(), get_w());
        area.h = MAX((*i)->get_h(), get_h());
    }

    // Total height is the heighest child height times the child count.

    area.h *= int(child.size());
}

void gui::varray::laydn(int x, int y, int w, int h)
{
    int n = int(child.size()), c = 0;

    widget::laydn(x, y, w, h);

    // Distribute vertical space evenly to all children.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i, ++c)
    {
        int y0 = y + (c    ) * h / n;
        int y1 = y + (c + 1) * h / n;

        (*i)->laydn(x, y0, w, y1 - y0);
    }
}

//-----------------------------------------------------------------------------
// Horizontal group of widgets.

void gui::hgroup::layup()
{
    // Find the highest child height.  Sum the child widths.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
    {
        (*i)->layup();

        area.w =    ((*i)->get_w() + get_w());
        area.h = MAX((*i)->get_h(),  get_h());
    }
}

void gui::hgroup::laydn(int x, int y, int w, int h)
{
    int dx, c = 0, excess = w - area.w;

    widget::laydn(x, y, w, h);

    // Count the number of expanding children.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
        if ((*i)->exp_w())
            c++;

    // Give children requested space.  Distribute excess among the expanders.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
    {
        int W = (*i)->get_w();

        if ((*i)->exp_w())
            (*i)->laydn(x, y, (dx = W + (excess / c)), h);
        else
            (*i)->laydn(x, y, (dx = W), h);

        x += dx;
    }
}

//-----------------------------------------------------------------------------
// Vertical group of widgets.

void gui::vgroup::layup()
{
    // Find the widest child width.  Sum the child heights.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
    {
        (*i)->layup();

        area.w = MAX((*i)->get_w(),  get_w());
        area.h =    ((*i)->get_h() + get_h());
    }
}

void gui::vgroup::laydn(int x, int y, int w, int h)
{
    int dy, c = 0, excess = h - area.h;

    widget::laydn(x, y, w, h);

    // Count the number of expanding children.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
        if ((*i)->exp_h())
            c++;

    // Give children requested space.  Distribute excess among the expanders.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
    {
        int H = (*i)->get_h();

        if ((*i)->exp_h())
            (*i)->laydn(x, y, w, (dy = H + (excess / c)));
        else
            (*i)->laydn(x, y, w, (dy = H));

        y += dy;
    }
}

//-----------------------------------------------------------------------------
// Tabb selector panel.

void gui::option::layup()
{
    // Find the largest child width extent.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
    {
        (*i)->layup();

        area.w = MAX((*i)->get_w(), get_w());
        area.h = MAX((*i)->get_h(), get_h());
    }
}

void gui::option::laydn(int x, int y, int w, int h)
{
    widget::laydn(x, y, w, h);

    // Give children all available space.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
        (*i)->laydn(x, y, w, h);
}

gui::widget *gui::option::enter(int x, int y)
{
    return child[index]->enter(x, y);
}

void gui::option::draw(const widget *focus, const widget *input) const
{
    child[index]->draw(focus, input);
}

//-----------------------------------------------------------------------------
// Padding frame.

void gui::frame::layup()
{
    // Find the largest child width extent.  Pad it.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
    {
        (*i)->layup();

        area.w = MAX((*i)->get_w(), get_w());
        area.h = MAX((*i)->get_h(), get_h());
    }

    area.w += s + s;
    area.h += s + s;
}

void gui::frame::laydn(int x, int y, int w, int h)
{
    widget::laydn(x, y, w, h);

    // Give children all available space.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
        (*i)->laydn(x + s, y + s, w - s - s, h - s - s);
}

void gui::frame::draw(const widget *focus, const widget *input) const
{
    glPushAttrib(GL_ENABLE_BIT);
    {
        glDisable(GL_TEXTURE_2D);

        back_color(focus);

        // Draw the background.

        glBegin(GL_QUADS);
        {
            const int Lo = area.L();
            const int Li = area.L() + s;
            const int Ri = area.R() - s;
            const int Ro = area.R();
            const int To = area.T();
            const int Ti = area.T() + s;
            const int Bi = area.B() - s;
            const int Bo = area.B();

            glVertex2i(Lo, To);
            glVertex2i(Lo, Bo);
            glVertex2i(Li, Bi);
            glVertex2i(Li, Ti);

            glVertex2i(Ri, Ti);
            glVertex2i(Ri, Bi);
            glVertex2i(Ro, Bo);
            glVertex2i(Ro, To);

            glVertex2i(Lo, To);
            glVertex2i(Li, Ti);
            glVertex2i(Ri, Ti);
            glVertex2i(Ro, To);

            glVertex2i(Li, Bi);
            glVertex2i(Lo, Bo);
            glVertex2i(Ro, Bo);
            glVertex2i(Ri, Bi);
        }
        glEnd();
    }
    glPopAttrib();

    tree::draw(focus, input);
}

//-----------------------------------------------------------------------------
// Top level dialog.

gui::dialog::dialog()
{
    root  = 0;
    focus = 0;
    input = 0;
}

gui::dialog::~dialog()
{
    if (root) delete root;
}

void gui::dialog::point(int x, int y)
{
    // Dragging outside of a widget should not defocus it.

    if (focus && focus->pressed())
        focus->point(x, y);
    else
        focus = root->enter(x, y);

    last_x = x;
    last_y = y;
}

void gui::dialog::click(bool d)
{
    // Click any focused widget.  Shift the input focus there.

    if (focus)
        input = focus->click(last_x, last_y, d);

    // Activation may have resulted in widget deletion.  Confirm the focus.

    if (d == false)
        focus = root->enter(last_x, last_y);
}

void gui::dialog::keybd(int k, int c)
{
    if (input)
        input->keybd(k, c);
}

void gui::dialog::show()
{
    if (root) root->show();
}

void gui::dialog::hide()
{
    if (root) root->hide();

    input = 0;
    focus = 0;
}

void gui::dialog::draw() const
{
    if (root)
    {
        glPushAttrib(GL_ENABLE_BIT);
        {
            glMatrixMode(GL_PROJECTION);
            {
                glLoadIdentity();
                view->mult_O();
            }
            glMatrixMode(GL_MODELVIEW);
            {
                glLoadIdentity();
            }

            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_TEXTURE_2D);

            glEnable(GL_BLEND);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            root->draw(focus, input);
        }
        glPopAttrib();
    }
}

//-----------------------------------------------------------------------------
