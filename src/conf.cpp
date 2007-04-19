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

#include <cstdio>
#include <mxml.h>

#include "conf.hpp"

//-----------------------------------------------------------------------------

void app::conf::init()
{
    head = mxmlNewElement(NULL, "?xml");
    root = mxmlNewElement(head, "conf");

    mxmlElementSetAttr(head, "version",  "1.0");
    mxmlElementSetAttr(head, "?", NULL);
}

//-----------------------------------------------------------------------------

static mxml_type_t load_cb(mxml_node_t *node)
{
    std::string name(node->value.element.name);
    const char *attr;

    // Determine the MXML type for the given option.

    if ((name == "option") && (attr = mxmlElementGetAttr(node, "type")))
    {
        std::string type(attr);

        if (type == "int")    return MXML_INTEGER;
        if (type == "float")  return MXML_REAL;
        if (type == "string") return MXML_TEXT;
    }

    return MXML_ELEMENT;
}

bool app::conf::load()
{
    FILE *fp;

    if ((fp = fopen(file.c_str(), "r")))
    {
        head = mxmlLoadFile(NULL, fp, load_cb);
        root = mxmlFindElement(head, head, "conf", 0, 0, MXML_DESCEND_FIRST);

        fclose(fp);

        if (root) return true;
    }
    return false;
}

//-----------------------------------------------------------------------------

static const char *save_cb(mxml_node_t *node, int where)
{
    std::string name(node->value.element.name);

    switch (where)
    {
    case MXML_WS_AFTER_OPEN:
        if (name == "?xml") return "\n";
        if (name == "conf") return "\n";
        break;

    case MXML_WS_BEFORE_OPEN:
        if (name == "option") return "  ";
        break;

    case MXML_WS_AFTER_CLOSE:
        return "\n";
    }

    return NULL;
}

void app::conf::save()
{
    FILE *fp;

    if ((fp = fopen(file.c_str(), "w")))
    {
        mxmlSaveFile(head, fp, save_cb);
        fclose(fp);
    }
}

//-----------------------------------------------------------------------------

mxml_node_t *app::conf::find(std::string type, std::string name)
{
    mxml_node_t *node;

    // Find an element with the given type and name attributes.

    for (node = mxmlFindElement(root, root, "option", "type",
                                type.c_str(), MXML_DESCEND_FIRST);
         node;
         node = mxmlFindElement(node, root, "option", "type",
                                type.c_str(), MXML_NO_DESCEND))

        if (mxmlElementGetAttr(node, "name") == name)
            return node;

    return 0;
}

//-----------------------------------------------------------------------------

int app::conf::get_i(std::string name)
{
    mxml_node_t *node;

    if ((node = find("int", name)))
    {
        if (node->child)
            return int(node->child->value.integer);
    }
    return 0;
}

float app::conf::get_f(std::string name)
{
    mxml_node_t *node;

    if ((node = find("float", name)))
    {
        if (node->child)
            return float(node->child->value.real);
    }
    return 0;
}

std::string app::conf::get_s(std::string name)
{
    mxml_node_t *node;

    if ((node = find("string", name)))
    {
        if (node->child)
            return node->child->value.text.string;
    }
    return "";
}

//-----------------------------------------------------------------------------

void app::conf::set_i(std::string name, int value)
{
    // Set the value of an existing integer option, or create a new one.

    mxml_node_t *node;

    if ((node = find("int", name)))
        mxmlSetInteger(node->child, value);
    else
    {
        node = mxmlNewElement(root, "option");
        mxmlElementSetAttr(node, "type", "int");
        mxmlElementSetAttr(node, "name", name.c_str());
        mxmlNewInteger    (node, value);
    }
}

void app::conf::set_f(std::string name, float value)
{
    // Set the value of an existing floating point option, or create a new one.

    mxml_node_t *node;

    if ((node = find("float", name)))
        mxmlSetReal(node->child, value);
    else
    {
        node = mxmlNewElement(root, "option");
        mxmlElementSetAttr(node, "type", "float");
        mxmlElementSetAttr(node, "name", name.c_str());
        mxmlNewReal       (node, value);
    }
}

void app::conf::set_s(std::string name, std::string value)
{
    // Be careful not to feed Mini-XML a null string.

    const char *str = value.empty() ? " " : value.c_str();

    // Set the value of an existing string option, or create a new one.

    mxml_node_t *node;

    if ((node = find("string", name)))
    {
        mxmlSetText(node->child, 0, str);
    }
    else
    {
        node = mxmlNewElement(root, "option");
        mxmlElementSetAttr(node, "type", "string");
        mxmlElementSetAttr(node, "name", name.c_str());
        mxmlNewText       (node, 0, str);
    }
}

//-----------------------------------------------------------------------------

app::conf::conf(std::string file) : file(file), head(0), root(0)
{
    if (load() == false)
        init();
}

app::conf::~conf()
{
    save();
    if (head) mxmlDelete(head);
}

//-----------------------------------------------------------------------------
