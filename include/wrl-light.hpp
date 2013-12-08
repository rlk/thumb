//  Copyright (C) 2013 Robert Kooima
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

#ifndef WRL_LIGHT_HPP
#define WRL_LIGHT_HPP

#include <wrl-solid.hpp>

//-----------------------------------------------------------------------------

namespace wrl
{
    class light : public sphere
    {
    public:

        light(std::string);

        virtual light *clone() const { return new light(*this); }
        virtual int priority() const { return -1;               }

        virtual void save(app::node);
    };
}

//-----------------------------------------------------------------------------

#endif
