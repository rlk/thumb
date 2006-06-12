#include <SDL.h>

#include "joint.hpp"
#include "info.hpp"

//-----------------------------------------------------------------------------

mode::info::info(ops::scene& s) : mode(s), gui(s)
{
}

//-----------------------------------------------------------------------------

void mode::info::enter()
{
    gui.show();
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
                        SDL_DEFAULT_REPEAT_INTERVAL);
}

void mode::info::leave()
{
    SDL_EnableKeyRepeat(0, 0);
    gui.hide();
}

//-----------------------------------------------------------------------------

bool mode::info::point(const float[3], const float[3], int x, int y)
{
    gui.point(x, y);
    return false;
}

bool mode::info::click(int b, bool d)
{
    if (b == 1)
    {
        gui.click(d);
        return true;
    }
    return false;
}

bool mode::info::keybd(int k, bool d, int c)
{
    if (d) gui.keybd(k, c);
    return true;
}

//-----------------------------------------------------------------------------

void mode::info::draw() const
{
    scene.draw_line();
    gui.draw();
}

//-----------------------------------------------------------------------------
