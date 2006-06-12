#ifndef LANG_HPP
#define LANG_HPP

#include <string>
#include <mxml.h>

#define DEFAULT_LANG_FILE "lang.xml"

//-----------------------------------------------------------------------------

namespace app
{
    class lang
    {
        std::string  loc;
        mxml_node_t *head;
        mxml_node_t *root;

    public:

        lang(std::string, std::string);
       ~lang();

        std::string get(std::string);
    };
}

//-----------------------------------------------------------------------------

#endif
