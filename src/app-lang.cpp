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

#include "app-data.hpp"
#include "app-conf.hpp"
#include "app-lang.hpp"

//-----------------------------------------------------------------------------

static std::string get_language()
{
    // Check for a language setting in the config file.

    std::string loc = ::conf->get_s("lang");

    if (!loc.empty())
        return loc;

    // Check for a language setting in the environment.

    const char *env = getenv("LANG");

    if (env) return env;

    // If all else fails, use the default.

    return "en_US";
}    

//-----------------------------------------------------------------------------

app::lang::lang(const std::string& name) :
    language(get_language()),
    file(name),
    root(file.get_root().find("dict"))
{
}

//-----------------------------------------------------------------------------

std::string app::lang::get(const std::string& message)
{
    // Find the requested message element.

    if (app::node n = root.find("message", "tag", message))
    {
        // Find the current language element.

        if (app::node c = n.find("text", "lang", language))
            return c.get_s();
    }

    // On failure, return the raw message.

    return message;
}

//-----------------------------------------------------------------------------
