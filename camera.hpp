#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "solid.hpp"

//-----------------------------------------------------------------------------

namespace ent
{
    class camera : public free
    {
    public:

        camera();

        void proj() const;
        void pick(float[3], float[3], int, int) const;
    };
}

//-----------------------------------------------------------------------------

#endif
