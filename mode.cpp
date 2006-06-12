#include "mode.hpp"

//-----------------------------------------------------------------------------

bool mode::mode::point(const float[3], const float[3], int, int)
{
    return false;
}

bool mode::mode::click(int, bool)
{
    return false;
}

bool mode::mode::keybd(int, bool, int)
{
    return false;
}

bool mode::mode::timer(float dt)
{
    ent::entity::phys_step(dt);
    return false;
}

//-----------------------------------------------------------------------------
