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

#ifndef VIEW_HPP
#define VIEW_HPP

//-----------------------------------------------------------------------------

namespace app
{
    class view
    {
        int   w;
        int   h;
        float n;
        float f;
        float z;

        float default_M[16];
        float current_M[16];

    public:

        view(int, int, float, float, float);

        void clr();
        void set(const float[16]);

        void turn(float, float, float);
        void move(float, float, float);

        void mult_S() const;
        void mult_P(GLfloat) const;
        void mult_O() const;
        void mult_M() const;
        void mult_R() const;
        void mult_T() const;
        void mult_V() const;

        void frust(float *) const;

        void pick(float[3], float[3], int, int) const;

        void apply(float) const;
        void push()       const;
        void pop()        const;
    };
}

//-----------------------------------------------------------------------------

extern app::view *view;

//-----------------------------------------------------------------------------

#endif
