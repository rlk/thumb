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
#include "app-host.hpp"
#include "dev-wiimote.hpp"

#ifdef ENABLE_WIIMOTE
extern "C" {
#include <libcwiimote/wiimote.h>
#include <libcwiimote/wiimote_api.h>
}

//-----------------------------------------------------------------------------

//#define FILTER(A, B) { (A) = ((A) * 7.0 + (B)) / 8.0; }
#define FILTER(A, B) { (A) = (B); }

//-----------------------------------------------------------------------------

#define BUTTON_NC 0
#define BUTTON_DN 1
#define BUTTON_UP 2

#define BUTTON_A      0
#define BUTTON_B      1
#define BUTTON_C      2
#define BUTTON_Z      3
#define BUTTON_PLUS   4
#define BUTTON_MINUS  5
#define BUTTON_HOME   6
#define BUTTON_ONE    7
#define BUTTON_TWO    8
#define BUTTON_R      9
#define BUTTON_L     10
#define BUTTON_D     11
#define BUTTON_U     12
#define BUTTON_COUNT 13

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
    float joyx;
    float joyy;
    float irx;
    float iry;
    struct button_state A;
    struct button_state B;
    struct button_state C;
    struct button_state Z;
    struct button_state plus;
    struct button_state minus;
    struct button_state home;
    struct button_state one;
    struct button_state two;
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

