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

#include <cstring>

#include "matrix.hpp"
#include "app-conf.hpp"
#include "app-user.hpp"
#include "dev-wiimote.hpp"

#ifdef ENABLE_WIIMOTE
extern "C" {
#include <libcwiimote/wiimote.h>
#include <libcwiimote/wiimote_api.h>
}

//-----------------------------------------------------------------------------

#define FILTER 8

#define BUTTON_NC 0
#define BUTTON_DN 1
#define BUTTON_UP 2

struct button_state
{
    unsigned char curr;
    unsigned char last;
    unsigned char upc;
    unsigned char dnc;
};

struct tilt_state
{
    char  address[256];
    int   status;
    float x;
    float z;
    struct button_state A;
    struct button_state B;
    struct button_state plus;
    struct button_state minus;
    struct button_state home;
    struct button_state L;
    struct button_state R;
    struct button_state U;
    struct button_state D;
};

static struct tilt_state state;
static SDL_mutex        *mutex  = NULL;
static SDL_Thread       *thread = NULL;

//-----------------------------------------------------------------------------

static void set_button(struct button_state *B, int s)
{
    if ((B->curr == 0) != (s == 0))
    {
        if (B->curr)
        {
            B->upc++;
            B->curr = 0;
        }
        else
        {
            B->dnc++;
            B->curr = 1;
        }
    }
}

static int get_button(struct button_state *B)
{
    int ch = BUTTON_NC;

    if      (B->last == 1 && B->upc > 0)
    {
        B->upc--;
        B->last = 0;
        ch = BUTTON_UP;
    }
    else if (B->last == 0 && B->dnc > 0)
    {
        B->dnc--;
        B->last = 1;
        ch = BUTTON_DN;
    }

    return ch;
}

static int get_wiimote(void *data)
{
    wiimote_t wiimote = WIIMOTE_INIT;

    if (wiimote_connect(&wiimote, state.address) < 0)
        fprintf(stderr, "%s\n", wiimote_get_error());
    else
    {
        int running = 1;

        printf("Wiimote %s connected\n", state.address);

        wiimote.mode.bits = WIIMOTE_MODE_ACC;
        wiimote.led.one   = 1;

        SDL_mutexP(mutex);
        state.status = running;
        SDL_mutexV(mutex);

        while (mutex && running && wiimote_is_open(&wiimote))
        {
            if (wiimote_update(&wiimote) < 0)
                break;

            SDL_mutexP(mutex);
            {
                running = state.status;

                set_button(&state.A,     wiimote.keys.a);
                set_button(&state.B,     wiimote.keys.b);
                set_button(&state.plus,  wiimote.keys.plus);
                set_button(&state.minus, wiimote.keys.minus);
                set_button(&state.home,  wiimote.keys.home);
                set_button(&state.L,     wiimote.keys.left);
                set_button(&state.R,     wiimote.keys.right);
                set_button(&state.U,     wiimote.keys.up);
                set_button(&state.D,     wiimote.keys.down);
/*
                    if (isnormal(wiimote.tilt.y))
                    {
                        state.x = (state.x * (FILTER - 1) +
                                   wiimote.tilt.y) / FILTER;
                    }
                    if (isnormal(wiimote.tilt.x))
                    {
                        state.z = (state.z * (FILTER - 1) +
                                   wiimote.tilt.x) / FILTER;
                    }
*/
            }
            SDL_mutexV(mutex);
        }
        wiimote_disconnect(&wiimote);
    }
    return 0;
}

//-----------------------------------------------------------------------------

dev::wiimote::wiimote(uni::universe& universe) :
    universe(universe)
{
    // Initialize the state.

    memset(&state, 0, sizeof (struct tilt_state));

    strcpy(state.address, ::conf->get_s("wiimote").c_str());

    // Create the mutex and sensor thread.

    mutex  = SDL_CreateMutex();
    thread = SDL_CreateThread(get_wiimote, 0);
}

dev::wiimote::~wiimote()
{
    if (mutex)
    {
        int status;

        // Get/set the status of the tilt sensor thread.

        SDL_mutexP(mutex);
        state.status = 0;
        SDL_mutexV(mutex);

        // Kill the thread and destroy the mutex.

        SDL_WaitThread(thread, &status);
        SDL_DestroyMutex(mutex);

        mutex  = NULL;
        thread = NULL;
    }
}

//-----------------------------------------------------------------------------

bool dev::wiimote::timer(int t)
{
    int ch = BUTTON_NC;

    SDL_mutexP(mutex);
    {
        switch (get_button(&state.A))
        {
        case BUTTON_UP: printf("A up\n"); break;
        case BUTTON_DN: printf("A dn\n"); break;
        default:                          break;
        }
    }
    SDL_mutexV(mutex);

    return false;
}

//-----------------------------------------------------------------------------

#endif // ENABLE_WIIMOTE
