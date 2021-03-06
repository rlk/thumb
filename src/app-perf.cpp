//  Copyright (C) 2007-2011 Robert Kooima
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

#include <ogl-opengl.hpp>
#include <app-perf.hpp>

// TODO: Convert this away from iostream.

#ifdef NVPM //=================================================================

std::map<UINT, char *> app::perf::_name;
std::map<UINT, UINT64> app::perf::_hint;

int app::perf::counter(UINT index, char *name)
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

app::perf::perf(SDL_Window *w, int n) : window(w), num(0), tot(0), lim(n), val(0), avg(0)
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

app::perf::~perf()
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

void app::perf::step(bool log)
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

        if (tot++ == lim) dump(log);
    }
}

void app::perf::dump(bool log)
{
    if (num) std::cout << std::endl;

    for (UINT i = 0; i < num; ++i)
    {
        char  *n = _name[val[i].unCounterIndex];
        UINT64 h = _hint[val[i].unCounterIndex];

        UINT64 v = avg[i].ulValue  / tot;
        UINT64 c = avg[i].ulCycles / tot;

        // Output the counter name.

        std::cout << std::left  << std::setw(36) << std::setfill('.') << n
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

app::perf::perf(SDL_Window *w, int n) : window(w)
{
    Uint64 c = SDL_GetPerformanceCounter();

    total_start  = c;
    total_frames = 0;

    local_start  = c;
    local_frames = 0;
    local_limit  = n;
}

app::perf::~perf()
{
}

void app::perf::step(bool log)
{
    // Count a frame.

    local_frames++;
    total_frames++;

    // Report as often as configured.

    if (local_frames == local_limit)
    {
        dump(log);
        local_frames = 0;
    }
}

void app::perf::dump(bool log)
{
    // Sample the timer.

    Uint64 current = SDL_GetPerformanceCounter();
    Uint64 persec  = SDL_GetPerformanceFrequency();

    // Calculate the timings.

    double d1 = double(current - local_start) / double(persec);
    double dn = double(current - total_start) / double(persec);
    double m1 = 1000.0 * d1 / local_frames;
    double mn = 1000.0 * dn / total_frames;
    int   fps = int(ceil(local_frames / d1));

    local_start = current;

    // Report to a string. Set the window title and log.

    std::ostringstream str;

    str << std::fixed << std::setprecision(1) << m1  << "ms "
                                       << "(" << mn  << "ms) "
                                              << fps << "fps";

    SDL_SetWindowTitle(window, str.str().c_str());

    if (log) std::cout << str.str() << std::endl;
}

#endif // not NVPM ============================================================
