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

#ifndef USER_HPP
#define USER_HPP

//-----------------------------------------------------------------------------

namespace app
{
    class view
    {
    private:

        double      move_M[16];
        double      look_M[16];
        double transform_M[16];

        double      move_I[16];
        double      look_I[16];
        double transform_I[16];

    public:

        view();

        void go_home();

        void get_point(double *, const double *,
                       double *, const double *) const;

        const double *get_move_matrix()  const { return      move_M; }
        const double *get_look_matrix()  const { return      look_M; }
        const double *get_view_matrix()  const { return transform_M; }

        const double *get_move_inverse() const { return      move_I; }
        const double *get_look_inverse() const { return      look_I; }
        const double *get_view_inverse() const { return transform_I; }

        void set_move_matrix(const double *M);
        void set_look_matrix(const double *M);

        void load_transform() const;

//      const double *get_M() const { return current_M; }
//      const double *get_I() const { return current_I; }

        // Interactive view controls.
/*
        void turn(double, double, double, const double *);
        void turn(double, double, double);
        void move(double, double, double);
        void look(double, double);
*/

        // void set_M(const double *);

        // Transform application.

        // void draw() const;
    };
}

//-----------------------------------------------------------------------------

extern app::view *view;

//-----------------------------------------------------------------------------

#endif
