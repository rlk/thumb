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

#ifndef APP_PERF_HPP
#define APP_PERF_HPP

#include <map>

#include <app-default.hpp>

//-----------------------------------------------------------------------------

#ifdef NVPM
#include <NVPerfSDK.h>
#endif

//-----------------------------------------------------------------------------

namespace app
{
#ifdef NVPM //-----------------------------------------------------------------

    class perf
    {
        SDL_Window *window;

        static std::map<UINT, char *> _name;
        static std::map<UINT, UINT64> _hint;

        UINT             num;
        UINT             tot;
        UINT             lim;
        NVPMSampleValue *val;
        NVPMSampleValue *avg;

        static int counter(UINT, char *);

    public:

        perf(SDL_Window *, int=DEFAULT_PERF_AVERAGE);
       ~perf();

        void step(bool=false);
        void dump(bool=false);
    };

#else // not NVPM -------------------------------------------------------------

    class perf
    {
        SDL_Window *window;

        Uint64 total_start;
        int    total_frames;

        Uint64 local_start;
        int    local_frames;
        int    local_limit;

    public:

        perf(SDL_Window *, int=DEFAULT_PERF_AVERAGE);
       ~perf();

        void step(bool=false);
        void dump(bool=false);
    };

#endif // not NVPM ------------------------------------------------------------
}

extern app::perf *perf;

//-----------------------------------------------------------------------------

#endif
