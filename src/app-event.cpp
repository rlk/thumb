//  Copyright (C) 2008-2011 Robert Kooima
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
#include <cstdlib>

#include <app-event.hpp>

//-----------------------------------------------------------------------------

void app::event::put_type(unsigned char t)
{
    payload.type = t;
}

unsigned char app::event::get_type() const
{
    return payload.type;
}

//-----------------------------------------------------------------------------

// Append a double to the payload data.

void app::event::put_real(double d)
{
    memcpy(payload.data + payload.size, &d, sizeof (double));
    payload.size += sizeof (double);
}

// Return the next double in the payload data.

double app::event::get_real()
{
    double d;
    memcpy(&d, payload.data + payload_index, sizeof (double));
    payload_index += sizeof (double);
    return d;
}

//-----------------------------------------------------------------------------

// Append a bool to the payload data.

void app::event::put_bool(bool b)
{
    payload.data[payload.size++] = (b ? 1 : 0);
}

// Return the next bool in the payload data.

bool app::event::get_bool()
{
    return (payload.data[payload_index++] == 0) ? false : true;
}

//-----------------------------------------------------------------------------

// Append a byte to the payload data.

void app::event::put_byte(int b)
{
    payload.data[payload.size++] = char(b);
}

// Return the next byte in the payload data.

int app::event::get_byte()
{
    return int(payload.data[payload_index++]);
}

//-----------------------------------------------------------------------------

// Append an int to the payload data.

void app::event::put_word(int i)
{
    memcpy(payload.data + payload.size, &i, sizeof (int));
    payload.size += sizeof (int);
}

// Return the next int in the payload data.

int app::event::get_word()
{
    int i;
    memcpy(&i, payload.data + payload_index, sizeof (int));
    payload_index += sizeof (int);
    return i;
}

//-----------------------------------------------------------------------------

// Append a long long to the payload data.

void app::event::put_long(long long l)
{
    memcpy(payload.data + payload.size, &l, sizeof (long long));
    payload.size += sizeof (long long);
}

// Return the next long long in the payload data.

long long app::event::get_long()
{
    long long l;
    memcpy(&l, payload.data + payload_index, sizeof (long long));
    payload_index += sizeof (long long);
    return l;
}

//-----------------------------------------------------------------------------

void app::event::payload_encode()
{
    // Encode the event data in the payload buffer.

    payload_cache = true;
    payload.size  = 0;

    switch (get_type())
    {
    case E_POINT:

        put_byte(data.point.i);
        put_real(data.point.p[0]);
        put_real(data.point.p[1]);
        put_real(data.point.p[2]);
        put_real(data.point.q[0]);
        put_real(data.point.q[1]);
        put_real(data.point.q[2]);
        put_real(data.point.q[3]);
        break;

    case E_CLICK:

        put_byte(data.click.b);
        put_word(data.click.m);
        put_bool(data.click.d);
        break;

    case E_KEY:

        put_word(data.key.c);
        put_word(data.key.k);
        put_word(data.key.m);
        put_bool(data.key.d);
        break;

    case E_AXIS:

        put_byte(data.axis.i);
        put_byte(data.axis.a);
        put_real(data.axis.v);
        break;

    case E_BUTTON:

        put_byte(data.button.i);
        put_byte(data.button.b);
        put_bool(data.button.d);
        break;

    case E_USER:

        put_long(data.user.d);
        break;

    case E_TICK:

        put_real(data.tick.dt);
        break;
    }
}

void app::event::payload_decode()
{
    // Decode the event data from the payload data.

    payload_cache = true;
    payload_index = 0;

    switch (get_type())
    {
    case E_POINT:

        data.point.i    = get_byte();
        data.point.p[0] = get_real();
        data.point.p[1] = get_real();
        data.point.p[2] = get_real();
        data.point.q[0] = get_real();
        data.point.q[1] = get_real();
        data.point.q[2] = get_real();
        data.point.q[3] = get_real();
        break;

    case E_CLICK:

        data.click.b = get_byte();
        data.click.m = get_word();
        data.click.d = get_bool();
        break;

    case E_KEY:

        data.key.c = get_word();
        data.key.k = get_word();
        data.key.m = get_word();
        data.key.d = get_bool();
        break;

    case E_AXIS:

        data.axis.i = get_byte();
        data.axis.a = get_byte();
        data.axis.v = get_real();
        break;

    case E_BUTTON:

        data.button.i = get_byte();
        data.button.b = get_byte();
        data.button.d = get_bool();
        break;

    case E_USER:

        data.user.d = get_long();
        break;

    case E_TICK:

        data.tick.dt = get_real();
        break;
    }
}

