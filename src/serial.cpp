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


#include <sstream>
#include <iomanip>

#include <mxml.h>

#include "serial.hpp"
#include "data.hpp"

//-----------------------------------------------------------------------------

static const char *save_cb(mxml_node_t *node, int where)
{
    const char *space = "                                                    ";
    int         depth = 0;

    for (mxml_node_t *curr = node->parent; curr; curr = curr->parent)
        depth += 2;
        
    switch (where)
    {
    case MXML_WS_AFTER_OPEN:
    case MXML_WS_AFTER_CLOSE:
        return "\n";

    case MXML_WS_BEFORE_OPEN:
        return space + depth;

    case MXML_WS_BEFORE_CLOSE:
        if (node->child)
            return space + depth;
        else
            return 0;
    }
    return 0;
}

//-----------------------------------------------------------------------------

void app::serial::load()
{
    if (head) mxmlDelete(head);

    if (const char *buff = (const char *) ::data->load(file))
        head = mxmlLoadString(0, buff, MXML_TEXT_CALLBACK);
    else
        head = 0;

    ::data->free(file);

    dirty = false;
}

void app::serial::save()
{
    if (dirty)
    {
        if (char *buff = mxmlSaveAllocString(head, save_cb))
        {
            ::data->save(file, buff);
            free(buff);
        }
    }
    dirty = false;
}

//-----------------------------------------------------------------------------

app::serial::serial(const char *name) : file(name), head(0), dirty(false)
{
    load();
}

app::serial::~serial()
{
    save();
}

//-----------------------------------------------------------------------------

void app::serial::set_attr_i(node elem, const char *name, int i)
{
    if (elem && name)
    {
        std::ostringstream value;
    
        value << i;

        mxmlElementSetAttr(elem, name, value.str().c_str());
        dirty = true;
    }
}

int app::serial::get_attr_i(node elem, const char *name, int k)
{
    const char *c;

    if (elem && (c = mxmlElementGetAttr(elem, name)))
        return atoi(c);
    else
        return k;
}

//-----------------------------------------------------------------------------

void app::serial::set_attr_f(node elem, const char *name, double k)
{
    if (elem && name)
    {
        std::ostringstream value;
    
        value << k;

        mxmlElementSetAttr(elem, name, value.str().c_str());
        dirty = true;
    }
}

double app::serial::get_attr_f(node elem, const char *name, double k)
{
    const char *c;

    if (elem && (c = mxmlElementGetAttr(elem, name)))
        return atof(c);
    else
        return k;
}

//-----------------------------------------------------------------------------

void app::serial::set_attr_s(node elem, const char *name, const char *s)
{
    if (elem && name)
    {
        mxmlElementSetAttr(elem, name, s);
        dirty = true;
    }
}

const char *app::serial::get_attr_s(node elem, const char *name, const char *s)
{
    const char *c;

    if (elem && (c = mxmlElementGetAttr(elem, name)))
        return c;
    else
        return s;
}

//-----------------------------------------------------------------------------

app::node app::serial::find(node tree,
                            const char *name,
                            const char *attr,
                            const char *valu)
{
    if (tree)
        return mxmlFindElement(tree, tree, name, attr, valu, MXML_DESCEND);
    else
        return mxmlFindElement(head, head, name, attr, valu, MXML_DESCEND);
}

app::node app::serial::next(node tree,
                            node iter,
                            const char *name,
                            const char *attr,
                            const char *valu)
{
    if (tree)
        return mxmlFindElement(iter, tree, name, attr, valu, MXML_NO_DESCEND);
    else
        return mxmlFindElement(iter, head, name, attr, valu, MXML_NO_DESCEND);
}

//-----------------------------------------------------------------------------
