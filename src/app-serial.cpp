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

/* TODO: push this catch up to where a default may be supplied.
    try
    {
*/
        if (const char *buff = (const char *) ::data->load(file))
            head = mxmlLoadString(0, buff, MXML_TEXT_CALLBACK);
/*
    }
    catch (find_error& e)
    {
    }
*/
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

app::serial::serial(const std::string& name) : file(name), head(0)
{
    load();
}

app::serial::~serial()
{
    save();

    if (head) mxmlDelete(head);
}

//-----------------------------------------------------------------------------

void app::set_attr_d(app::node elem, const std::string& name, int d)
{
    if (elem && !name.empty())
    {
        // Store the integer attribute as text.

        mxmlElementSetAttrf(elem, name.c_str(), "%d", d);

        // Touch the tree.

        for (; elem; elem = elem->parent)
            elem->user_data = (void *) 1;
    }
}

int app::get_attr_d(node elem, const std::string& name, int d)
{
    const char *c;

    if (elem && !name.empty() && (c = mxmlElementGetAttr(elem, name.c_str())))
        return atoi(c);
    else
        return d;
}

//-----------------------------------------------------------------------------

void app::set_attr_f(node elem, const std::string& name, double f)
{
    if (elem && !name.empty())
    {
        // Store the double attribute as text.

        mxmlElementSetAttrf(elem, name.c_str(), "%f", f);

        // Touch the tree.

        for (; elem; elem = elem->parent)
            elem->user_data = (void *) 1;
    }
}

double app::get_attr_f(node elem, const std::string& name, double f)
{
    const char *c;

    if (elem && !name.empty() && (c = mxmlElementGetAttr(elem, name.c_str())))
        return atof(c);
    else
        return f;
}

//-----------------------------------------------------------------------------

void app::set_attr_s(node elem, const std::string& name,
                                const std::string& s)
{
    if (elem && !name.empty())
    {
        // Store the text attribute.

        mxmlElementSetAttr(elem, name.c_str(), s.c_str());

        // Touch the tree.

        for (; elem; elem = elem->parent)
            elem->user_data = (void *) 1;
    }
}

std::string app::get_attr_s(node elem, const std::string& name,
                                       const std::string& s)
{
    const char *c;

    if (elem && !name.empty() && (c = mxmlElementGetAttr(elem, name.c_str())))
        return std::string(c);
    else
        return s;
}

//-----------------------------------------------------------------------------

app::node app::find(node tree,
                    const std::string& name,
                    const std::string& attr,
                    const std::string& valu)
{
    return mxmlFindElement(tree, tree,
                           name.empty() ? 0 : name.c_str(),
                           attr.empty() ? 0 : attr.c_str(),
                           valu.empty() ? 0 : valu.c_str(), MXML_DESCEND);
}

app::node app::next(node tree,
                    node iter,
                    const std::string& name,
                    const std::string& attr,
                    const std::string& valu)
{
    return mxmlFindElement(iter, tree,
                           name.empty() ? 0 : name.c_str(),
                           attr.empty() ? 0 : attr.c_str(),
                           valu.empty() ? 0 : valu.c_str(), MXML_NO_DESCEND);
}

//-----------------------------------------------------------------------------

app::node app::create(const std::string& tag)
{
    return mxmlNewElement(MXML_NO_PARENT, tag.c_str());
}

void app::insert(node parent, node prev, node curr)
{
    if (parent && curr)
    {
        if (prev)
            mxmlAdd(parent, MXML_ADD_AFTER, prev, curr);
        else
            mxmlAdd(parent, MXML_ADD_TO_PARENT, 0, curr);

        for (app::node elem = curr; elem; elem = elem->parent)
            elem->user_data = (void *) 1;
    }
}

void app::remove(node curr)
{
    // Touch the tree.

    for (app::node elem = curr; elem; elem = elem->parent)
        elem->user_data = (void *) 1;

    if (curr) mxmlDelete(curr);
}

//-----------------------------------------------------------------------------
