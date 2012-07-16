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

#include <cmath>

#include <ogl-opengl.hpp>
#include <app-host.hpp>

#include "scm-path.hpp"

//------------------------------------------------------------------------------

view_path::view_path() : head_t(0), head_d(0), curr(0), filename("path.xml")
{
}

void view_path::clear()
{
    step.clear();
    curr = 0;
}

//------------------------------------------------------------------------------

static int mmod(int n, int m)
{
    const int d = n % m;
    return (d < 0) ? d + m : d;
}

//------------------------------------------------------------------------------

void view_path::fore(bool movie)
{
    if (head_d <= 0 && !step.empty())
    {
        if (movie)
        {
            ::host->set_movie_mode(2);
            ::host->set_bench_mode(1);
        }
        head_d = +1;
    }
    else stop();
}

void view_path::back(bool movie)
{
    if (head_d >= 0 && !step.empty())
    {
        if (movie)
        {
            ::host->set_movie_mode(2);
            ::host->set_bench_mode(1);
        }
        head_d = -1;
    }
    else stop();
}

void view_path::stop()
{
    ::host->set_bench_mode(0);
    ::host->set_movie_mode(0);
    head_d = 0;
}

void view_path::next()
{
    if (!step.empty())
        curr = mmod((curr + 1), step.size());
}

void view_path::prev()
{
    if (!step.empty())
        curr = mmod((curr - 1), step.size());
}

void view_path::home()
{
    curr   = 0;
    head_t = 0;
}

void view_path::jump()
{
    head_t = curr;
}

//------------------------------------------------------------------------------

void view_path::get(view_step& s)
{
    if (!step.empty())
        s = now();
}

void view_path::set(view_step& s)
{
    if (!step.empty())
    {
        step[curr] = s;
    }
}

void view_path::ins(view_step& s)
{
    if (step.empty())
    {
        step.push_back(s);
        curr = 0;
    }
    else
    {
        step.insert(step.begin() + curr, s);
        curr++;
    }
}

void view_path::add(view_step& s)
{
    if (step.empty())
    {
        step.push_back(s);
        curr = 0;
    }
    else
    {
        step.insert(step.begin() + curr + 1, s);
        curr++;
    }
}

void view_path::del()
{
    if (!step.empty())
    {
        step.erase(step.begin() + curr);
        curr = mmod(curr - 1, step.size());
    }
}

//------------------------------------------------------------------------------

void view_path::faster()
{
    if (!step.empty())
        step[curr].set_speed(step[curr].get_speed() * 1.1);
}

void view_path::slower()
{
    if (!step.empty())
        step[curr].set_speed(step[curr].get_speed() / 1.1);
}

void view_path::inc_tens()
{
    if (!step.empty())
        step[curr].set_tension(step[curr].get_tension() + 0.1);
}

void view_path::dec_tens()
{
    if (!step.empty())
        step[curr].set_tension(step[curr].get_tension() - 0.1);
}

void view_path::inc_bias()
{
    if (!step.empty())
        step[curr].set_bias(step[curr].get_bias() + 0.1);
}

void view_path::dec_bias()
{
    if (!step.empty())
        step[curr].set_bias(step[curr].get_bias() - 0.1);
}

//------------------------------------------------------------------------------

void view_path::save()
{
    app::node head("?xml");
    app::node body("path");

    head.set_s("version", "1.0");
    head.set_s("?", "");

    body.insert(head);

    for (int i = 0; i < int(step.size()); i++)
        step[i].serialize().insert(body);

    head.write(filename);
}

void view_path::load()
{
    app::file file(filename);

    app::node p = file.get_root().find("path");
    app::node n;

    step.clear();

    for (n = p.find("step"); n; n = p.next(n, "step"))
        step.push_back(view_step(n));

    if (!step.empty())
        curr = mmod(curr, step.size());
}

//------------------------------------------------------------------------------

view_step view_path::erp(double t) const
{
    int    i = int(floor(t));
    double k = t - floor(t);

    if (step.size() == 0) return view_step();
    if (step.size() == 1) return step[0];
    if (t           == 0) return step[0];
    if (k           == 0) return step[i];

    int n = step.size() - 2;
    int m = step.size() - 1;

    view_step a = (i > 0) ? step[i - 1] : view_step(&step[0], &step[1], -1.0);
    view_step b =           step[i    ];
    view_step c =           step[i + 1];
    view_step d = (i < n) ? step[i + 2] : view_step(&step[n], &step[m], +2.0);

    return view_step(&a, &b, &c, &d, k);
}

void view_path::time(double dt)
{
    if (step.size())
    {
        head_t += head_d * dt * now().get_speed();

        if (head_t > step.size() - 1)
        {
            head_t = step.size() - 1;
            stop();
        }
        if (head_t < 0)
        {
            head_t = 0;
            stop();
        }
    }
    else stop();
}

void view_path::draw() const
{
    if (!step.empty())
    {
        view_step s;
        double   t;

        glPushAttrib(GL_ENABLE_BIT);
        {
            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glPointSize(8.0);
            glBegin(GL_POINTS);
            {
                int last = int(step.size()) - 1;

                for (int i = 0; i < int(step.size()); ++i)
                {
                    if      (i == curr) glColor4f(1.0f, 1.0f, 0.0f, 0.8f);
                    else if (i ==    0) glColor4f(0.0f, 1.0f, 0.0f, 0.8f);
                    else if (i == last) glColor4f(1.0f, 0.0f, 0.0f, 0.8f);
                    else                glColor4f(1.0f, 0.5f, 0.0f, 0.6f);

                    step[i].draw();
                }
            }
            glEnd();

            glPointSize(4.0);
            glColor4f(0.0f, 1.0f, 0.0, 1.0f);
            glBegin(GL_POINTS);
            {
                for (t = 0.0; t < step.size() - 1; t += 0.25 * s.get_speed())
                {
                    s = erp(t);
                    s.draw();
                }
            }
            glEnd();

            glLineWidth(2.0);
            glColor4f(0.0f, 1.0f, 0.0, 0.5f);
            glBegin(GL_LINE_STRIP);
            {
                for (t = 0.0; t < step.size() - 1; t += 0.25 * s.get_speed())
                {
                    s = erp(t);
                    s.draw();
                }
                step.back().draw();
            }
            glEnd();
        }
        glPopAttrib();
    }
}

//------------------------------------------------------------------------------
