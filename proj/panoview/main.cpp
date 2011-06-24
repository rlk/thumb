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

#include <SDL.h>

#include <stdexcept>
#include <cstdio>
#include <cassert>

#include <app-prog.hpp>
#include <app-default.hpp>

#include "panoview.hpp"

//-----------------------------------------------------------------------------

struct trio
{
    trio(int i=0, int j=0, int k=0) : i(i), j(j), k(k) { }
    int i;
    int j;
    int k;
    
    bool operator<(const trio& that) const {
        if (i == that.i)
            return j < that.j;
        else
            return i < that.i;
    }
};

int main(int argc, char *argv[])
{
    tree<trio> P;

    P.insert(trio(0, 1, 9));
    P.insert(trio(0, 2, 8));
    P.insert(trio(1, 1, 99));
    P.insert(trio(1, 2, 88));

    printf("%d\n", P.search(trio(0, 2)).k);
    
    printf("%d\n", P.eject().k);
    printf("%d\n", P.eject().k);
/*
    try
    {
        app::prog *P;

        P = new panoview(std::string(argc > 1 ? argv[1] : DEFAULT_TAG));
        P->run();

        delete P;
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "Exception: %s\n", e.what());
    }
*/
    return 0;
}

//-----------------------------------------------------------------------------
