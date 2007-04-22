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

#ifndef PARAM_HPP
#define PARAM_HPP

#include <string>

#include <ode/ode.h>
#include <mxml.h>

#include "util.hpp"

//-----------------------------------------------------------------------------

namespace ent
{
    class param
    {
    public:

        // dSurfaceParam element tags.

        enum {
            category = dParamGroup * 4 + 1,
            collide  = dParamGroup * 4 + 2,
            density  = dParamGroup * 4 + 3,
            mu       = dParamGroup * 4 + 4,
            bounce   = dParamGroup * 4 + 5,
            soft_erp = dParamGroup * 4 + 6,
            soft_cfm = dParamGroup * 4 + 7
        };

    protected:

        bool   state;
        double cache;

        std::string name;
        std::string expr;

    public:

        param(std::string, std::string);

        void set(std::string& e) { expr = e; state = false; }
        void get(std::string& e) { e = expr;                }

        double value();

        void load(mxml_node_t *);
        void save(mxml_node_t *);
    };
}

//-----------------------------------------------------------------------------

#endif
