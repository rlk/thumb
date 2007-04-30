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

#ifndef LANG_HPP
#define LANG_HPP

#include <string>
#include <mxml.h>

#define DEFAULT_LANG_FILE "lang.xml"

//-----------------------------------------------------------------------------

namespace app
{
    //-------------------------------------------------------------------------
    // Language translation manager

    class lang
    {
        std::string  curr;
        mxml_node_t *head;
        mxml_node_t *root;

    public:

        lang(std::string);
       ~lang();

        std::string get(std::string);
    };
}

//-----------------------------------------------------------------------------

extern app::lang *lang;

//-----------------------------------------------------------------------------

#endif
