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

#ifndef GUI_GUI_HPP
#define GUI_GUI_HPP

#include <vector>

#include <thumb.hpp>
#include <app-font.hpp>
#include <etc-rect.hpp>

//-----------------------------------------------------------------------------

namespace gui
{
    //-------------------------------------------------------------------------
    // Basic widget.

    class widget
    {
    protected:

        rect area;

        bool is_enabled;
        bool is_pressed;

        std::vector<widget *> child;

        void back_color(const widget *) const;
        void fore_color()               const;

    public:

        widget();

        widget *add(widget *w) { child.push_back(w); return this; }

        // Layout state.

        virtual bool exp_w() const { return false;  }
        virtual bool exp_h() const { return false;  }
        int          get_w() const { return area.w; }
        int          get_h() const { return area.h; }

        // Current value access and activation.

        virtual void        value(std::string);
        virtual std::string value() const;
        virtual void        apply() { };

        // Event handlers.

        virtual void    layup() { };
        virtual void    laydn(int, int, int, int);
        virtual widget *click(int, int, int, bool);
        virtual widget *enter(int, int)      { return 0; }
        virtual void    point(int, int)      {           }
        virtual void    key  (int, int)      {           }
        virtual void    glyph(int)           {           }

        bool pressed() const { return is_pressed; }

        // Display handlers.

        virtual void show() { }
        virtual void hide() { }
        virtual void draw(const widget *, const widget *) const { }

        virtual ~widget();
    };

    typedef std::vector<widget *>                 widget_v;
    typedef std::vector<widget *>::iterator       widget_i;
    typedef std::vector<widget *>::const_iterator widget_c;

    //-------------------------------------------------------------------------
    // Tree definition widgets.

    class leaf : public widget
    {
    public:
        virtual widget *enter(int, int);

        virtual void draw(const widget *, const widget *) const;
    };

    class tree : public widget
    {
    public:
        virtual bool exp_w() const;
        virtual bool exp_h() const;

        virtual widget *enter(int, int);

        virtual void show();
        virtual void hide();
        virtual void draw(const widget *, const widget *) const;

        virtual ~tree();
    };

    //-------------------------------------------------------------------------
    // Basic text string.

    class string : public leaf
    {
        static app::font *sans_font;
        static app::font *mono_font;

        static int count;

        static void init_font();
        static void free_font();

    protected:

        app::text *text;
        app::font *font;

        int just;

        std::string str;
        GLubyte color[3];

        virtual void init_text();
        virtual void draw_text() const;
        virtual void just_text();

    public:

        static const int sans = 0;
        static const int mono = 1;

        string(std::string, int f=0, int j=0, GLubyte=0xFF,
                                              GLubyte=0xFF,
                                              GLubyte=0xFF);

        virtual void        value(std::string);
        virtual std::string value() const;

        virtual void laydn(int, int, int, int);
        virtual void draw(const widget *, const widget *) const;

        virtual ~string();
    };

    //-------------------------------------------------------------------------
    // Pushbutton widget.

    class button : public string
    {
        int border;

    public:

        button(std::string, int=0, int=0, int=2);

        virtual widget *click(int, int, int, bool);
        virtual void draw(const widget *, const widget *) const;
    };

    //-------------------------------------------------------------------------
    // String input widget.

    class input : public string
    {
    protected:

        bool is_changed;

    public:

        input(std::string, int=0, int=0);

        virtual void hide();
    };

    //-------------------------------------------------------------------------
    // Bitmap widget.

    class bitmap : public input
    {
    protected:

        unsigned int bits;

    public:

        bitmap();

        widget *click(int, int, int, bool);
        void draw(const widget *, const widget *) const;
    };

    //-------------------------------------------------------------------------
    // Editable text field.

    class editor : public input
    {
        static std::string clip;

        int si;
        int sc;

        void grow_select(int);
        void move_select(int);

    protected:

        virtual void update();

    public:

        editor(std::string);

        virtual bool exp_w() const { return true;  }
        virtual bool exp_h() const { return false; }

        virtual widget *click(int, int, int, bool);
        virtual void    point(int, int);
        virtual void    key  (int, int);
        virtual void    glyph(int);

        virtual void draw(const widget *, const widget *) const;
    };

    //-------------------------------------------------------------------------
    // Vertically scrolling panel.

    class scroll : public tree
    {
    protected:

        int child_d;
        int child_h;
        int scroll_w;

    public:

        scroll() : child_d(0), child_h(0), scroll_w(32) { is_enabled = true; }

