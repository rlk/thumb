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
#include "app-prog.hpp"
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
    float y;
    struct button_state A;
    struct button_state B;
    struct button_state C;
    struct button_state Z;
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
                set_button(&state.C,     wiimote.ext.nunchuk.keys.c);
                set_button(&state.Z,     wiimote.ext.nunchuk.keys.z);
                set_button(&state.plus,  wiimote.keys.plus);
                set_button(&state.minus, wiimote.keys.minus);
                set_button(&state.home,  wiimote.keys.home);
                set_button(&state.L,     wiimote.keys.left);
                set_button(&state.R,     wiimote.keys.right);
                set_button(&state.U,     wiimote.keys.up);
                set_button(&state.D,     wiimote.keys.down);

                state.x = (wiimote.ext.nunchuk.joyx - 128.0f) / 128.0f;
                state.y = (wiimote.ext.nunchuk.joyy - 128.0f) / 128.0f;


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

    move[0] = 0;
    move[1] = 0;
    move[2] = 0;

    turn[0] = 0;
    turn[1] = 0;
    turn[2] = 0;
    turn[3] = 0;
    turn[4] = 0;

    view_move_rate = conf->get_f("view_move_rate");
    view_turn_rate = conf->get_f("view_turn_rate");

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
    int P  = 0;
    int M  = 0;

    double dt = t / 1000.0;

    double kr = dt * view_turn_rate;
    double kp = dt * universe.move_rate();
    double ka = dt * universe.turn_rate();


    SDL_mutexP(mutex);
    {
        if (get_button(&state.plus)  == BUTTON_DN) P = 1;
        if (get_button(&state.minus) == BUTTON_DN) M = 1;

        move[0] = (fabs(state.x) > 0.25) ? +state.x : 0.0;
        move[2] = (fabs(state.y) > 0.25) ? -state.y : 0.0;
    }
    SDL_mutexV(mutex);

    if (P) ::prog->trigger(1);
    if (M) ::prog->trigger(2);

    user->move(move[0] * kp, move[1] * kp, move[2] * kp);

    return true;
}

//-----------------------------------------------------------------------------

#endif // ENABLE_WIIMOTE
