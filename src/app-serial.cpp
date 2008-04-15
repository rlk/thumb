//  Copyright (C) 2007 Robert Kooima
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

#include <cstring>

#include <mxml.h>

#include "app-serial.hpp"
#include "app-data.hpp"

//-----------------------------------------------------------------------------

static const char *save_cb(mxml_node_t *node, int where)
{
    const char *space  = "                                                   ";
    const char *indent = space + strlen(space) - 1;

    for (mxml_node_t *curr = node->parent; curr; curr = curr->parent)
        indent -= 2;
        
    switch (where)
    {
    case MXML_WS_AFTER_OPEN:
    case MXML_WS_AFTER_CLOSE:
        return "\n";

    case MXML_WS_BEFORE_OPEN:
        return indent;

    case MXML_WS_BEFORE_CLOSE:
        if (node->child)
            return indent;
        else
            return 0;
    }
    return 0;
}

//-----------------------------------------------------------------------------

void app::serial::load()
{
    // Release any previously loaded DOM.

    if (head)
        mxmlDelete(head);

    head = 0;

    // Load the data stream and parse the XML.

    try
    {
        if (const char *buff = (const char *) ::data->load(file))
            head = mxmlLoadString(0, buff, MXML_TEXT_CALLBACK);
    }
    catch (find_error& e)
    {
    }

    // Release the data stream.

    ::data->free(file);

    // Mark the DOM as clean.

    if (head) head->user_data = 0;
}

void app::serial::save()
{
    // Confirm that the DOM exists, and has been modified.

    if (head && head->user_data)
    {
        // Write the XML to a stream and store the stream.

        if (char *buff = mxmlSaveAllocString(head, save_cb))
        {
            ::data->save(file, buff);
            free(buff);
        }
    }

    // Mark the DOM as clean.

    if (head) head->user_data = 0;
}

//-----------------------------------------------------------------------------

app::serial::serial(const char *name) : file(name), head(0)
{
    load();
}

app::serial::~serial()
{
    save();
}

//-----------------------------------------------------------------------------

void app::set_attr_d(app::node elem, const char *name, int d)
{
    if (elem && name)
    {
        // Store the integer attribute as text.

        mxmlElementSetAttrf(elem, name, "%d", d);

        // Touch the tree.

        for (; elem; elem = elem->parent)
            elem->user_data = (void *) 1;
    }
}

int app::get_attr_d(node elem, const char *name, int d)
{
    const char *c;

    if (elem && (c = mxmlElementGetAttr(elem, name)))
        return atoi(c);
    else
        return d;
}

//-----------------------------------------------------------------------------

void app::set_attr_f(node elem, const char *name, double f)
{
    if (elem && name)
    {
        // Store the double attribute as text.

        mxmlElementSetAttrf(elem, name, "%f", f);

        // Touch the tree.

        for (; elem; elem = elem->parent)
            elem->user_data = (void *) 1;
    }
}

double app::get_attr_f(node elem, const char *name, double k)
{
    const char *c;

    if (elem && (c = mxmlElementGetAttr(elem, name)))
        return atof(c);
    else
        return k;
}

//-----------------------------------------------------------------------------

void app::set_attr_s(node elem, const char *name, const char *s)
{
    if (elem && name)
    {
        // Store the text attribute.

        mxmlElementSetAttr(elem, name, s);

        // Touch the tree.

        for (; elem; elem = elem->parent)
            elem->user_data = (void *) 1;
    }
}

const char *app::get_attr_s(node elem, const char *name, const char *s)
{
    const char *c;

    if (elem && (c = mxmlElementGetAttr(elem, name)))
        return c;
    else
        return s;
}

//-----------------------------------------------------------------------------

app::node app::find(node tree,
                    const char *name,
                    const char *attr,
                    const char *valu)
{
    return mxmlFindElement(tree, tree, name, attr, valu, MXML_DESCEND);
}

app::node app::next(node tree,
                    node iter,
                    const char *name,
                    const char *attr,
                    const char *valu)
{
    return mxmlFindElement(iter, tree, name, attr, valu, MXML_NO_DESCEND);
}

//-----------------------------------------------------------------------------
