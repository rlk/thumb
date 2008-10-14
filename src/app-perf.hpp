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

#ifndef PERF_HPP
#define PERF_HPP

#include <map>

#include "default.hpp"

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
        static std::map<UINT, char *> _name;
        static std::map<UINT, UINT64> _hint;

        UINT             num;
        UINT             tot;
        UINT             lim;
        NVPMSampleValue *val;
        NVPMSampleValue *avg;

        static int counter(UINT, char *);

    public:

        perf(int=DEFAULT_PERF_AVERAGE);
       ~perf();

        void step(bool=false);
        void dump(bool=false);
    };

#else // not NVPM -------------------------------------------------------------

    class perf
    {
        int total;
        int frames;
        int ticks;
        int limit;
        int last;
        int fault;

    public:

        perf(int=DEFAULT_PERF_AVERAGE);
       ~perf();

        void miss();
        void step(bool=false);
        void dump(bool=false);
    };

#endif // not NVPM ------------------------------------------------------------
}

extern app::perf *perf;

//-----------------------------------------------------------------------------

#endif
