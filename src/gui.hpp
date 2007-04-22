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

#ifndef GUI_HPP
#define GUI_HPP

#include <vector>

#include "font.hpp"
#include "rect.hpp"

//-----------------------------------------------------------------------------

namespace gui
{
    //-------------------------------------------------------------------------
    // Basic widget.

    class widget
    {
    protected:

        rect area;
        int  just;

        bool is_enabled;
        bool is_pressed;
        bool is_invalid;
        bool is_varied;

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
        virtual widget *click(int, int, bool);
        virtual widget *enter(int, int) { return 0; }
        virtual void    point(int, int) {           }
        virtual void    keybd(int, int) {           }

        bool pressed() const { return is_pressed; }

        // Display handlers.

        virtual void show() { }
        virtual void hide() { }
        virtual void draw(const widget *, const widget *) const { }

        virtual ~widget();
    };

    typedef std::vector<widget *> widget_v;

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
    protected:

        app::text *text;
        app::font *font;

        std::string str;
        GLubyte color[3];

        void text_color() const;

        virtual void dotext() const;
        virtual void update();

    public:

        string(app::font *, std::string, int j=0,
               GLubyte=0xFF, GLubyte=0xFF, GLubyte=0xFF);

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
    public:

        button(std::string);

        virtual widget *click(int, int, bool);
        virtual void draw(const widget *, const widget *) const;
    };

    //-------------------------------------------------------------------------
    // Bitmap widget.

    class bitmap : public string
    {
    protected:

        unsigned int bits;

    public:

        bitmap();

        widget *click(int, int, bool);
        void draw(const widget *, const widget *) const;
    };

    //-------------------------------------------------------------------------
    // Editable text field.

    class editor : public string
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

        virtual widget *click(int, int, bool);
        virtual void    point(int, int);
        virtual void    keybd(int, int);

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
        virtual widget *click(int, int, bool);
        virtual widget *enter(int, int);
        virtual void    point(int, int);

        virtual void draw(const widget *, const widget *) const;
    };

    //-------------------------------------------------------------------------
    // File selection list.

    class finder : public scroll
    {
        std::string  key;
        std::string  file;
        std::string  ext;
        std::string  cwd;
        gui::widget *state;

    public:

        finder(std::string k, std::string e, gui::widget *w) :
            key(k), ext(e), cwd(conf->get_s(k)), state(w) { enlist(); }

        virtual std::string value() const { return file; }

        void set_dir(std::string&);
        void set_reg(std::string&);
        void set_dup();

        void enlist();
        void update();

        virtual void show();
    };

    class finder_elt : public string
    {
    protected:

        finder *state;

    public:

        finder_elt(std::string t, gui::finder *w);

        virtual void draw(const widget *, const widget *) const;
        virtual widget *click(int, int, bool);
    };

    class finder_dir : public finder_elt
    {
    public:
        finder_dir(std::string t, gui::finder *w) : finder_elt(t, w) { }
        void apply() { state->set_dir(str); }
    };

    class finder_reg : public finder_elt
    {
    public:
        finder_reg(std::string t, gui::finder *w) : finder_elt(t, w) { }
        void apply() { state->set_reg(str); }
    };

    class findup : public button
    {
    protected:

        finder *state;

    public:

        findup(gui::finder *w) : button("Up"), state(w) { }
        void apply() { state->set_dup(); }
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
        
        void select(int i) { index = i; }

        virtual void    layup();
        virtual void    laydn(int, int, int, int);
        virtual widget *enter(int, int);

        virtual void draw(const widget *, const widget *) const;
    };

    //-------------------------------------------------------------------------
    // Padding frame.

    class frame : public tree
    {
        int s;

    public:
        frame() : s(2) { }
        
        virtual void layup();
        virtual void laydn(int, int, int, int);

        virtual void draw(const widget *, const widget *) const;
    };
}

//-----------------------------------------------------------------------------

namespace gui
{
    //-------------------------------------------------------------------------
    // Top level dialog.

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
        void click(bool);
        void keybd(int, int);
    
        void show();
        void hide();
        void draw() const;

        virtual ~dialog();
    };
}

//-----------------------------------------------------------------------------

#endif
