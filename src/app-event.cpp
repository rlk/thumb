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
    // Ensure that any existing string buffers are freed.

    if (payload.type == E_INPUT)
    {
        if (data.input.src) free(data.input.src);
        if (data.input.dst) free(data.input.dst);
    }

    data.input.src = 0;
    data.input.dst = 0;

    payload.type = t;
}

unsigned char app::event::get_type() const
{
    return payload.type;
}

//-----------------------------------------------------------------------------

void app::event::put_text(const char *text)
{
    // Set the payload to the given string.

    size_t n = std::min(size_t(DATAMAX - 1), strlen(text));

    memset(payload.data, 0, DATAMAX);
    strncpy(payload.data, text, n);
    payload.size = (unsigned char) n;
}

const char *app::event::get_text()
{
    // Return the payload as a string.

    return payload.data;
}

//-----------------------------------------------------------------------------

void app::event::put_real(double d)
{
    // Append a double to the payload data.

    union swap s;

    s.d = d;

    ((uint32_t *) (payload.data + payload.size))[0] = htonl(s.l[0]);
    ((uint32_t *) (payload.data + payload.size))[1] = htonl(s.l[1]);

    payload.size += 2 * sizeof (uint32_t);
}

double app::event::get_real()
{
    // Return the next double in the payload data.

    union swap s;

    s.l[0] = ntohl(((uint32_t *) (payload.data + payload_index))[0]);
    s.l[1] = ntohl(((uint32_t *) (payload.data + payload_index))[1]);

    payload_index += 2 * sizeof (uint32_t);

    return s.d;
}

//-----------------------------------------------------------------------------

void app::event::put_bool(bool b)
{
    // Append a bool to the payload data.

    payload.data[payload.size++] = (b ? 1 : 0);
}

bool app::event::get_bool()
{
    // Return the next bool in the payload data.

    return payload.data[payload_index++] ? true : false;
}

//-----------------------------------------------------------------------------

void app::event::put_byte(int b)
{
    // Append a byte to the payload data.

    payload.data[payload.size++] = char(b);
}

int app::event::get_byte()
{
    // Return the next byte in the payload data.

    return int(payload.data[payload_index++]);
}

//-----------------------------------------------------------------------------

void app::event::put_word(int i)
{
    // Append an int to the payload data.

    union swap s;

    s.i = i;

    ((uint32_t *) (payload.data + payload.size))[0] = htonl(s.l[0]);

    payload.size += sizeof (uint32_t);
}

int app::event::get_word()
{
    // Return the next int in the payload data.

    union swap s;

    s.l[0] = ntohl(((uint32_t *) (payload.data + payload_index))[0]);

    payload_index += sizeof (uint32_t);

    return s.i;
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
        put_byte(data.click.i);
        put_byte(data.click.b);
        put_word(data.click.m);
        put_bool(data.click.d);
        break;

    case E_KEYBD:
        put_word(data.keybd.c);
        put_word(data.keybd.k);
        put_word(data.keybd.m);
        put_bool(data.keybd.d);
        break;

    case E_VALUE:
        put_byte(data.value.i);
        put_byte(data.value.a);
        put_real(data.value.v);
        break;

    case E_INPUT:
        put_text(data.input.src);
        break;

    case E_TIMER:
        put_word(data.timer.dt);
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

        data.click.i = get_byte();
        data.click.b = get_byte();
        data.click.m = get_word();
        data.click.d = get_bool();
        break;

    case E_KEYBD:

        data.keybd.c = get_word();
        data.keybd.k = get_word();
        data.keybd.m = get_word();
        data.keybd.d = get_bool();
        break;

    case E_VALUE:

        data.value.i = get_byte();
        data.value.a = get_byte();
        data.value.v = get_real();
        break;

    case E_INPUT:

        data.input.src = strdup(get_text());
        break;

    case E_TIMER:

        data.timer.dt = get_word();
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

app::event *app::event::mk_click(int i, int b, int m, bool d)
{
    put_type(E_CLICK);

    data.click.i = i;
    data.click.b = b;
    data.click.m = m;
    data.click.d = d;

    payload_cache = false;
    return this;
}

app::event *app::event::mk_keybd(int c, int k, int m, bool d)
{
    put_type(E_KEYBD);

    data.keybd.c = c;
    data.keybd.k = k;
    data.keybd.m = m;
    data.keybd.d = d;

    payload_cache = false;
    return this;
}

app::event *app::event::mk_value(int i, int a, double v)
{
    put_type(E_VALUE);

    data.value.i = i;
    data.value.a = a;
    data.value.v = v;

    payload_cache = false;
    return this;
}

app::event *app::event::mk_input(const char *s)
{
    put_type(E_INPUT);

    data.input.src = strdup(s);
    data.input.dst = 0;

    payload_cache = false;
    return this;
}

app::event *app::event::mk_timer(int t)
{
    put_type(E_TIMER);

    data.timer.dt = t;

    payload_cache = false;
    return this;
}

app::event *app::event::mk_paint()
{
    put_type(E_PAINT);

    payload_cache = false;
    return this;
}

app::event *app::event::mk_frame()
{
    put_type(E_FRAME);

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

//-----------------------------------------------------------------------------

void app::event::set_dst(const char *str)
{
    assert(payload.type == E_INPUT);

    if (data.input.dst)
        free(data.input.dst);

    if (str)
        data.input.dst = strdup(str);
    else
        data.input.dst = 0;
}
