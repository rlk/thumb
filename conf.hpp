#ifndef CONF_HPP
#define CONF_HPP

#include <string>
#include <mxml.h>

#define DEFAULT_CONF_FILE "conf.xml"

//-----------------------------------------------------------------------------

namespace app
{
    class conf
    {
        std::string  file;
        mxml_node_t *head;
        mxml_node_t *root;

        mxml_node_t *find(std::string, std::string);

        void init();
        bool load();
        void save();

    public:

        conf(std::string);
       ~conf();

        int         get_i(std::string);
        float       get_f(std::string);
        std::string get_s(std::string);

        void        set_i(std::string, int);
        void        set_f(std::string, float);
        void        set_s(std::string, std::string);
    };
}

//-----------------------------------------------------------------------------

#endif
