#include <cstdio>

#include "lang.hpp"

//-----------------------------------------------------------------------------

app::lang::lang(std::string file, std::string loc) : loc(loc)
{
    FILE *fp;

    if ((fp = fopen(file.c_str(), "rb")))
    {
        head = mxmlLoadFile(0, fp, MXML_OPAQUE_CALLBACK);
        root = mxmlFindElement(head, head, "dict", 0, 0, MXML_DESCEND_FIRST);

        fclose(fp);
    }
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
