#ifndef SKY_HPP
#define SKY_HPP

#include "solid.hpp"
#include "image.hpp"
#include "data.hpp"

//-----------------------------------------------------------------------------

namespace ent
{
    class sky : public free
    {
        float           dist;
        ogl::image_file glow;
        ogl::image_file fill;
        ogl::shader     prog;

    public:

        sky(float);

        void draw_fill() const;
    };
}

//-----------------------------------------------------------------------------

#endif
