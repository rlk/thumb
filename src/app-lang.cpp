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

    if (loc.size() > 0)
        return loc;

    // Check for a language setting in the environment.

    const char *env = getenv("LANG");

    if (env) return env;

    // If all else fails, use the default.

    return "en_US";
}    

//-----------------------------------------------------------------------------

app::lang::lang(std::string file)
{
    curr = get_language();

    const char *buff;

    if ((buff = (const char *) ::data->load(file)))
    {
        head = mxmlLoadString(0, buff, MXML_OPAQUE_CALLBACK);
        root = mxmlFindElement(head, head, "dict", 0, 0, MXML_DESCEND_FIRST);
    }

    ::data->free(file);
}

app::lang::~lang()
{
    if (head) mxmlDelete(head);
}

//-----------------------------------------------------------------------------

std::string app::lang::get(std::string message)
{
    // Find the requested message element.

    mxml_node_t *mesg = mxmlFindElement(root, root, "message", "tag",
                                        message.c_str(), MXML_DESCEND_FIRST);
    if (mesg)
    {
        // Find the current language element.

        mxml_node_t *text = mxmlFindElement(mesg, mesg, "text", "lang",
                                            curr.c_str(), MXML_DESCEND_FIRST);

        if (text) return text->child->value.opaque;
    }

    // On failure, return the raw message.

    return message;
}

//-----------------------------------------------------------------------------
