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

#ifndef PROG_HPP
#define PROG_HPP

//-----------------------------------------------------------------------------

namespace app
{
    class prog
    {
        int key_snap;
        int key_exit;
        int key_init;

    protected:

        void snap(std::string, int, int) const;

    public:

        prog();

        virtual void timer(double)    { }
        virtual void point(int, int)  { }
        virtual void click(int, bool) { }
        virtual void keybd(int, bool, int);
        
        virtual void draw() { }

        virtual ~prog() { }
    };
}

//-----------------------------------------------------------------------------

#endif
