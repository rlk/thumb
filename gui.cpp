#include <algorithm>
#include <SDL.h>

#include "main.hpp"
#include "util.hpp"
#include "gui.hpp"

//-----------------------------------------------------------------------------
// Basic widget.

gui::widget::widget()
{
    x = 0;
    y = 0;
    w = 0;
    h = 0;

    just = 0;

    is_enabled = false;
    is_pressed = false;
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
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
}

void gui::widget::back_color(const widget *focus) const
{
    // Set the background color.

    if      (is_pressed)    glColor4f(0.5f, 0.5f, 0.5f, 0.7f);
    else if (this == focus) glColor4f(0.2f, 0.2f, 0.2f, 0.7f);
    else if (is_enabled)    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    else                    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
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

    return (is_enabled && this->x <= x && x <= this->x + this->w &&
                          this->y <= y && y <= this->y + this->h) ? this : 0;
}

void gui::leaf::draw(const widget *focus, const widget *input) const
{
    back_color(focus);

    // Draw the background.

    glBegin(GL_QUADS);
    {
        glVertex2i(x,     y);
        glVertex2i(x,     y + h);
        glVertex2i(x + w, y + h);
        glVertex2i(x + w, y);
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

gui::string::string(app::font *f, std::string t, int j,
                    GLubyte r, GLubyte g, GLubyte b) :
    font(f), text(lang->get(t))
{
    texture  = 0;
    color[0] = r;
    color[1] = g;
    color[2] = b;
    just     = j;

    update();

    // Pad the string.

    w = inner_w + f->size() * 2;
    h = inner_h + f->size();
}

void gui::string::text_color() const
{
    // Set the text color.

    if (is_varied)
        glColor4ub(0xFF, 0xFF, 0x40, 0xC0);
    else
        glColor3ubv(color);
}

void gui::string::dotext() const
{
    // Draw the string.

    glPushAttrib(GL_ENABLE_BIT);
    {
        glEnable(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, texture);

        glBegin(GL_QUADS);
        {
            int W = MIN(w - 8, inner_w);
            int H = MIN(h - 8, inner_h);

            float s = float(W) / float(outer_w);
            float t = float(H) / float(outer_h);

            int dx = (w - inner_w) / 2;
            int dy = (h - inner_h) / 2;

            if (just > 0) dx = w - inner_w - 4;
            if (just < 0) dx = 4;

            text_color();

            glTexCoord2f(0, t); glVertex2i(x + dx,     y + dy);
            glTexCoord2f(0, 0); glVertex2i(x + dx,     y + dy + H);
            glTexCoord2f(s, 0); glVertex2i(x + dx + W, y + dy + H);
            glTexCoord2f(s, t); glVertex2i(x + dx + W, y + dy);
        }
        glEnd();
    
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glPopAttrib();
}

void gui::string::update()
{
    // Clear the previous string texture.

    if (texture) glDeleteTextures(1, &texture);

    // Draw the new string texture.

    texture = font->draw(text, inner_w, inner_h, outer_w, outer_h);
}

void gui::string::value(std::string s)
{
    text = lang->get(s);
    update();
}

std::string gui::string::value() const
{
    return text;
}

void gui::string::draw(const widget *focus, const widget *input) const
{
    leaf::draw(focus, input);

    dotext();
}

gui::string::~string()
{
    if (texture) glDeleteTextures(1, &texture);
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

        glVertex2i(x     + 2, y     + 2);
        glVertex2i(x     + 2, y + h - 2);
        glVertex2i(x + w - 2, y + h - 2);
        glVertex2i(x + w - 2, y     + 2);
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

    // Clear the previous text selection grid and compute the new one.

    grid.clear();
    font->grid(text, grid);

    // This end-of-string grid marker gives the append cursor a place to go.

    grid.push_back(app::rect(inner_w, 0, 0, inner_h));

    // Make sure the cursor position is sane.

    si = MIN(si, int(text.length()));
    sc = 0;
}

int gui::editor::find_select(int px, int py)
{
    if (grid.size() > 1)
    {
        std::vector<app::rect>::iterator i;
        int j;

        int dx = (w - inner_w) / 2;
        int dy = (h - inner_h) / 2;

        if (just > 0) dx = w - inner_w - 4;
        if (just < 0) dx = 4;

        // Determine which character the pointer is on.

        for (j = 0, i = grid.begin(); (i + 1) != grid.end(); ++i, ++j)
        {
            int x0 = x + dx + (i    )->x;
            int x1 = x + dx + (i + 1)->x;

            int y0 = y + dy;
            int y1 = y - dy + h;

            if (x0 <= px && px <= x1 &&
                y0 <= py && py <= y1) return j;
        }
    }
    return -1;
}

void gui::editor::grow_select(int sd)
{
    int sn = text.length();
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
    int sn = text.length();

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

    int dx = (w - inner_w) / 2;

    if (just > 0) dx = w - inner_w - 4;
    if (just < 0) dx = 4;

    glBegin(GL_QUADS);
    {
        // Draw the editor shape.

        fore_color();

        glVertex2i(x     + 2, y     + 2);
        glVertex2i(x     + 2, y + h - 2);
        glVertex2i(x + w - 2, y + h - 2);
        glVertex2i(x + w - 2, y     + 2);

        // Draw the selection / cursor.

        if (this == input)
        {
            int x0;
            int x1;

            if (sc)
            {
                x0 = x + dx + grid[si     ].x;
                x1 = x + dx + grid[si + sc].x;
            }
            else if (si)
            {
                x0 = x + dx + grid[si].x - 1;
                x1 = x + dx + grid[si].x + 1;
            }
            else
            {
                x0 = x + dx - 1;
                x1 = x + dx + 1;
            }

            glColor4ub(0xFF, 0xC0, 0x40, 0x80);

            glVertex2i(x0, y +     3);
            glVertex2i(x0, y + h - 3);
            glVertex2i(x1, y + h - 3);
            glVertex2i(x1, y +     3);
        }
    }
    glEnd();

    // Draw the text.

    dotext();
}

gui::widget *gui::editor::click(int x, int y, bool d)
{
    int sp = find_select(x, y);

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
            si = text.length();
            sc = 0;
        }
    }
    return this;
}

void gui::editor::point(int x, int y)
{
    int sp = find_select(x, y);

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
    else sc = text.length() - si;
}

void gui::editor::keybd(int k, int c)
{
    int  n = int(text.length());
    bool s = bool(SDL_GetModState() & KMOD_SHIFT);

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
            text.erase(si, sc);
        else if (si > 0)
            text.erase(--si, 1);

        sc = 0;
        update();
        return;

    // Handle deletion to the right.

    case SDLK_DELETE:
        if (sc)
            text.erase(si, sc);
        else
            text.erase(si, 1);

        sc = 0;
        update();
        return;
    }

    // Handle text copy.

    if (c == 3 && sc > 0)
        clip = text.substr(si, sc);

    // Handle text paste.

    if (c == 22 && clip.length() > 0)
    {
        text.replace(si, sc, clip);
        si = si + clip.length();
        sc = 0;
        update();
    }

    // Handle text insertion.

    if (32 <= c && c < 127)
    {
        text.replace(si, sc, std::string(1, c));
        si = si + 1;
        sc = 0;
        update();
    }
}

