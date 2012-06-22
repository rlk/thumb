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

#include "scm-path.hpp"

//------------------------------------------------------------------------------

scm_path::scm_path() : curr(0)
{
}

void scm_path::clear()
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

void scm_path::fore(bool stop)
{
    if (head_d <= 0 && !step.empty())
        head_d = +1;
    else
        head_d =  0;
}

void scm_path::back(bool stop)
{
    if (head_d >= 0 && !step.empty())
        head_d = -1;
    else
        head_d =  0;
}

void scm_path::next()
{
    if (!step.empty())
        curr = mmod((curr + 1), step.size());
}

void scm_path::prev()
{
    if (!step.empty())
        curr = mmod((curr - 1), step.size());
}

void scm_path::home()
{
}

void scm_path::jump()
{
    head_t = curr;
}

//------------------------------------------------------------------------------

void scm_path::get(scm_step& s)
{
    if (!step.empty())
    {
        int    i = floor(head_t);
        double t = head_t - i;

        s = scm_step(step[mmod(i - 1, step.size())],
                     step[mmod(i,     step.size())],
                     step[mmod(i + 1, step.size())],
                     step[mmod(i + 2, step.size())], t);
    }
}

void scm_path::put(scm_step& s)
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

void scm_path::del()
{
    if (!step.empty())
    {
        step.erase(step.begin() + curr);
        curr = mmod(curr - 1, step.size());
    }
}

//------------------------------------------------------------------------------

void scm_path::save()
{
}

void scm_path::load()
{
}

//------------------------------------------------------------------------------

void scm_path::time(double dt)
{
    head_t += head_d * dt;
}

void scm_path::draw()
{
    double t;
    int    i;

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
            for (i = 0; i < int(step.size()); ++i)
            {
                if (i == curr)
                    glColor4f(1.0f, 1.0f, 0.0f, 0.8f);
                else
                    glColor4f(1.0f, 0.5f, 0.0f, 0.6f);

                step[i].draw();
            }
        }
        glEnd();

        glPointSize(4.0);
        glBegin(GL_POINTS);
        {
            if (step.size())
            {
                i = floor(head_t);
                t = head_t - i;

                glColor4f(0.0f, 1.0f, 0.0, 1.0f);

                scm_step s = scm_step(step[mmod(i - 1, step.size())],
                                      step[mmod(i,     step.size())],
                                      step[mmod(i + 1, step.size())],
                                      step[mmod(i + 2, step.size())], t);
                s.draw();
            }
        }
        glEnd();

        glBegin(GL_LINE_LOOP);
        {
            for (int i = 0; i < int(step.size()); ++i)
                for (int j = 0; j < 16; ++j)
                {
                    t = (double) j / 16;

                    glColor4f(0.0f, 1.0f, 0.0f, 0.8f * t + 0.2f);

                    scm_step s = scm_step(step[mmod(i - 1, step.size())],
                                          step[mmod(i,     step.size())],
                                          step[mmod(i + 1, step.size())],
                                          step[mmod(i + 2, step.size())], t);
                    s.draw();
                }
        }
        glEnd();
    }
    glPopAttrib();
}

//------------------------------------------------------------------------------
