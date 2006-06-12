#include "play.hpp"

//-----------------------------------------------------------------------------

mode::play::play(ops::scene& s) : mode(s)
{
}

//-----------------------------------------------------------------------------

void mode::play::enter()
{
    scene.embody_all();
}

void mode::play::leave()
{
    scene.debody_all();
}

//-----------------------------------------------------------------------------

bool mode::play::timer(float dt)
{
    scene.update_joints();
    ent::entity::phys_step(dt);
    scene.update_solids();

    return true;
}

//-----------------------------------------------------------------------------
