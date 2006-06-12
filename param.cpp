#include <sstream>

#include "param.hpp"

//-----------------------------------------------------------------------------

dReal ent::param::value()
{
    dReal d;

    if (expr == "-inf") return -dInfinity;
    if (expr ==  "inf") return  dInfinity;

    std::istringstream str(expr);
    str >> d;

    return d;
}

//-----------------------------------------------------------------------------

void ent::solid_param::load(mxml_node_t *node)
{
    mxml_node_t *n;

    if ((n = mxmlFindElement(node, node, "param", "name",
                             name().c_str(), MXML_DESCEND_FIRST)))
    {
        if (n->child->value.opaque)
            expr = n->child->value.opaque;
    }
}

void ent::joint_param::load(mxml_node_t *node)
{
    std::string attr = name();

    if (axis == 1) attr += "2";
    if (axis == 2) attr += "3";

    mxml_node_t *n;;

    if ((n = mxmlFindElement(node, node, "param", "name",
                             attr.c_str(), MXML_DESCEND_FIRST)))
    {
        if (n->child->value.opaque)
            expr = n->child->value.opaque;
    }
}

//-----------------------------------------------------------------------------

void ent::solid_param::save(mxml_node_t *node)
{
    mxml_node_t *n = mxmlNewElement(node, "param");

    mxmlElementSetAttr(n, "name", name().c_str());
    mxmlNewText(n, 0, expr.c_str());
}

void ent::joint_param::save(mxml_node_t *node)
{
    std::string attr = name();

    if (axis == 1) attr += "2";
    if (axis == 2) attr += "3";

    mxml_node_t *n = mxmlNewElement(node, "param");

    mxmlElementSetAttr(n, "name", attr.c_str());
    mxmlNewText(n, 0, expr.c_str());
}

//-----------------------------------------------------------------------------
