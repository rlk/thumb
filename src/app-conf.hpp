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

#ifndef APP_CONF_HPP
#define APP_CONF_HPP

#include "app-serial.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    class conf
    {
        app::file file;
        app::node root;

        // Locate the named option, or return a null node.

        app::node locate(const std::string& name) const {
            return root.find("option", "name", name);
        }

        // Locate the named option, or create one if need be.

        app::node create(const std::string& key) {
            if (app::node n = locate(key))
                return n;
            else
            {
                app::node c("option");
                c.insert(root);
                c.set_s("name", key);
                return c;
            }
        }

    public:

        conf(const std::string& name) :
            file(name),
            root(file.get_root().find("conf")) { }

        // Get options.

        int         get_i(const std::string& key, int    val = 0) const {
            return locate(key).get_i(val);
        }
        double      get_f(const std::string& key, double val = 0) const {
            return locate(key).get_f(val);
        }
        std::string get_s(const std::string& key) const {
            return locate(key).get_s();
        }

        // Set options.

        void        set_i(const std::string& key, int    val) {
            create(key).set_i(val);
        }
        void        set_f(const std::string& key, double val) {
            create(key).set_f(val);
        }
        void        set_s(const std::string& key, const std::string& val) {
            create(key).set_s(val);
        }
    };
}

//-----------------------------------------------------------------------------

extern app::conf *conf;

//-----------------------------------------------------------------------------
#endif
