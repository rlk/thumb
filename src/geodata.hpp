#ifndef GEODATA_HPP
#define GEODATA_HPP

#include <string>

#include "obj.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class geodata
    {
        std::string name;
        obj::obj   *data;

    public:

        const std::string& get_name() const { return name; }

        geodata(std::string);
       ~geodata();

        void box_bound(float *) const;
        void sph_bound(float *) const;

        void draw(int=DRAW_OPAQUE) const;
        int  type(               ) const;

        void init();
        void fini();
    };
}

#endif
