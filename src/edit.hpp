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

#ifndef EDIT_HPP
#define EDIT_HPP

#include "mode.hpp"
#include "constraint.hpp"

//-----------------------------------------------------------------------------

namespace mode
{
    class edit : public mode
    {
        // Editor configuration.

        int key_undo;
        int key_redo;

        int key_axis_X;
        int key_axis_Y;
        int key_axis_Z;
        int key_home;

        int key_position_mode;
        int key_rotation_mode;

        int key_make_body;
        int key_make_nonbody;
        int key_make_joint;

        int key_selection_delete;
        int key_selection_invert;
        int key_selection_extend;
        int key_selection_clear;
        int key_selection_clone;

        // Edit-mode state.

        constraint transform;

        bool drag;
        bool move;

        float point_p[3];
        float point_v[3];

    public:

        edit(wrl::world&);

        virtual bool point(const float[3], const float[3], int, int);
        virtual bool click(int, bool);
        virtual bool keybd(int, bool, int);
        virtual bool timer(float);

        virtual GLfloat view(const GLfloat *);
        virtual void    draw();
    };
}

//-----------------------------------------------------------------------------

#endif