        virtual bool exp_w() const { return true; }
        virtual bool exp_h() const { return true; }

        virtual void    layup();
        virtual void    laydn(int, int, int, int);
        virtual widget *click(int, int, int, bool);
        virtual widget *enter(int, int);
        virtual void    point(int, int);

        virtual void draw(const widget *, const widget *) const;
    };

    //-------------------------------------------------------------------------
    // Expandable space filler.

    class filler : public leaf
    {
        bool h;
        bool v;

    public:
        filler(bool h=true, bool v=true) : h(h), v(v) { }

        virtual bool exp_w() const { return h; }
        virtual bool exp_h() const { return v; }
    };

    //-------------------------------------------------------------------------
    // Invisible spacer.

    class spacer : public leaf
    {
    public:
        spacer() { area.w = area.h = 4; }
        virtual void draw(const widget *, const widget *) const { }
    };

    //-------------------------------------------------------------------------
    // Grouping widgets.

    class harray : public tree
    {
    public:
        virtual void layup();
        virtual void laydn(int, int, int, int);
    };

    class varray : public tree
    {
    public:
        virtual void layup();
        virtual void laydn(int, int, int, int);
    };

    class hgroup : public tree
    {
    public:
        virtual void layup();
        virtual void laydn(int, int, int, int);
    };

    class vgroup : public tree
    {
    public:
        virtual void layup();
        virtual void laydn(int, int, int, int);
    };

    //-------------------------------------------------------------------------
    // Tab selector.

    class option : public tree
    {
        int index;

    public:
        option() : index(0) { }

        void set_index(int i)  { index = i;    }
        int  get_index() const { return index; }

        virtual void    layup();
        virtual void    laydn(int, int, int, int);
        virtual widget *enter(int, int);

        virtual void draw(const widget *, const widget *) const;
    };

    //-------------------------------------------------------------------------
    // Padding frame.

    class frame : public tree
    {
        int border;

    public:
        frame() : border(2) { }

        virtual void layup();
        virtual void laydn(int, int, int, int);

        virtual void draw(const widget *, const widget *) const;
    };

    //-------------------------------------------------------------------------
    // Text file viewer

    class pager : public varray
    {
    public:
        pager(std::string);
    };

    class pager_line : public string
    {
    public:
        pager_line(std::string, GLubyte, GLubyte, GLubyte);
    };

    //-------------------------------------------------------------------------
    // File selector

    class finder;

    class selector : public vgroup
    {
    protected:

        editor *D;
        editor *R;
        finder *F;

    public:

        selector(std::string, std::string);

        virtual std::string value() const;

        virtual void mov_dir(std::string);
        virtual void set_dir(std::string);
        virtual void set_reg(std::string);

        virtual void show();

        virtual ~selector();
    };

    class selector_val : public editor
    {
    protected:
        selector *target;
    public:
        selector_val(std::string t, gui::selector *s) : editor(t), target(s) { }
    };

    class selector_dir : public selector_val
    {
    public:
        selector_dir(std::string t, gui::selector *s) : selector_val(t, s) { }
        void apply() { target->set_dir(str); }
    };

    class selector_reg : public selector_val
    {
    public:
        selector_reg(std::string t, gui::selector *s) : selector_val(t, s) { }
        void apply() { target->set_reg(str); }
    };

    //-------------------------------------------------------------------------
    // File selection list.

    class finder : public scroll
    {
    protected:
        selector *target;
        std::string ext;

    public:
        finder(gui::selector *s, std::string e) : target(s), ext(e) { }
        void value(std::string);
    };

    class finder_elt : public button
    {
    protected:
        selector *target;
    public:
        finder_elt(std::string t, gui::selector *s);
    };

    class finder_dir : public finder_elt
    {
    public:
        finder_dir(std::string t, gui::selector *s, bool e=true);
        void apply() { target->mov_dir(str); }
    };

    class finder_reg : public finder_elt
    {
    public:
        finder_reg(std::string t, gui::selector *s, bool e=true);
        void apply() { target->set_reg(str); }
    };

    //-------------------------------------------------------------------------
    // Top level dialog

    class dialog
    {
    protected:

        widget *root;
        widget *focus;
        widget *input;

        int last_x;
        int last_y;

    public:

        dialog();

        void point(int, int);
        void click(int, bool);
        void key  (int, int);
        void glyph(int);

        void show();
        void hide();
        void draw() const;

        int get_last_x() const { return last_x; }
        int get_last_y() const { return last_y; }

        virtual ~dialog();
    };
}

//-----------------------------------------------------------------------------

#endif
