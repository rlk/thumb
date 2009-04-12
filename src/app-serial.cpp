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
    const char *space  = "                                                   "
                         "                                                   "
                         "                                                   ";

    // Find a string giving the proper level of indentation for this node.

    const char *indent = space + strlen(space) - 1;

    for (mxml_node_t *curr = node->parent; curr; curr = curr->parent)
        indent = std::max(space, indent - 2);

    // Return the proper whitespace for location of this node.

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

app::node::node(const std::string& tag) :
    ptr(mxmlNewElement(MXML_NO_PARENT, tag.c_str()))
{
}

app::node::node(mxml_node_t *p) :
    ptr(p)
{
}

//-----------------------------------------------------------------------------

void app::node::dirty()
{
    for (mxml_node_t *itr = ptr; itr; itr = itr->parent)
        itr->user_data = (void *) 1;
}

void app::node::clean()
{
    if (ptr)
        ptr->user_data = (void *) 0;
}

//-----------------------------------------------------------------------------

void app::node::read(const std::string& name)
{
    // Release any previously loaded DOM.

    if (ptr) delete ptr;

    ptr = 0;

    // Load the data stream and parse the XML.

    if (const char *buff = (const char *) ::data->load(name))
    {
        ptr = mxmlLoadString(0, buff, MXML_TEXT_CALLBACK);
        clean();
    }
        
    // Release the data stream.

    ::data->free(name);
}

void app::node::write(const std::string& name)
{
    // If dirty, write the XML to a stream and store the stream.

    if (ptr && ptr->user_data)
    {
        if (char *buff = mxmlSaveAllocString(ptr, save_cb))
        {
            ::data->save(name, buff);
            free(buff);
        }
        clean();
    }
}

//-----------------------------------------------------------------------------

void app::node::set_i(const std::string& name, int i)
{
    if (ptr && !name.empty())
    {
        // Store the integer attribute as text.

        mxmlElementSetAttrf(ptr, name.c_str(), "%d", i);
        dirty();
    }
}

int app::node::get_i(const std::string& name, int i)
{
    const char *c;

    if (ptr && !name.empty() && (c = mxmlElementGetAttr(ptr, name.c_str())))
        return atoi(c);
    else
        return i;
}

//-----------------------------------------------------------------------------

void app::node::set_f(const std::string& name, double f)
{
    if (ptr && !name.empty())
    {
        // Store the double attribute as text.

        mxmlElementSetAttrf(ptr, name.c_str(), "%f", f);
        dirty();
    }
}

double app::node::get_f(const std::string& name, double f)
{
    const char *c;

    if (ptr && !name.empty() && (c = mxmlElementGetAttr(ptr, name.c_str())))
        return atof(c);
    else
        return f;
}

//-----------------------------------------------------------------------------

void app::node::set_s(const std::string& name,
                      const std::string& s)
{
    if (ptr && !name.empty())
    {
        // Store the text attribute.

        mxmlElementSetAttr(ptr, name.c_str(), s.c_str());
        dirty();
    }
}

std::string app::node::get_s(const std::string& name,
                             const std::string& s)
{
    const char *c;

    if (ptr && !name.empty() && (c = mxmlElementGetAttr(ptr, name.c_str())))
        return std::string(c);
    else
        return s;
}

//-----------------------------------------------------------------------------

app::node app::node::find(const std::string& name,
                          const std::string& attr,
                          const std::string& valu)
{
    if (ptr)
        return mxmlFindElement(ptr, ptr,
                               name.empty() ? 0 : name.c_str(),
                               attr.empty() ? 0 : attr.c_str(),
                               valu.empty() ? 0 : valu.c_str(),
                               MXML_DESCEND);
    else
        return 0;
}

app::node app::node::next(app::node itr,
                          const std::string& name,
                          const std::string& attr,
                          const std::string& valu)
{
    if (ptr && itr.ptr)
        return mxmlFindElement(itr.ptr, ptr,
                               name.empty() ? 0 : name.c_str(),
                               attr.empty() ? 0 : attr.c_str(),
                               valu.empty() ? 0 : valu.c_str(),
                               MXML_NO_DESCEND);
    else
        return 0;
}

//-----------------------------------------------------------------------------

void app::node::insert(app::node parent,
                       app::node after)
{
    if (parent.ptr && ptr)
    {
        mxmlAdd(parent.ptr, MXML_ADD_AFTER, after.ptr, ptr);
        dirty();
    }
}

void app::node::remove()
{
    if (ptr)
    {
        dirty();
        mxmlDelete(ptr);
    }
}

//-----------------------------------------------------------------------------

app::file::file(const std::string& name) :
    name(name),
    head(0)
{
    head.read(name);
}

app::file::~file()
{
    head.write(name);
}

//-----------------------------------------------------------------------------
