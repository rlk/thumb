#ifndef MODE_HPP
#define MODE_HPP

#include "scene.hpp"
#include "entity.hpp"

//-----------------------------------------------------------------------------

namespace mode
{
    class mode
    {
    protected:

        ops::scene& scene;

    public:

        mode(ops::scene& s) : scene(s) { }

        virtual void enter() { }
        virtual void leave() { }

        virtual bool point(const float[3], const float[3], int, int);
        virtual bool click(int, bool);
        virtual bool keybd(int, bool, int);
        virtual bool timer(float);

        virtual void draw() const { }

        virtual ~mode() { }
    };
}

//-----------------------------------------------------------------------------

#endif
