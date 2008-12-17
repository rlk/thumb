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
#include <iostream>
#include <stdexcept>

#include "uni-overlay.hpp"
#include "util.hpp"

#define MAXSTR 256

//-----------------------------------------------------------------------------

void uni::overlay::m_moveto(const char *ibuf, char *obuf)
{
    float lat;
    float lon;

    if (sscanf(ibuf, "%f %f\n", &lat, &lon) == 2)
        printf("moveto(%f, %f)\n", lat, lon);
}

void uni::overlay::m_lookup(const char *ibuf, char *obuf)
{
    float x;
    float y;

    if (sscanf(ibuf, "%f %f\n", &x, &y) == 2)
    {
        printf("lookup(%f, %f)\n", x, y);

        if (obuf)
            sprintf(obuf, "%f %f\n", 0.0f, 0.0f);
    }
}

void uni::overlay::m_get_position(const char *ibuf, char *obuf)
{
}

//-----------------------------------------------------------------------------

void uni::overlay::m_model_create(const char *ibuf, char *obuf)
{
    int state = 0;
    char filename[MAXSTR];

    if (sscanf(ibuf, "%s", filename) == 1)
    {
        printf("model create %s\n", filename);
        state = 1;
    }

    if (obuf) sprintf(obuf, "%d\n", 1);
}

void uni::overlay::m_model_delete(const char *ibuf, char *obuf)
{
    int state = 0;
    int index;

    if (sscanf(ibuf, "%d", &index) == 1)
    {
        printf("model delete %d\n", index);
        state = 1;
    }

    if (obuf) sprintf(obuf, "%d\n", state);
}

void uni::overlay::m_model_position(const char *ibuf, char *obuf)
{
    int   state = 0;
    int   index;
    float lat;
    float lon;
    float rad;
    char  coord[256];

    if (sscanf(ibuf, "%d %f %f %f %s", &index, &lat, &lon, &rad, coord) == 5)
    {
        printf("model position(%d, %f, %f, %f, %s)\n",
               index, lat, lon, rad, coord);
        state = 1;
    }

    if (obuf) sprintf(obuf, "%d\n", state);
}

void uni::overlay::m_model_rotation(const char *ibuf, char *obuf)
{
    int   state = 0;
    int   index;
    float angle;

    if (sscanf(ibuf, "%d %f", &index, &angle) == 2)
    {
        printf("model rotation(%d, %f)\n", index, angle);
        state = 1;
    }

    if (obuf) sprintf(obuf, "%d\n", state);
}

void uni::overlay::m_model_scale(const char *ibuf, char *obuf)
{
    int   state = 0;
    int   index;
    float scale;

    if (sscanf(ibuf, "%d %f", &index, &scale) == 2)
    {
        printf("model scale(%d, %f)\n", index, scale);
        state = 1;
    }

    if (obuf) sprintf(obuf, "%d\n", state);
}

//-----------------------------------------------------------------------------

void uni::overlay::m_image_create(const char *ibuf, char *obuf)
{
    int state = 0;
    char filename[MAXSTR];

    if (sscanf(ibuf, "%s", filename) == 1)
    {
        printf("image create %s\n", filename);
        state = 1;
    }

    if (obuf) sprintf(obuf, "%d\n", 1);
}

void uni::overlay::m_image_delete(const char *ibuf, char *obuf)
{
    int state = 0;
    int index;

    if (sscanf(ibuf, "%d", &index) == 1)
    {
        printf("image delete %d\n", index);
        state = 1;
    }

    if (obuf) sprintf(obuf, "%d\n", state);
}

void uni::overlay::m_image_position(const char *ibuf, char *obuf)
{
    int   state = 0;
    int   index;
    float lat;
    float lon;
    float rad;
    char  coord[256];

    if (sscanf(ibuf, "%d %s %f %f %f", &index, coord, &lat, &lon, &rad) == 5)
    {
        printf("image position(%d, %s, %f, %f, %f)\n",
               index, coord, lat, lon, rad);
        state = 1;
    }

    if (obuf) sprintf(obuf, "%d\n", state);
}

void uni::overlay::m_image_rotation(const char *ibuf, char *obuf)
{
    int   state = 0;
    int   index;
    float angle;

    if (sscanf(ibuf, "%d %f", &index, &angle) == 2)
    {
        printf("image rotation(%d, %f)\n", index, angle);
        state = 1;
    }

    if (obuf) sprintf(obuf, "%d\n", state);
}