gui::editor::~editor()
{
    if (texture) glDeleteTextures(1, &texture);
}

//-----------------------------------------------------------------------------
// Vertically scrolling panel.

void gui::scroll::layup()
{
    // Find the widest child width.

    for (widget_v::iterator i = child.begin(); i != child.end(); ++i)
    {
        (*i)->layup();

        w = MAX((*i)->get_w(),  w);
        h =    ((*i)->get_h() + h);
    }

    // Include the scrollbar width.  Height is arbitrary.

    w += scroll_w;

    child_h = h;
    h       = 0;
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

    if ((m = MAX(child_h - h, 0)) > 0)
    {
        thumb_h = (h - 4) * h / child_h;

        child_d = m * (y - (this->y + thumb_h / 2 + 2)) / (h - thumb_h - 4);

        if (child_d > m) child_d = m;
        if (child_d < 0) child_d = 0;
    }
}

gui::widget *gui::scroll::enter(int x, int y)
{
    int y0 = this->y;
    int y1 = this->y + this->h;
    int x0 = this->x;
    int xm = this->x + this->w - this->scroll_w;
    int x1 = this->x + this->w;

    // Check for a hit on the scrollbar, or search the child list.

    if (is_enabled)
    {
        if (y0 <= y && y < y1)
        {
            if (x0 <= x && x < xm) return tree::enter(x, y + child_d);
            if (xm <= x && x < x1) return this;
        }
    }
    return 0;
}

