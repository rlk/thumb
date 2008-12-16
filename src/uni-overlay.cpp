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

        sprintf(obuf, "%f %f\n", 0.0f, 0.0f);
    }
}

void uni::overlay::m_get_position(const char *ibuf, char *obuf)
{
}

//-----------------------------------------------------------------------------

void uni::overlay::m_model_create(const char *ibuf, char *obuf)
{
    char filename[MAXSTR];

    if (sscanf(ibuf, "%s", filename) == 1)
        printf("model create %s\n", filename);
}

void uni::overlay::m_model_delete(const char *ibuf, char *obuf)
{
}

void uni::overlay::m_model_position(const char *ibuf, char *obuf)
{
}

void uni::overlay::m_model_rotation(const char *ibuf, char *obuf)
{
}

void uni::overlay::m_model_scale(const char *ibuf, char *obuf)
{
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
/*
    else if (!strcmp(key, "image_create"))   m_image_create  (val, obuf);
    else if (!strcmp(key, "image_delete"))   m_image_delete  (val, obuf);
    else if (!strcmp(key, "image_position")) m_image_position(val, obuf);
    else if (!strcmp(key, "image_rotation")) m_image_rotation(val, obuf);
    else if (!strcmp(key, "image_scale"))    m_image_scale   (val, obuf);

    else if (!strcmp(key, "capture_color"))  m_capture_color (val, obuf);
    else if (!strcmp(key, "capture_radius")) m_capture_radius(val, obuf);

    else if (!strcmp(key, "data_hide"))      m_data_hide(val, obuf);
    else if (!strcmp(key, "data_show"))      m_data_show(val, obuf);
*/
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
