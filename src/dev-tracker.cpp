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

#include <cassert>

#include "util.hpp"
#include "matrix.hpp"
#include "default.hpp"
#include "app-prog.hpp"
#include "app-conf.hpp"
#include "app-user.hpp"
#include "app-host.hpp"
#include "app-event.hpp"
#include "dev-tracker.hpp"

//=============================================================================
// TRACKD shared memory protocol implementation

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>

#ifdef __linux__
#include <sys/shm.h>
#include <stdint.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef unsigned int uint32_t;
#endif

#ifdef __APPLE__
#include <sys/shm.h>
#include <stdint.h>
#endif

//-----------------------------------------------------------------------------

struct tracker_header
{
    uint32_t version;
    uint32_t count;
    uint32_t offset;
    uint32_t size;
    uint32_t time[2];
    uint32_t command;
};

struct control_header
{
    uint32_t version;
    uint32_t but_offset;
    uint32_t val_offset;
    uint32_t but_count;
    uint32_t val_count;
    uint32_t time[2];
    uint32_t command;
};

struct sensor
{
    float    p[3];
    float    r[3];
    uint32_t t[2];
    uint32_t calib;
    uint32_t frame;
};

//-----------------------------------------------------------------------------

#ifndef _WIN32
static int tracker_id = -1;
static int control_id = -1;
#else
static volatile HANDLE tracker_id = NULL;
static volatile HANDLE control_id = NULL;
#endif

static struct tracker_header *tracker = (struct tracker_header *) (-1);
static struct control_header *control = (struct control_header *) (-1);

static struct sensor *sensors = NULL;
static uint32_t      *buttons = NULL;
static float         *values  = NULL;

//-----------------------------------------------------------------------------

static bool tracker_init(int t_key, int c_key)
{
    // Acquire the tracker and controller shared memory segments.

#ifndef _WIN32

    if ((tracker_id = shmget(t_key, sizeof (struct tracker_header), 0)) >= 0)
        tracker = (struct tracker_header *) shmat(tracker_id, 0, 0);

    if ((control_id = shmget(c_key, sizeof (struct control_header), 0)) >= 0)
        control = (struct control_header *) shmat(control_id, 0, 0);

#else

    char shmkey[256];

    sprintf(shmkey, "%d", t_key);

    if ((tracker_id = OpenFileMapping(FILE_MAP_WRITE, FALSE, shmkey)))
        tracker = (struct tracker_header *)
                    MapViewOfFile(tracker_id, FILE_MAP_WRITE, 0, 0, 0);
    else
        tracker = (struct tracker_header *) (-1);

    sprintf(shmkey, "%d", c_key);

    if ((control_id = OpenFileMapping(FILE_MAP_WRITE, FALSE, shmkey)))
        control = (struct control_header *)
                    MapViewOfFile(control_id, FILE_MAP_WRITE, 0, 0, 0);
    else
        control = (struct control_header *) (-1);

#endif

    // Allocate storage for state caches.

    if (control != (struct control_header *) (-1))
    {
        buttons = (uint32_t *) calloc(control->but_count, sizeof (uint32_t));
        values  = (float    *) calloc(control->val_count, sizeof (float));
    }
    if (tracker != (struct tracker_header *) (-1))
        sensors = (sensor   *) calloc(tracker->count,     sizeof (sensor));

    return true;
}

static void tracker_fini(void)
{
    // Detach shared memory segments.

#ifndef _WIN32
    if (control != (struct control_header *) (-1)) shmdt(control);
    if (tracker != (struct tracker_header *) (-1)) shmdt(tracker);

    control_id = -1;
    tracker_id = -1;
#else
    if (control != (struct control_header *) (-1)) UnmapViewOfFile(control);
    if (tracker != (struct tracker_header *) (-1)) UnmapViewOfFile(tracker);

    CloseHandle(control_id);
    CloseHandle(tracker_id);

    control_id = NULL;
    tracker_id = NULL;
#endif

    if (sensors) free(sensors);
    if (values)  free(values);
    if (buttons) free(buttons);

    // Mark everything as uninitialized.

    control = (struct control_header *) (-1);
    tracker = (struct tracker_header *) (-1);

    buttons = NULL;
    values  = NULL;
    sensors = NULL;
}

