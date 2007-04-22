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

#ifndef CONF_HPP
#define CONF_HPP

#include <string>
#include <mxml.h>

#define DEFAULT_CONF_FILE "conf.xml"

//-----------------------------------------------------------------------------

namespace app
{
    class conf
    {
        std::string  file;
        mxml_node_t *head;
        mxml_node_t *root;

        mxml_node_t *find(std::string, std::string);

        bool dirty;

    public:

        conf(std::string);
       ~conf();

        void init();
        bool load();
        void save();

        int         get_i(std::string);
        float       get_f(std::string);
        std::string get_s(std::string);

        void        set_i(std::string, int);
        void        set_f(std::string, float);
        void        set_s(std::string, std::string);
    };
}

//-----------------------------------------------------------------------------

#endif
