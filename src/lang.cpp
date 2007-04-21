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

#include "lang.hpp"
#include "main.hpp"

//-----------------------------------------------------------------------------

app::lang::lang(std::string file, std::string loc) : loc(loc)
{
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
                                            loc.c_str(), MXML_DESCEND_FIRST);

        if (text) return text->child->value.opaque;
    }

    // On failure, return the raw message.

    return message;
}

//-----------------------------------------------------------------------------
