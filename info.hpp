#ifndef INFO_HPP
#define INFO_HPP

#include "control.hpp"
#include "image.hpp"
#include "mode.hpp"

//-----------------------------------------------------------------------------

namespace mode
{
    class info : public mode
    {
        cnt::control gui;

    public:

        info(ops::scene&);

        virtual void enter();
        virtual void leave();

        virtual bool point(const float[3], const float[3], int, int);
        virtual bool click(int, bool);
        virtual bool keybd(int, bool, int);

        virtual void draw() const;
    };
}

//-----------------------------------------------------------------------------

#endif
