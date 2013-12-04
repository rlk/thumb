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

#include <etc-vector.hpp>
#include <ogl-opengl.hpp>
#include <ogl-pool.hpp>
#include <app-view.hpp>
#include <app-glob.hpp>
#include <app-frustum.hpp>
#include <wrl-constraint.hpp>

//-----------------------------------------------------------------------------

wrl::constraint::constraint() : mode(0), axis(1), grid(3)
{
    pool = ::glob->new_pool();

    for (int i = 0; i < 10; ++i)
    {
        pool->add_node(rot[i] = new ogl::node());
        pool->add_node(pos[i] = new ogl::node());
    }

    rot[0]->add_unit(new ogl::unit("wire/constraint_rot_0.obj"));
    rot[0]->add_unit(new ogl::unit("wire/constraint_rot_0.obj"));
    rot[1]->add_unit(new ogl::unit("wire/constraint_rot_1.obj"));
    rot[2]->add_unit(new ogl::unit("wire/constraint_rot_2.obj"));
    rot[3]->add_unit(new ogl::unit("wire/constraint_rot_3.obj"));
    rot[4]->add_unit(new ogl::unit("wire/constraint_rot_4.obj"));
    rot[5]->add_unit(new ogl::unit("wire/constraint_rot_5.obj"));
    rot[6]->add_unit(new ogl::unit("wire/constraint_rot_6.obj"));
    rot[7]->add_unit(new ogl::unit("wire/constraint_rot_7.obj"));
    rot[8]->add_unit(new ogl::unit("wire/constraint_rot_8.obj"));
    rot[9]->add_unit(new ogl::unit("wire/constraint_rot_9.obj"));

    pos[0]->add_unit(new ogl::unit("wire/constraint_pos_0.obj"));
    pos[1]->add_unit(new ogl::unit("wire/constraint_pos_1.obj"));
    pos[2]->add_unit(new ogl::unit("wire/constraint_pos_2.obj"));
    pos[3]->add_unit(new ogl::unit("wire/constraint_pos_3.obj"));
    pos[4]->add_unit(new ogl::unit("wire/constraint_pos_4.obj"));
    pos[5]->add_unit(new ogl::unit("wire/constraint_pos_5.obj"));
    pos[6]->add_unit(new ogl::unit("wire/constraint_pos_6.obj"));
    pos[7]->add_unit(new ogl::unit("wire/constraint_pos_7.obj"));
    pos[8]->add_unit(new ogl::unit("wire/constraint_pos_8.obj"));
    pos[9]->add_unit(new ogl::unit("wire/constraint_pos_9.obj"));

    set_grid(3);

    orient();
}

wrl::constraint::~constraint()
{
    ::glob->free_pool(pool);
}

//-----------------------------------------------------------------------------

void wrl::constraint::orient()
{
    switch (axis)
    {
    case 0:
        T[0][0] = -M[0][2]; T[0][1] =  M[0][1]; T[0][2] =  M[0][0];
        T[1][0] = -M[1][2]; T[1][1] =  M[1][1]; T[1][2] =  M[1][0];
        T[2][0] = -M[2][2]; T[2][1] =  M[2][1]; T[2][2] =  M[2][0];
        break;
    case 1:
        T[0][0] =  M[0][0]; T[0][1] = -M[0][2]; T[0][2] =  M[0][1];
        T[1][0] =  M[1][0]; T[1][1] = -M[1][2]; T[1][2] =  M[1][1];
        T[2][0] =  M[2][0]; T[2][1] = -M[2][2]; T[2][2] =  M[2][1];
        break;
    case 2:
        T[0][0] =  M[0][0]; T[0][1] =  M[0][1]; T[0][2] =  M[0][2];
        T[1][0] =  M[1][0]; T[1][1] =  M[1][1]; T[1][2] =  M[1][2];
        T[2][0] =  M[2][0]; T[2][1] =  M[2][1]; T[2][2] =  M[2][2];
        break;
    }

    T[0][3] = M[0][3];
    T[1][3] = M[1][3];
    T[2][3] = M[2][3];

    for (int i = 0; i < 10; ++i)
    {
        rot[i]->transform(T);
        pos[i]->transform(T);
    }
}

