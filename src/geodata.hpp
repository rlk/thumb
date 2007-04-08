#ifndef GEODATA_HPP
#define GEODATA_HPP

#include <string>

//-----------------------------------------------------------------------------

namespace ogl
{
    class geodata
    {
        std::string name;
        int         desc;

    public:

        const std::string& get_name() const { return name; }

        geodata(std::string);
       ~geodata();

        void box_bound(float *) const;
        void sph_bound(float *) const;

        void draw() const;
    };
}

#endif
