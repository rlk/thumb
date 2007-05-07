//  Copyright (C) 2007 Robert Kooima
//
//  THUMB is free software; you can redistribute it and/or modify it under
//  the terms of  the GNU General Public License as  published by the Free
//  Software  Foundation;  either version 2  of the  License,  or (at your
//  option) any later version.
//
//  This program  is distributed in the  hope that it will  be useful, but
//  WITHOUT   ANY  WARRANTY;   without  even   the  implied   warranty  of
//  MERCHANTABILITY  or FITNESS  FOR A  PARTICULAR PURPOSE.   See  the GNU
//  General Public License for more details.

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <SDL.h>

#include "opengl.hpp"
#include "perf.hpp"

#ifdef NVPM //=================================================================

std::map<UINT, char *> perf::_name;
std::map<UINT, UINT64> perf::_hint;

int perf::counter(UINT index, char *name)
{
    UINT64 type;
    UINT64 hint;

    // Add all GPU and OpenGL counters.  Ignore D3D and Simple Experiments.

    NVPMGetCounterAttribute(index, NVPMA_COUNTER_TYPE, &type);

    if (type == NVPM_CT_GPU || type == NVPM_CT_OGL)
    {
        NVPMAddCounter(index);
        NVPMGetCounterAttribute(index, NVPMA_COUNTER_DISPLAY_HINT, &hint);

        _name[index] = name;
        _hint[index] = hint;
    }
    return NVPM_OK;
}

//-----------------------------------------------------------------------------

perf::perf(int n) : num(0), tot(0), lim(n), val(0), avg(0)
{
    if (NVPMInit() == NVPM_OK)
    {
        NVPMGetNumCounters(&num);
        NVPMEnumCounters(counter);

        val = new NVPMSampleValue[num];
        avg = new NVPMSampleValue[num];

        memset(val, 0, num * sizeof (NVPMSampleValue));
        memset(avg, 0, num * sizeof (NVPMSampleValue));
    }
}

perf::~perf()
{
    if (val)
    {
        for (UINT i = 0; i < num; ++i)
            NVPMRemoveCounter(val[i].unCounterIndex);

        delete [] val;
        delete [] avg;
    }

    NVPMShutdown();
}

//-----------------------------------------------------------------------------

void perf::step()
{
    if (val && avg)
    {
        // Sample the current counter values.

        NVPMSample(val, &num);

        // Accumulate running totals.

        for (UINT i = 0; i < num; ++i)
        {
            avg[i].ulValue  += val[i].ulValue;
            avg[i].ulCycles += val[i].ulCycles;
        }

        // Dump averages.

        if (tot++ == lim) dump();
    }
}

void perf::dump()
{
    if (num) std::cout << std::endl;

    for (UINT i = 0; i < num; ++i)
    {
        char  *n = _name[val[i].unCounterIndex];
        UINT64 h = _hint[val[i].unCounterIndex];

        UINT64 v = avg[i].ulValue  / tot;
        UINT64 c = avg[i].ulCycles / tot;

        // Output the counter name.

        std::cout << std::left  << std::setw(27) << n
                  << std::right << std::setw( 9) << std::setprecision(3);

        // Output the counter value.

        switch (h)
        {
        case NVPM_CDH_PERCENT:
            std::cout << 100.0 * v / c << " %  ";
            break;
        case NVPM_CDH_RAW:
            std::cout <<         v     << "    ";
            break;
        }

        // Output as two columns.

        if (i % 2 || i == num - 1) std::cout << std::endl;
    }

    // Reset the averages.

    memset(avg, 0, num * sizeof (NVPMSampleValue));
    tot = 0;
}

#else // not NVPM =============================================================

perf::perf(int n) : frames(0), ticks(0), limit(n), last(0)
{
}

perf::~perf()
{
}

void perf::step()
{
    static int last = 0;
    
    int dt = int(SDL_GetTicks()) - last;

    frames +=  1;
    ticks  += dt;
    last   += dt;

    if (frames == limit) dump();
}

void perf::dump()
{
    int fps = int(ceil(1000.0 * frames / ticks));

    frames = 0;
    ticks  = 0;

    std::ostringstream str;

    str << fps;

    SDL_WM_SetCaption(str.str().c_str(),
                      str.str().c_str());
}

#endif // not NVPM ============================================================
