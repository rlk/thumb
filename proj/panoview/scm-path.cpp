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

scm_path::scm_path() : curr(0), filename("path.dat")
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

static int clamp(int n, int m)
{
    if      (n <     0) return     0;
    else if (n > m - 1) return m - 1;
    else                return n;
}

//------------------------------------------------------------------------------
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
        s = interpolate(int(floor(head_t)), head_t - floor(head_t));
    s.write(stdout);
}

void scm_path::set(scm_step& s)
{
    if (!step.empty())
    {
        step[curr] = s;
    }
}

void scm_path::ins(scm_step& s)
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
    FILE *stream;

    if ((stream = fopen(filename.c_str(), "w")))
    {
        for (int i = 0; i < int(step.size()); ++i)
            step[i].write(stream);

        fclose(stream);
    }
}

void scm_path::load()
{
    FILE *stream;

    if ((stream = fopen(filename.c_str(), "r")))
    {
        scm_step s;

        step.clear();

        while (s.read(stream))
            step.push_back(s);

        curr = mmod(curr, step.size());
        fclose(stream);
    }
}

//------------------------------------------------------------------------------

scm_step scm_path::interpolate(size_t i, double t)
{
    if (t == 0.0 and i     < step.size()) return step[i    ];
    if (t == 1.0 and i + 1 < step.size()) return step[i + 1];

    scm_step *a = (0 <= (i - 1) && (i - 1) < step.size()) ? &step[i - 1] : NULL;
    scm_step *b = (0 <= (i    ) && (i    ) < step.size()) ? &step[i    ] : NULL;
    scm_step *c = (0 <= (i + 1) && (i + 1) < step.size()) ? &step[i + 1] : NULL;
    scm_step *d = (0 <= (i + 2) && (i + 2) < step.size()) ? &step[i + 2] : NULL;

    return scm_step(a, b, c, d, t);
}

void scm_path::time(double dt)
{
    head_t += head_d * dt;

    if (head_t > step.size() - 1)
    {
        head_t = step.size() - 1;
        head_d = 0;
    }
    if (head_t < 0)
    {
        head_t = 0;
        head_d = 0;
    }
}

void scm_path::draw()
{
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
            for (int i = 0; i < int(step.size()); ++i)
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
        glColor4f(0.0f, 1.0f, 0.0, 1.0f);
        glBegin(GL_POINTS);
        {
            if (step.size())
                interpolate(int(floor(head_t)), head_t - floor(head_t)).draw();
        }
        glEnd();

        glBegin(GL_LINE_STRIP);
        {
            for (int i = 0; i < int(step.size()); ++i)
                for (int j = 0; j < 32; ++j)
                {
                    double t = double(j) / 32;

                    glColor4f(0.0f, 1.0f, 0.0f, 0.8f * t + 0.2f);

                    interpolate(i, t).draw();
                }
        }
        glEnd();
    }
    glPopAttrib();
}

//------------------------------------------------------------------------------