static int continuity(int d)
{
    /* HACK: This works around an apparent bug in libcwiimote 0.4. */

    if ( 512 <= d && d <  768) return d - 256;
    if (1024 <= d && d < 1280) return d - 512;
    if (1536 <= d && d < 2048) return d - 768;

    return d;
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

        wiimote.led.one  = 1;
        wiimote.mode.ir  = 1;
        wiimote.mode.ext = 1;

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
                set_button(&state.one,   wiimote.keys.one);
                set_button(&state.two,   wiimote.keys.two);
                set_button(&state.L,     wiimote.keys.left);
                set_button(&state.R,     wiimote.keys.right);
                set_button(&state.U,     wiimote.keys.up);
                set_button(&state.D,     wiimote.keys.down);

                FILTER(state.joyx, wiimote.ext.nunchuk.joyx);
                FILTER(state.joyy, wiimote.ext.nunchuk.joyy);

                if (wiimote.ir1.x != 1791)
                    FILTER(state.irx, continuity(wiimote.ir1.x));
                if (wiimote.ir1.y != 1791)
                    FILTER(state.iry, continuity(wiimote.ir1.y));
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
    // Initialize the input state.

    view_move_rate = conf->get_f("view_move_rate");
    view_turn_rate = conf->get_f("view_turn_rate");

    move[0] = 0;
    move[1] = 0;
    move[2] = 0;

    turn[0] = 0;
    turn[1] = 0;
    turn[2] = 0;

    load_idt(init_R);
    load_idt(curr_R);

    button = std::vector<bool>(BUTTON_COUNT, false);

    // Initialize the shared state.

    memset(&state, 0, sizeof (struct tilt_state));

    state.joyx = 128;
    state.joyy = 128;

    strcpy(state.address, ::conf->get_s("wiimote").c_str());

    // Create the mutex and sensor thread.

    if (::host->root())
    {
        mutex  = SDL_CreateMutex();
        thread = SDL_CreateThread(get_wiimote, 0);
    }
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

void dev::wiimote::translate()
{
    int ch[BUTTON_COUNT];

    float joyx;
    float joyy;
    float irx;
    float iry;

    // Note all changes in Wiimote state.

    SDL_mutexP(mutex);
    {

        ch[BUTTON_A]     = get_button(&state.A);
        ch[BUTTON_B]     = get_button(&state.B);
        ch[BUTTON_C]     = get_button(&state.C);
        ch[BUTTON_Z]     = get_button(&state.Z);
        ch[BUTTON_PLUS]  = get_button(&state.plus);
        ch[BUTTON_MINUS] = get_button(&state.minus);
        ch[BUTTON_HOME]  = get_button(&state.home);
        ch[BUTTON_ONE]   = get_button(&state.one);
        ch[BUTTON_TWO]   = get_button(&state.two);
        ch[BUTTON_R]     = get_button(&state.R);
        ch[BUTTON_L]     = get_button(&state.L);
        ch[BUTTON_D]     = get_button(&state.D);
        ch[BUTTON_U]     = get_button(&state.U);

        irx  = state.irx;
        iry  = state.iry;
        joyx = state.joyx;
        joyy = state.joyy;
    }
    SDL_mutexV(mutex);

    // Convert Wiimote IR sensor changes int point events.

    double M[16], q[4], p[3] = { 0.0, 0.0, 0.0 };

    load_idt(M);

    M[ 8] = (irx - 512.0) / 1024.0;
    M[ 9] = (iry - 512.0) / 1024.0;
    M[10] = 1.0;
    M[11] = 0.0;

    normalize(M + 8);
    crossprod(M + 0, M + 4, M + 8);
    normalize(M + 0);
    crossprod(M + 4, M + 8, M + 0);
    normalize(M + 4);

    get_quaternion(q, M);

    ::host->point(0, p, q);

    // Convert Wiimote button state changes into click events.

    for (int i = 0; i < BUTTON_COUNT; ++i)
        switch (ch[i])
        {
        case BUTTON_NC:                                break;
        case BUTTON_DN: ::host->click(0, i, 0, true);  break;
        case BUTTON_UP: ::host->click(0, i, 0, false); break;
        }

    // Convert Nunchuk joystick changes into value events.

    ::host->value(0, 0, joyx);
    ::host->value(0, 1, joyy);
}

//-----------------------------------------------------------------------------

bool dev::wiimote::point(int i, const double *p, const double *q)
{
    set_quaternion(curr_R, q);

    return false;
}

bool dev::wiimote::click(int i, int b, int m, bool d)
{
    button[b] = d;

    // B points.

    if (b == BUTTON_B)
        load_mat(init_R, curr_R);

    // Plus and Minus move through the demo.

    if (!d && b == BUTTON_PLUS)  { ::prog->next(); return false; }
    if (!d && b == BUTTON_MINUS) { ::prog->prev(); return false; }

    // D-pad selects display options.

    if (d)
    {
        if      (button[BUTTON_TWO])
            switch (b)
            {
            case BUTTON_R: ::prog->toggle(11); break;
            case BUTTON_L: ::prog->toggle(10); break;
            case BUTTON_D: ::prog->toggle( 9); break;
            case BUTTON_U: ::prog->toggle( 8); break;
            }
        else if (button[BUTTON_ONE])
            switch (b)
            {
            case BUTTON_R: ::prog->toggle( 7); break;
            case BUTTON_L: ::prog->toggle( 6); break;
            case BUTTON_D: ::prog->toggle( 5); break;
            case BUTTON_U: ::prog->toggle( 4); break;
            }
        else
            switch (b)
            {
            case BUTTON_R: ::prog->toggle( 3); break;
            case BUTTON_L: ::prog->toggle( 2); break;
            case BUTTON_D: ::prog->toggle( 1); break;
            case BUTTON_U: ::prog->toggle( 0); break;
            }
    }

    // Home goes home.

    if (d && b == BUTTON_HOME) ::user->home();

    return true;
}

bool dev::wiimote::value(int d, int a, double v)
{
    double f = (v - 128.0) / 128.0;

    if      (button[BUTTON_Z])
    {
        if (a == 0) turn[2] = -((fabs(f) > 0.25) ? f : 0.0);
        if (a == 1) move[1] =  ((fabs(f) > 0.25) ? f : 0.0);

        move[0] = 0.0;
        move[2] = 0.0;
        time    = 0.0;
    }
    else if (button[BUTTON_C])
    {
        if (a == 0) time    = ((fabs(f) > 0.25) ? f : 0.0);

        move[0] = 0.0;
        move[2] = 0.0;
        turn[2] = 0.0;
        move[1] = 0.0;
    }
    else
    {
        if (a == 0) move[0] =  ((fabs(f) > 0.25) ? f : 0.0);
        if (a == 1) move[2] = -((fabs(f) > 0.25) ? f : 0.0);

        turn[2] = 0.0;
        move[1] = 0.0;
        time    = 0.0;
    }
    return false;
}

bool dev::wiimote::timer(int t)
{
    double dt = t / 1000.0;

    double kr = dt * view_turn_rate * 3.0;
    double kp = dt * universe.move_rate();
    double ka = dt * universe.turn_rate();

    // Translate Wiimote state changes to host events.

    if (::host->root())
        translate();

    // Handle pointing.

    if (button[BUTTON_B])
    {
        double dR[3];
        double dz[3];
        double dy[3];

        dy[0] = init_R[ 4] - curr_R[ 4];
        dy[1] = init_R[ 5] - curr_R[ 5];
        dy[2] = init_R[ 6] - curr_R[ 6];

        dz[0] = init_R[ 8] - curr_R[ 8];
        dz[1] = init_R[ 9] - curr_R[ 9];
        dz[2] = init_R[10] - curr_R[10];

        dR[0] =  DOT3(dz, init_R + 4);
        dR[1] = -DOT3(dz, init_R + 0);
        dR[2] =  DOT3(dy, init_R + 0);

        user->turn(dR[0] * kr, dR[1] * kr, dR[2] * kr, curr_R);
    }

    // Handle joysticks.

    user->move(move[0] * kp, move[1] * kp, move[2] * kp);
    user->turn(turn[0] * kr, turn[1] * kr, turn[2] * kr);

    universe.set_time(universe.get_time() + ka * time * 600.0);


    return false;
}

//-----------------------------------------------------------------------------

#endif // ENABLE_WIIMOTE