void uni::overlay::m_image_scale(const char *ibuf, char *obuf)
{
    int   state = 0;
    int   index;
    float scale;

    if (sscanf(ibuf, "%d %f", &index, &scale) == 2)
    {
        printf("image scale(%d, %f)\n", index, scale);
        state = 1;
    }

    if (obuf) sprintf(obuf, "%d\n", state);
}

//-----------------------------------------------------------------------------

void uni::overlay::m_capture_color(const char *ibuf, char *obuf)
{
    int   state = 0;
    float lat;
    float lon;
    float rad;
    float fov;
    char  coord[256];

    if (sscanf(ibuf, "%s %f %f %f %f", coord, &lat, &lon, &rad, &fov) == 5)
    {
        printf("capture_color(%s, %f, %f, %f, %f)\n",
               coord, lat, lon, rad, fov);
        state = 1;
    }

    if (obuf) sprintf(obuf, "%d\n", 1);
}

void uni::overlay::m_capture_radius(const char *ibuf, char *obuf)
{
    int   state = 0;
    float lat;
    float lon;
    float rad;
    float fov;
    char  coord[256];

    if (sscanf(ibuf, "%s %f %f %f %f", coord, &lat, &lon, &rad, &fov) == 5)
    {
        printf("capture_radius(%s, %f, %f, %f, %f)\n",
               coord, lat, lon, rad, fov);
        state = 1;
    }

    if (obuf) sprintf(obuf, "%d\n", 1);
}

//-----------------------------------------------------------------------------

void uni::overlay::m_data_show(const char *ibuf, char *obuf)
{
    int  state = 0;
    char name[MAXSTR];
    int  prio;

    if (sscanf(ibuf, "%s %d", name, &prio) == 2)
    {
        printf("data show(%s, %d)\n", name, prio);
        state = 1;
    }

    if (obuf) sprintf(obuf, "%d\n", 1);
}

void uni::overlay::m_data_hide(const char *ibuf, char *obuf)
{
    int  state = 0;
    char name[MAXSTR];
    int  prio;

    if (sscanf(ibuf, "%s %d", name, &prio) == 2)
    {
        printf("data hide(%s, %d)\n", name, prio);
        state = 1;
    }

    if (obuf) sprintf(obuf, "%d\n", 1);
}

//-----------------------------------------------------------------------------

void uni::overlay::script(const char *ibuf, char *obuf)
{
    char key[MAXSTR];
    const char *val;

    sscanf(ibuf, "%s\n", key);
    val = ibuf + strlen(key);

    // Dispatch the message to the proper handler function.

    if      (!strcmp(key, "lookup"))         m_lookup(val, obuf);
    else if (!strcmp(key, "moveto"))         m_moveto(val, obuf);
    else if (!strcmp(key, "get_position"))   m_get_position(val, obuf);

    else if (!strcmp(key, "model_create"))   m_model_create  (val, obuf);
    else if (!strcmp(key, "model_delete"))   m_model_delete  (val, obuf);
    else if (!strcmp(key, "model_position")) m_model_position(val, obuf);
    else if (!strcmp(key, "model_rotation")) m_model_rotation(val, obuf);
    else if (!strcmp(key, "model_scale"))    m_model_scale   (val, obuf);

    else if (!strcmp(key, "image_create"))   m_image_create  (val, obuf);
    else if (!strcmp(key, "image_delete"))   m_image_delete  (val, obuf);
    else if (!strcmp(key, "image_position")) m_image_position(val, obuf);
    else if (!strcmp(key, "image_rotation")) m_image_rotation(val, obuf);
    else if (!strcmp(key, "image_scale"))    m_image_scale   (val, obuf);

    else if (!strcmp(key, "capture_color"))  m_capture_color (val, obuf);
    else if (!strcmp(key, "capture_radius")) m_capture_radius(val, obuf);

    else if (!strcmp(key, "data_hide"))      m_data_hide(val, obuf);
    else if (!strcmp(key, "data_show"))      m_data_show(val, obuf);

    else if (obuf) sprintf(obuf, "unrecognized keyword '%s'\n", key);
}

//-----------------------------------------------------------------------------

void uni::overlay::step()
{
}

//-----------------------------------------------------------------------------

uni::overlay::overlay()
{
}

uni::overlay::~overlay()
{
}

//-----------------------------------------------------------------------------
