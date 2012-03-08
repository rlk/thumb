//  Copyright (C) 2005-2012 Robert Kooima
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

#ifndef SPH_LABEL_HPP
#define SPH_LABEL_HPP

#include <string>
#include <vector>
#include <app-font.hpp>

//-----------------------------------------------------------------------------

class sph_label
{
public:

    sph_label(const std::string&);
   ~sph_label();

    void draw(const double *, double, double);

private:

    //-------------------------------------------------------------------------

    class label
    {
    public:

        label(app::font& font, const std::string& text, GLuint o, double p,
                                                                  double l)
            : str(font.render(text)), o(o), p(p), l(l) { }

        virtual ~label() { delete str; }

        virtual void   draw(const double *, double, double) = 0;
        virtual double view(const double *, double, double);

    protected:

        app::text *str;
        GLuint o;
        double p, l;
    };

    //-------------------------------------------------------------------------

    class point : public label
    {
    public:

        point(app::font& font, const std::string& text, GLuint o, double p,
                                                                  double l)
            : label(font, text, o, p, l) { }

        virtual void draw(const double *, double, double);
    };

    //-------------------------------------------------------------------------

    class circle : public label
    {
    public:

        circle(app::font& font, const std::string& text, GLuint o, double p,
                                                                   double l,
                                                                   double d)
            : label(font, text, o, p, l), d(d) { }

        virtual void draw(const double *, double, double);

    protected:

        double d;
    };

    //-------------------------------------------------------------------------

    GLint ring;
    GLint mark;

    std::vector<label *> items;
};

//-----------------------------------------------------------------------------

#endif
