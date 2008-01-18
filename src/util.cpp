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

#include <sstream>
#include <iomanip>

#include "util.hpp"

//-----------------------------------------------------------------------------

void set_attr_i(mxml_node_t *node, const char *name, int i)
{
    if (node && name)
    {
        std::ostringstream value;
    
        value << i;

        mxmlElementSetAttr(node, name, value.str().c_str());
    }
}

int get_attr_i(mxml_node_t *node, const char *name, int k)
{
    const char *c;

    if (node && (c = mxmlElementGetAttr(node, name)))
        return atoi(c);
    else
        return k;
}

//-----------------------------------------------------------------------------

void set_attr_f(mxml_node_t *node, const char *name, double k)
{
    if (node && name)
    {
        std::ostringstream value;
    
        value << std::right << std::setw(8) << std::setprecision(5) << k;

        mxmlElementSetAttr(node, name, value.str().c_str());
    }
}

double get_attr_f(mxml_node_t *node, const char *name, double k)
{
    const char *c;

    if (node && (c = mxmlElementGetAttr(node, name)))
        return atof(c);
    else
        return k;
}

//-----------------------------------------------------------------------------