void wrl::constraint::set_grid(int g)
{
    static const int a[] = {
         1,
         3,
         5,
        10,
        15,
        20,
        30,
        45,
        60,
        90,
    };
    static const double d[] = {
         0.0625,
         0.1250,
         0.2500,
         0.5000,
         1.0000,
         2.0000,
         4.0000,
         8.0000,
        16.0000,
        32.0000,
    };

    grid = g;
    grid = std::max(grid, 0);
    grid = std::min(grid, 9);

    grid_a = a[grid];
    grid_d = d[grid];

    set_mode(mode);
}

void wrl::constraint::set_mode(int m)
{
    mode = m;
}

void wrl::constraint::set_axis(int a)
{
    axis = a;
    orient();
}

void wrl::constraint::set_transform(const mat4& A)
{
    M = A;
    orient();
}

//-----------------------------------------------------------------------------

static double snap(double f, double d)
{
    double f0 = floor(f / d) * d;
    double f1 = ceil (f / d) * d;

    return (fabs(f0 - f) < fabs(f1 - f)) ? f0 : f1;
}

void wrl::constraint::calc_rot(double& a, double& d, const vec3& p,
                                                     const vec3& v) const
{
    double t = (wvector(T) * zvector(T) - p * zvector(T)) / (v * zvector(T));
    vec3 q = p + v * t - wvector(T);

    double aa = to_degrees(atan2(q * yvector(T), q * xvector(T)));
    double dd = length(q);

    a = snap(aa, double(grid_a));
    d = snap(dd,       (grid_d));
}

void wrl::constraint::calc_pos(double& x, double& y, const vec3& p,
                                                     const vec3& v) const
{
    double t = (wvector(T) * zvector(T) - p * zvector(T)) / (v * zvector(T));
    vec3 q = p + v * t - wvector(T);

    double xx = q * xvector(T);
    double yy = q * yvector(T);

    x = snap(xx, grid_d);
    y = snap(yy, grid_d);
}

//-----------------------------------------------------------------------------

bool wrl::constraint::point(const vec3& p, const vec3& v, mat4& A)
{
    if (to_degrees(acos(mouse_v * v)) > 1.0)
    {
        if (mode)
        {
            double a;
            double d;

            calc_rot(a, d, p, v);

            if (fabs(a - mouse_a) > 0.0 || fabs(d - mouse_d) > 0.0)
            {
                A = translation( wvector(T))
                  *    rotation( zvector(T), to_radians(a - mouse_a))
                  * translation(-wvector(T));

                return true;
            }
        }
        else
        {
            double x;
            double y;

            calc_pos(x, y, p, v);

            if (fabs(x - mouse_x) > 0.0 || fabs(y - mouse_y) > 0.0)
            {
                A = translation(xvector(T) * (x - mouse_x)
                              + yvector(T) * (y - mouse_y));
                return true;
            }
        }
    }
    return false;
}

void wrl::constraint::click(const vec3& p, const vec3& v)
{
    mouse_p = p;
    mouse_v = v;
    calc_rot(mouse_a, mouse_d, p, v);
    calc_pos(mouse_x, mouse_y, p, v);
}

//-----------------------------------------------------------------------------

ogl::range wrl::constraint::prep(int frusc, const app::frustum *const *frusv)
{
    ogl::range r;

    // Prep the geometry pool.

    pool->prep();

    // Cache the visibility and determine the far plane distance.

    for (int frusi = 0; frusi < frusc; ++frusi)
        if (mode)
            r.merge(rot[grid]->view(frusi, frusv[frusi]->get_planes(), 5));
        else
            r.merge(pos[grid]->view(frusi, frusv[frusi]->get_planes(), 5));

    return r;
}

void wrl::constraint::draw(int frusi)
{
    // Draw the oriented constraint grid.

    ogl::line_state_init();
    {
        glPushMatrix();
        {
            glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

            glLineWidth(2.0f);

            pool->draw_init();
            {
                if (mode)
                    rot[grid]->draw(frusi, true, false);
                else
                    pos[grid]->draw(frusi, true, false);
            }
            pool->draw_fini();

            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        }
        glPopMatrix();
    }
    ogl::line_state_fini();
}

//-----------------------------------------------------------------------------

