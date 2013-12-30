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

#ifndef WRL_PARAM_HPP
#define WRL_PARAM_HPP

#include <string>
#include <map>

#include <etc-ode.hpp>
#include <app-file.hpp>

//-----------------------------------------------------------------------------

namespace wrl
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
            soft_cfm = dParamGroup * 4 + 7,

            brightness,
            falloff,
            cutoff
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

        void load(app::node);
        void save(app::node);
    };

    typedef std::map<int, param *> param_map;
}

//-----------------------------------------------------------------------------

#endif