//-----------------------------------------------------------------------------

app::event::event() :
    payload_cache(false),
    payload_index(0)
{
    payload.type = E_NULL;
    payload.size = 0;

    memset(payload.data, 0, DATAMAX);
    memset(&data, 0, sizeof (data));
}

app::event::~event()
{
    put_type(E_NULL);
}

//-----------------------------------------------------------------------------

app::event *app::event::mk_point(int i, const double *p, const double *q)
{
    put_type(E_POINT);

    data.point.i    = i;
    data.point.p[0] = p[0];
    data.point.p[1] = p[1];
    data.point.p[2] = p[2];
    data.point.q[0] = q[0];
    data.point.q[1] = q[1];
    data.point.q[2] = q[2];
    data.point.q[3] = q[3];

    payload_cache = false;
    return this;
}

app::event *app::event::mk_click(int b, int m, bool d)
{
    put_type(E_CLICK);

    data.click.b = b;
    data.click.m = m;
    data.click.d = d;

    payload_cache = false;
    return this;
}

app::event *app::event::mk_key(int c, int k, int m, bool d)
{
    put_type(E_KEY);

    data.key.c = c;
    data.key.k = k;
    data.key.m = m;
    data.key.d = d;

    payload_cache = false;
    return this;
}

app::event *app::event::mk_axis(int i, int a, double v)
{
    put_type(E_AXIS);

    data.axis.i = i;
    data.axis.a = a;
    data.axis.v = v;

    payload_cache = false;
    return this;
}

app::event *app::event::mk_button(int i, int b, bool d)
{
    put_type(E_BUTTON);

    data.button.i = i;
    data.button.b = b;
    data.button.d = d;

    payload_cache = false;
    return this;
}

app::event *app::event::mk_tick(double dt)
{
    put_type(E_TICK);

    data.tick.dt = dt;

    payload_cache = false;
    return this;
}

app::event *app::event::mk_draw()
{
    put_type(E_DRAW);

    payload_cache = false;
    return this;
}

app::event *app::event::mk_swap()
{
    put_type(E_SWAP);

    payload_cache = false;
    return this;
}

app::event *app::event::mk_user(long long d)
{
    put_type(E_USER);

    data.user.d = d;

    payload_cache = false;
    return this;
}

app::event *app::event::mk_start()
{
    put_type(E_START);

    payload_cache = false;
    return this;
}

app::event *app::event::mk_close()
{
    put_type(E_CLOSE);

    payload_cache = false;
    return this;
}

app::event *app::event::mk_flush()
{
    put_type(E_FLUSH);

    payload_cache = false;
    return this;
}

//-----------------------------------------------------------------------------

app::event *app::event::recv(SOCKET s)
{
    // Null any existing payload.

    put_type(E_NULL);

    // Block until receipt of the payload head and data.

    if (::recv(s, (char *) &payload, 2, 0) == -1)
        throw app::sock_error("recv");

    // TODO: repeat until payload size is received.

    memset(payload.data, 0, DATAMAX);

    if (payload.size > 0)
        if (::recv(s,  payload.data, payload.size, 0) == -1)
            throw app::sock_error("recv");

    // Decode the payload.

    payload_decode();

    return this;
}

app::event *app::event::send(SOCKET s)
{
    // Encode the payload, if necessary.

    if (payload_cache == false)
        payload_encode();

    // Send the payload on the given socket.

    if (::send(s, (const char *) &payload, payload.size + 2, 0) == -1)
        throw app::sock_error("send");

    return this;
}

std::string app::event::name()
{
    switch (payload.type)
    {
        case E_NULL:   return "NULL";
        case E_POINT:  return "POINT";
        case E_CLICK:  return "CLICK";
        case E_KEY:    return "KEY";
        case E_AXIS:   return "AXIS";
        case E_BUTTON: return "BUTTON";
        case E_TICK:   return "TICK";
        case E_DRAW:   return "DRAW";
        case E_SWAP:   return "SWAP";
        case E_USER:   return "USER";
        case E_START:  return "START";
        case E_CLOSE:  return "CLOSE";
        case E_FLUSH:  return "FLUSH";
        default:       return "UNKNOWN";
    }
}

//-----------------------------------------------------------------------------