bool tracker_status(void)
{
    return ((tracker != (struct tracker_header *) (-1)));
}

//-----------------------------------------------------------------------------

static int tracker_count_sensors()
{
    if (tracker != (struct tracker_header *) (-1))
        return int(tracker->count);
    else
        return 0;
}

static int tracker_count_values()
{
    if (control != (struct control_header *) (-1))
        return int(control->val_count);
    else
        return 0;
}

static int tracker_count_buttons()
{
    if (control != (struct control_header *) (-1))
        return int(control->but_count);
    else
        return 0;
}

//-----------------------------------------------------------------------------

static bool tracker_sensor(int id, double p[3], double q[4])
{
    // Confirm that sensor ID exists.

    if (tracker != (struct tracker_header *) (-1))
    {
        if ((uint32_t) id < tracker->count)
        {
            // Check that sensor ID has changed rotation.

            struct sensor *S =
                (struct sensor *)((unsigned char *) tracker
                                                  + tracker->offset
                                                  + tracker->size * id);

            if (memcmp(sensors[id].p, S->p, 6 * sizeof (float)))
            {
                memcpy(sensors[id].p, S->p, 6 * sizeof (float));

                // Return the position of sensor ID.

                p[0] = double(S->p[0]);
                p[1] = double(S->p[1]);
                p[2] = double(S->p[2]);

                // Return the orientation of sensor ID.

                double M[16];

                load_rot_mat(M, 0, 1, 0, S->r[0]);
                Rmul_rot_mat(M, 1, 0, 0, S->r[1]);
                Rmul_rot_mat(M, 0, 0, 1, S->r[2]);

                get_quaternion(q, M);

                return true;
            }
        }
    }

    return false;
}

static bool tracker_value(int id, double& a)
{
    // Confirm that valuator ID exists.

    if (control != (struct control_header *) (-1))
    {
        if ((uint32_t) id < control->val_count)
        {
            // Check that valuator ID has changed.

            float *p = (float *) ((unsigned char *) control
                                                  + control->val_offset);

            if (memcmp(values + id, p + id, sizeof (float)))
            {
                memcpy(values + id, p + id, sizeof (float));

                // Return valuator ID.

                a = p[id];

                return true;
            }
        }
    }

    return false;
}

static bool tracker_button(int id, bool& b)
{
    // Confirm that button ID exists.

    if (control != (struct control_header *) (-1))
    {
        if ((uint32_t) id < control->but_count)
        {
            // Check that button ID has changed.

            uint32_t *p = (uint32_t *) ((unsigned char *) control
                                                        + control->but_offset);

            if (memcmp(buttons + id, p + id, sizeof (uint32_t)))
            {
                memcpy(buttons + id, p + id, sizeof (uint32_t));

                // Return button ID.

                b = bool(p[id]);

                return true;
            }
        }
    }
    b    = false;
    return false;
}

//=============================================================================

dev::tracker::tracker() : flying(false), joy_x(0), joy_y(0)
{
    // Initialize the event state.

    init_P[0] = curr_P[0];
    init_P[1] = curr_P[1];
    init_P[2] = curr_P[2];

    load_idt(init_R);
    load_idt(curr_R);

    // Configure the buttons and axes.

    const std::string unit = conf->get_s("tracker_unit");

    scale = unit_scale(unit.empty() ? "ft" : unit);

    tracker_head_sensor = conf->get_i("tracker_head_sensor");
    tracker_hand_sensor = conf->get_i("tracker_hand_sensor");

    tracker_butn_fly    = conf->get_i("tracker_butn_fly");
    tracker_butn_home   = conf->get_i("tracker_butn_home");

    tracker_axis_A      = conf->get_i("tracker_axis_A");
    tracker_axis_T      = conf->get_i("tracker_axis_T");

    key_edit            = conf->get_i("key_edit");
    key_play            = conf->get_i("key_play");
    key_info            = conf->get_i("key_info");

    // Initialize the tracker interface.

    int tracker_key = conf->get_i("tracker_key");
    int control_key = conf->get_i("control_key");

    if (tracker_key == 0) tracker_key = DEFAULT_TRACKER_KEY;
    if (control_key == 0) control_key = DEFAULT_CONTROL_KEY;

    tracker_init(tracker_key, control_key);
}

dev::tracker::~tracker()
{
    tracker_fini();
}

