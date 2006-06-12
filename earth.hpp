#ifndef EARTH_HPP
#define EARTH_HPP

#include "solid.hpp"
#include "image.hpp"
#include "data.hpp"

//-----------------------------------------------------------------------------

namespace ent
{
    class earth : public free
    {
        float dist;

    public:

        earth(float);

        void draw_fill() const;
    };
}

//-----------------------------------------------------------------------------

#endif