void gui::scroll::draw(const widget *focus, const widget *input) const
{
    int thumb_h = h - 4;
    int thumb_y = y + 2;
    int m;

    // Compute the size and position of the scroll thumb.

    if ((m = MAX(child_h - h, 0)) > 0)
    {
        thumb_h = thumb_h *  h / child_h;
        thumb_y = thumb_y + (h - thumb_h - 4) * child_d / m;
    }

    // Draw the scroll bar.

    glPushAttrib(GL_ENABLE_BIT);
    {
        glDisable(GL_TEXTURE_2D);

        glBegin(GL_QUADS);
        {
            back_color(focus);

            glVertex2i(x + w - scroll_w, y);
            glVertex2i(x + w - scroll_w, y + h);
            glVertex2i(x + w,            y + h);
            glVertex2i(x + w,            y);
        }
        glEnd();

        // Draw the scroll thumb.

        glBegin(GL_QUADS);
        {
            fore_color();

            glVertex2i(x + w - scroll_w + 2, thumb_y);
            glVertex2i(x + w - scroll_w + 2, thumb_y + thumb_h);
            glVertex2i(x + w            - 2, thumb_y + thumb_h);
            glVertex2i(x + w            - 2, thumb_y);
        }
        glEnd();
    }
    glPopAttrib();

    // Draw the children.

    glPushAttrib(GL_ENABLE_BIT);
    glPushMatrix();
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(x, conf->get_i("window_h") - y - h, w - scroll_w, h);

        glTranslatef(0, -child_d, 0);
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

    laydn(x, y, w, h);
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

    w = inner_w + mono->size() * 2;
    h = inner_h + mono->size() / 2;
}

void gui::finder_elt::draw(const widget *focus, const widget *input) const
{
    //Draw the background.

    leaf::draw(focus, input);

    // Draw the finder element shape.

    glBegin(GL_QUADS);
    {
        fore_color();

        glVertex2i(x,     y);
        glVertex2i(x,     y + h);
        glVertex2i(x + w, y + h);
        glVertex2i(x + w, y);
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

        w = MAX((*i)->get_w(), w);
        h = MAX((*i)->get_h(), h);
    }

    // Total width is the widest child width times the child count.

    w *= child.size();
}

void gui::harray::laydn(int x, int y, int w, int h)
{
    int n = child.size(), c = 0;

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

        w = MAX((*i)->get_w(), w);
        h = MAX((*i)->get_h(), h);
    }

    // Total height is the heighest child height times the child count.

    h *= child.size();
}

void gui::varray::laydn(int x, int y, int w, int h)
{
    int n = child.size(), c = 0;

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

        w =    ((*i)->get_w() + w);
        h = MAX((*i)->get_h(),  h);
    }
}

void gui::hgroup::laydn(int x, int y, int w, int h)
{
    int dx, c = 0, excess = w - this->w;

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

        w = MAX((*i)->get_w(),  w);
        h =    ((*i)->get_h() + h);
    }
}

void gui::vgroup::laydn(int x, int y, int w, int h)
{
    int dy, c = 0, excess = h - this->h;

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

        w = MAX((*i)->get_w(), w);
        h = MAX((*i)->get_h(), h);
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

        w = MAX((*i)->get_w(), w);
        h = MAX((*i)->get_h(), h);
    }

    w += s + s;
    h += s + s;
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
            const int x0o = x;
            const int x0i = x     + s;
            const int x1i = x + w - s;
            const int x1o = x + w;
            const int y0o = y;
            const int y0i = y     + s;
            const int y1i = y + h - s;
            const int y1o = y + h;

            glVertex2i(x0o, y0o);
            glVertex2i(x0o, y1o);
            glVertex2i(x0i, y1i);
            glVertex2i(x0i, y0i);

            glVertex2i(x1i, y0i);
            glVertex2i(x1i, y1i);
            glVertex2i(x1o, y1o);
            glVertex2i(x1o, y0o);

            glVertex2i(x0o, y0o);
            glVertex2i(x0i, y0i);
            glVertex2i(x1i, y0i);
            glVertex2i(x1o, y0o);

            glVertex2i(x0i, y1i);
            glVertex2i(x0o, y1o);
            glVertex2i(x1o, y1o);
            glVertex2i(x1i, y1i);
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
        int w = conf->get_i("window_w");
        int h = conf->get_i("window_h");

        glPushAttrib(GL_ENABLE_BIT);
        {
            glMatrixMode(GL_PROJECTION);
            {
                glLoadIdentity();
                glOrtho(0, w, h, 0, 0, 1);
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
