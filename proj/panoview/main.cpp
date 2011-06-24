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

void mix(int *v, int n)
{
    for (int i = 0; i < n; ++i)
    {
        int a = rand() % n;
        int b = rand() % n;
        
        if (a != b)
        {
            int t = v[a];
            v[a]  = v[b];
            v[b]  = t;
        }
    }
}

int main(int argc, char *argv[])
{
    tree<int> P;
    int i;
    int n = 20;
    
    int v[n];
    
    for (i = 0; i < n; ++i)
        v[i] = i;
    
    mix(v, n);
    for (i = 0; i < n; ++i)
    {
        P.insert(v[i]);
        P.dump();
    }

    for (i = 0; i < n; ++i)
    {
        printf("[%d]\n", P.eject());
        P.dump();
    }

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