//-----------------------------------------------------------------------------

void dev::tracker::translate() const
{
    // Translate tracker status changes into event messages.

    if (tracker_status())
    {
        app::event E;

        int    i;
        bool   b;
        double v;
        double p[3];
        double q[3];

        for (i = 0; i < tracker_count_buttons(); ++i)
            if (tracker_button(i, b))
            {
                switch (i)
                {
                case 2:  E.mk_keybd(0, key_play, 0, b); break;
                case 3:  E.mk_keybd(0, key_edit, 0, b); break;
                case 4:  E.mk_keybd(0, key_info, 0, b); break;
                default: E.mk_click(0, i, 0, b);        break;
                }
                ::host->process_event(&E);
            }

        for (i = 0; i < tracker_count_values (); ++i)
            if (tracker_value (i, v))
                ::host->process_event(E.mk_value(0, i, v));

        for (i = 0; i < tracker_count_sensors(); ++i)
            if (tracker_sensor(i, p, q))
            {
                p[0] *= scale;
                p[1] *= scale;
                p[2] *= scale;

                ::host->process_event(E.mk_point(i, p, q));
            }
    }
}

//-----------------------------------------------------------------------------

bool dev::tracker::process_point(app::event *E)
{
    const int i     = E->data.point.i;
    const double *p = E->data.point.p;
    const double *q = E->data.point.q;

    if (i == tracker_head_sensor)
        ::host->set_head(p, q);

    if (i == tracker_hand_sensor)
    {
        curr_P[0] = p[0];
        curr_P[1] = p[1];
        curr_P[2] = p[2];

        set_quaternion(curr_R, q);
    }

    return false;
}

bool dev::tracker::process_click(app::event *E)
{
    const int  b = E->data.click.b;
    const bool d = E->data.click.d;

    if (b == tracker_butn_fly)
    {
        flying = d;

        init_P[0] = curr_P[0];
        init_P[1] = curr_P[1];
        init_P[2] = curr_P[2];

        load_mat(init_R, curr_R);

        return true;
    }
    return false;
}

bool dev::tracker::process_value(app::event *E)
{
    if (E->data.value.a == 0) joy_x = E->data.value.v;
    if (E->data.value.a == 1) joy_y = E->data.value.v;

    return false;
}

bool dev::tracker::process_timer(app::event *E)
{
    const double dt = E->data.timer.dt * 0.001;

    const double kr = dt * ::user->get_turn_rate() * 45.0;
    const double kp = dt * ::user->get_move_rate();

    // Translate state changes to host events.

    if (::host->root())
        translate();

    // Handle navigation.

    if (flying)
    {
        double dP[3];
        double dR[3];
        double dz[3];
        double dy[3];

        dP[0] = curr_P[ 0] - init_P[ 0];
        dP[1] = curr_P[ 1] - init_P[ 1];
        dP[2] = curr_P[ 2] - init_P[ 2];
        
        dy[0] = init_R[ 4] - curr_R[ 4];
        dy[1] = init_R[ 5] - curr_R[ 5];
        dy[2] = init_R[ 6] - curr_R[ 6];

        dz[0] = init_R[ 8] - curr_R[ 8];
        dz[1] = init_R[ 9] - curr_R[ 9];
        dz[2] = init_R[10] - curr_R[10];

        dR[0] =  DOT3(dz, init_R + 4);
        dR[1] = -DOT3(dz, init_R + 0);
        dR[2] =  DOT3(dy, init_R + 0);

        ::user->turn(dR[0] * kr, dR[1] * kr, dR[2] * kr, curr_R);
        ::user->move(dP[0] * kp, dP[1] * kp, dP[2] * kp);
    }

    // Handle the passage of time.

    if (fabs(joy_x) > 0.1)
        ::user->pass(joy_x * dt * 4.0 * 3600.0);

    return false;
}

bool dev::tracker::process_event(app::event *E)
{
    assert(E);

    bool R = false;

    switch (E->get_type())
    {
    case E_POINT: R |= process_point(E); break;
    case E_CLICK: R |= process_click(E); break;
    case E_VALUE: R |= process_value(E); break;
    case E_TIMER: R |= process_timer(E); break;
    }

    return R || dev::input::process_event(E);
}

//-----------------------------------------------------------------------------
