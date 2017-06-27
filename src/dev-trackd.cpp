//  Copyright (C) 2007-2011, 2017 Robert Kooima
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

#include <etc-vector.hpp>
#include <etc-log.hpp>
#include <app-default.hpp>
#include <app-conf.hpp>
#include <app-view.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <dev-trackd.hpp>

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
#include <windows.h>
#include <stdint.h>
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

                mat4 M = yrotation(to_radians(double(S->r[0])))
                       * xrotation(to_radians(double(S->r[1])))
                       * zrotation(to_radians(double(S->r[2])));

                quat r(mat3(xvector(M),
                            yvector(M),
                            zvector(M)));

                q[0] = r[0];
                q[1] = r[1];
                q[2] = r[2];
                q[3] = r[3];

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

                b = (p[id] != 0);

                return true;
            }
        }
    }
    b    = false;
    return false;
}

//=============================================================================

dev::trackd::trackd() :
    status(false),
    flying(false)
{
    scale       = ::conf->get_f("trackd_scale",       1.0);
    move_rate   = ::conf->get_f("trackd_move_rate",  30.0);
    turn_rate   = ::conf->get_f("trackd_turn_rate", 120.0);
    head_sensor = ::conf->get_i("trackd_head_sensor", 0);
    hand_sensor = ::conf->get_i("trackd_hand_sensor", 1);
    fly_button  = ::conf->get_i("trackd_fly_button",  0);

    int tracker_key = ::conf->get_i("tracker_key", 4126);
    int control_key = ::conf->get_i("control_key", 4127);

    if (tracker_init(tracker_key, control_key))
    {
        etc::log("tracking %d sensors %d values %d buttons",
                  tracker_count_sensors(),
                  tracker_count_values(),
                  tracker_count_buttons());
    }
}

dev::trackd::~trackd()
{
    tracker_fini();
}

//-----------------------------------------------------------------------------

void dev::trackd::translate()
{
    if (tracker_status())
    {
        app::event E;

        double p[3];
        double q[4];
        double v;
        bool   d;

        for (int b = 0; b < tracker_count_buttons(); ++b)
            if (tracker_button(b, d))
                ::host->process_event(E.mk_button(0, b, d));

        for (int a = 0; a < tracker_count_values (); ++a)
            if (tracker_value (a, v))
                ::host->process_event(E.mk_axis(0, a, v));

        for (int i = 0; i < tracker_count_sensors(); ++i)
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

bool dev::trackd::process_point(app::event *E)
{
    const int     i = E->data.point.i;
    const double *p = E->data.point.p;
    const double *q = E->data.point.q;

    if (i == hand_sensor)
    {
        curr_p = vec3(p[0], p[1], p[2]);
        curr_q = quat(q[0], q[1], q[2], q[3]);
    }
    return false;
}

bool dev::trackd::process_axis(app::event *E)
{
//  const int    a = E->data.axis.a;
//  const double v = E->data.axis.v;

    return false;
}

bool dev::trackd::process_button(app::event *E)
{
    const int  b = E->data.button.b;
    const bool d = E->data.button.d;

    if (b == fly_button)
    {
        flying = d;
        init_p = curr_p;
        init_q = curr_q;

        return true;
    }
    return false;
}

bool dev::trackd::process_tick(app::event *E)
{
    const double dt = E->data.tick.dt;

    if (::host->root())
        translate();

    if (flying)
    {
        quat o = ::host->get_orientation();
        quat q = normal(inverse(init_q) * curr_q);
        quat r = normal(slerp(quat(), q, 1.0 / 30.0));
        vec3 d = (curr_p - init_p) * dt * move_rate;

        ::host->set_orientation(o * r);
        ::host->offset_position(o * d);
    }

    return false;
}

bool dev::trackd::process_event(app::event *E)
{
    switch (E->get_type())
    {
    case E_POINT:  if (process_point (E)) return true; else break;
    case E_AXIS:   if (process_axis  (E)) return true; else break;
    case E_BUTTON: if (process_button(E)) return true; else break;
    case E_TICK:   if (process_tick  (E)) return true; else break;
    }
    return dev::input::process_event(E);
}

//=============================================================================
