//  Copyright (C) 2008 Robert Kooima
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

#include "app-event.hpp"

//-----------------------------------------------------------------------------

void app::event::put_type(unsigned char t)
{
    // Ensure that any existing string buffers are freed.

    if (payload.type == E_INPUT)
    {
        if (data.input.src) free(data.input.src);
        if (data.input.dst) free(data.input.dst);
    }

    payload.type = t;
}

unsigned char app::event::get_type()
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

    s.l[0] = ntohl(((uint32_t *) (payload.data + index))[0]);
    s.l[1] = ntohl(((uint32_t *) (payload.data + index))[1]);

    index += 2 * sizeof (uint32_t);

    return s.d;
}

//-----------------------------------------------------------------------------

void app::event::put_bool(bool b)
{
    payload.data[payload.size++] = (b ? 1 : 0);
}

bool app::event::get_bool()
{
    return payload.data[index++] ? true : false;
}

//-----------------------------------------------------------------------------

void app::event::put_byte(int b)
{
    payload.data[payload.size++] = char(b);
}

int app::event::get_byte()
{
    return int(payload.data[index++]);
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

    s.l[0] = ntohl(((uint32_t *) (payload.data + index))[0]);

    index += sizeof (uint32_t);

    return s.i;
}

//-----------------------------------------------------------------------------

app::event::event()
{
    payload.type = E_NULL;
    payload.size = 0;

    memset(payload.data, 0, DATAMAX);
    memset(&data, 0, sizeof (data));
}

app::event::~event()
{
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

    return this;
}

app::event *app::event::mk_click(int i, int b, int m, bool d)
{
    put_type(E_CLICK);

    data.click.i = i;
    data.click.b = b;
    data.click.m = m;
    data.click.d = d;

    return this;
}

app::event *app::event::mk_keybd(int c, int k, int m, bool d)
{
    put_type(E_KEYBD);

    data.keybd.c = c;
    data.keybd.k = k;
    data.keybd.m = m;
    data.keybd.d = d;

    return this;
}

app::event *app::event::mk_value(int i, int a, double v)
{
    put_type(E_VALUE);

    data.value.i = i;
    data.value.a = a;
    data.value.v = v;

    return this;
}

app::event *app::event::mk_input(const char *s)
{
    put_type(E_INPUT);

    data.input.src = strdup(s);

    return this;
}

app::event *app::event::mk_timer(int t)
{
    put_type(E_TIMER);

    data.timer.dt = t;

    return this;
}

app::event *app::event::mk_paint()
{
    put_type(E_PAINT);
    return this;
}

app::event *app::event::mk_frame()
{
    put_type(E_FRAME);
    return this;
}

app::event *app::event::mk_start()
{
    put_type(E_START);
    return this;
}

app::event *app::event::mk_close()
{
    put_type(E_CLOSE);
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

    if (payload.size > 0)
        if (::recv(s,  payload.data, payload.size, 0) == -1)
            throw app::sock_error("recv");

    // Decode the payload.

    index = 0;

    switch (get_type())
    {
    case E_POINT:
    {
        double p[3];
        double q[4];

        int i = get_byte();
        p[0]  = get_real();
        p[1]  = get_real();
        p[2]  = get_real();
        q[0]  = get_real();
        q[1]  = get_real();
        q[2]  = get_real();
        q[3]  = get_real();

        return mk_point(i, p, q);
    }
    case E_CLICK:
    {
        int  i = get_byte();
        int  b = get_byte();
        int  m = get_byte();
        bool d = get_bool();

        return mk_click(i, b, m, d);
    }
    case E_KEYBD:
    {
        int  c = get_word();
        int  k = get_word();
        int  m = get_word();
        bool d = get_bool();

        return mk_keybd(c, k, m, d);
    }
    case E_VALUE:
    {
        int    i = get_byte();
        int    a = get_byte();
        double p = get_real();

        return mk_value(i, a, p);
    }
    case E_INPUT: return mk_input(get_text());
    case E_TIMER: return mk_timer(get_word());
    case E_PAINT: return mk_paint();
    case E_FRAME: return mk_frame();
    case E_START: return mk_start();
    case E_CLOSE: return mk_close();
    default:      return 0;
    }
}

app::event *app::event::send(SOCKET s)
{
    payload.size = 0;

    // Encode the payload.

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
        put_byte(data.click.m);
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

    default:
        break;
    }

    // Send the payload on the given socket.

    if (::send(s, (const char *) &payload, payload.size + 2, 0) == -1)
        throw app::sock_error("send");

    return this;
}

//-----------------------------------------------------------------------------
