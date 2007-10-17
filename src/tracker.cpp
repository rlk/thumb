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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "matrix.hpp"

//-----------------------------------------------------------------------------

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

#include <math.h>

//-----------------------------------------------------------------------------

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef RAD
#define RAD(d) (PI * (d) / 180.0)
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

bool tracker_init(int t_key, int c_key)
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

void tracker_fini(void)
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

bool tracker_sensor(int id, double p[3], double R[3][3])
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

                R[0][0] = M[0];
                R[0][1] = M[1];
                R[0][2] = M[2];

                R[1][0] = M[4];
                R[1][1] = M[5];
                R[1][2] = M[6];

                R[2][0] = M[8];
                R[2][1] = M[9];
                R[2][2] = M[10];

/*
printf("p[%d] =\t%+12.5f %+12.5f %+12.5f\n", id, p[0],    p[1],    p[2]);
printf("R[%d] =\t%+12.5f %+12.5f %+12.5f\n", id, R[0][0], R[0][1], R[0][2]);
printf(       "\t%+12.5f %+12.5f %+12.5f\n",     R[1][0], R[1][1], R[1][2]);
printf(       "\t%+12.5f %+12.5f %+12.5f\n",     R[2][0], R[2][1], R[2][2]);
*/
                return true;
            }
        }
    }

    return false;
}

bool tracker_values(int id, double a[2])
{
    // Confirm that valuators ID and ID+1 exist.

    if (control != (struct control_header *) (-1))
    {
        if ((uint32_t) id < control->val_count - 1)
        {
            // Check that valuator ID has changed.

            float *p = (float *) ((unsigned char *) control
                                                  + control->val_offset);

//          if (memcmp(values + id, p + id, 2 * sizeof (float)))
            {
                memcpy(values + id, p + id, 2 * sizeof (float));

                // Return valuators ID and ID + 1.

                 a[0] = p[id + 0];
                 a[1] = p[id + 1];

                return true;
            }
        }
    }
    a[0] = 0.0;
    a[1] = 0.0;
    return false;
}

bool tracker_button(int id, bool& b)
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

//-----------------------------------------------------------------------------
