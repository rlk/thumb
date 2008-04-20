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

#include "app-message.hpp"

//-----------------------------------------------------------------------------

app::message::message(unsigned char type) : index(0)
{
    payload.type = type;
    payload.size = 0;
}

void app::message::put_real(double d)
{
    // Append a double to the payload data.

    union swap s;

    s.d = d;

    ((uint32_t *) (payload.data + payload.size))[0] = htonl(s.l[0]);
    ((uint32_t *) (payload.data + payload.size))[1] = htonl(s.l[1]);

    payload.size += 2 * sizeof (uint32_t);
}

double app::message::get_real()
{
    // Return the next double in the payload data.

    union swap s;

    s.l[0] = ntohl(((uint32_t *) (payload.data + index))[0]);
    s.l[1] = ntohl(((uint32_t *) (payload.data + index))[1]);

    index += 2 * sizeof (uint32_t);

    return s.d;
}

void app::message::put_bool(bool b)
{
    payload.data[payload.size++] = (b ? 1 : 0);
}

bool app::message::get_bool()
{
    return payload.data[index++] ? true : false;
}

void app::message::put_byte(int b)
{
    payload.data[payload.size++] = char(b);
}

int app::message::get_byte()
{
    return int(payload.data[index++]);
}

void app::message::put_word(int i)
{
    // Append an int to the payload data.

    union swap s;

    s.i = i;

    ((uint32_t *) (payload.data + payload.size))[0] = htonl(s.l[0]);

    payload.size += sizeof (uint32_t);
}

int app::message::get_word()
{
    // Return the next int in the payload data.

    union swap s;

    s.l[0] = ntohl(((uint32_t *) (payload.data + index))[0]);

    index += sizeof (uint32_t);

    return s.i;
}

//-----------------------------------------------------------------------------

const char *app::message::tag() const
{
    switch (payload.type)
    {
    case E_REPLY: return "reply";
    case E_POINT: return "point";
    case E_CLICK: return "click";
    case E_KEYBD: return "keybd";
    case E_VALUE: return "value";
    case E_TIMER: return "timer";
    case E_PAINT: return "paint";
    case E_FRONT: return "front";
    case E_CLOSE: return "close";
    }
    return "UNKNOWN";
}

void app::message::send(SOCKET s)
{
    // Send the payload on the given socket.

    if (::send(s, (const char *) &payload, payload.size + 2, 0) == -1)
        throw std::runtime_error(strerror(errno));
}

void app::message::recv(SOCKET s)
{
    // Block until receipt of the payload head and data.

    if (::recv(s, (char *) &payload, 2, 0) == -1)
        throw std::runtime_error(strerror(errno));

    // TODO: repeat until payload size is received.

    if (payload.size > 0)
        if (::recv(s,  payload.data, payload.size, 0) == -1)
            throw std::runtime_error(strerror(errno));

    // Reset the unpack pointer to the beginning.

    index = 0;
}

//-----------------------------------------------------------------------------
