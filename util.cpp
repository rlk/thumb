#include "util.hpp"

//-----------------------------------------------------------------------------

float cosi(int a)
{
    static float value[360];
    static bool  state = false;

    // If this is the first call, initialize the value array.

    if (!state)
    {
        for (int i = 0; i < 360; ++i)
            value[i] = float(cos(i * 6.28318528f / 360.0f));

        state = true;
    }

    return value[a % 360];
}

float sini(int a)
{
    static float value[360];
    static bool  state = false;

    // If this is the first call, initialize the value array.

    if (!state)
    {
        for (int i = 0; i < 360; ++i)
            value[i] = float(sin(i * 6.28318528f / 360.0f));

        state = true;
    }

    return value[a % 360];
}

//-----------------------------------------------------------------------------
