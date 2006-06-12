#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "solid.hpp"

//-----------------------------------------------------------------------------

namespace ent
{
    class light : public free
    {
        GLenum name;

    public:

        light(GLenum n);

        void draw_fill() const;
    };
}

//-----------------------------------------------------------------------------

#endif
