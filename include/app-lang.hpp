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

#ifndef APP_LANG_HPP
#define APP_LANG_HPP

#include <string>

#include <app-file.hpp>

//-----------------------------------------------------------------------------

namespace app
{
    //-------------------------------------------------------------------------
    // Language translation manager

    class lang
    {
        std::string language;

        app::file file;
        app::node root;

    public:

        lang(const std::string&);

        std::string get(const std::string&);
    };
}

//-----------------------------------------------------------------------------

extern app::lang *lang;

//-----------------------------------------------------------------------------

#endif
