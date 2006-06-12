#ifndef PLAY_HPP
#define PLAY_HPP

#include "mode.hpp"

//-----------------------------------------------------------------------------

namespace mode
{
    class play : public mode
    {
    public:

        play(ops::scene&);

        virtual void enter();
        virtual void leave();

        virtual bool timer(float);
    };
}

//-----------------------------------------------------------------------------

#endif
