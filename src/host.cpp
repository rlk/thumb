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

#include <SDL.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include <string.h>
#include <errno.h>

#include "tracker.hpp"
#include "opengl.hpp"
#include "matrix.hpp"
#include "perf.hpp"
#include "data.hpp"
#include "conf.hpp"
#include "glob.hpp"
#include "view.hpp"
#include "host.hpp"
#include "perf.hpp"
#include "prog.hpp"

#define JIFFY (1000 / 60)

#define MXML_FORALL(t, i, n) \
    for (i = mxmlFindElement((t), (t), (n), 0, 0, MXML_DESCEND); i; \
         i = mxmlFindElement((i), (t), (n), 0, 0, MXML_NO_DESCEND))

//=============================================================================

class host_error : public std::runtime_error
{
    std::string mesg(const char *s) {
        return std::string(s) + ": " + hstrerror(h_errno);
    }

public:
    host_error(const char *s) : std::runtime_error(mesg(s)) { }
};

class sock_error : public std::runtime_error
{
    std::string mesg(const char *s) {
        return std::string(s) + ": " + strerror(errno);
    }

public:
    sock_error(const char *s) : std::runtime_error(mesg(s)) { }
};

//-----------------------------------------------------------------------------

#define E_REPLY 0
#define E_POINT 1
#define E_CLICK 2
#define E_KEYBD 3
#define E_TIMER 4
#define E_TRACK 5
#define E_STICK 6
#define E_PAINT 7
#define E_FLEEP 8
#define E_CLOSE 9

//-----------------------------------------------------------------------------

app::message::message(unsigned char type) : index(0)
{
    payload.type = type;
    payload.size = 0;
}

void app::message::put_double(double d)
{
    // Append a double to the payload data.

    union swap s;

    s.d = d;

    ((uint32_t *) (payload.data + payload.size))[0] = htonl(s.l[0]);
    ((uint32_t *) (payload.data + payload.size))[1] = htonl(s.l[1]);

    payload.size += 2 * sizeof (uint32_t);
}

double app::message::get_double()
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
    return payload.data[index++];
}

void app::message::put_int(int i)
{
    // Append an int to the payload data.

    union swap s;

    s.i = i;

    ((uint32_t *) (payload.data + payload.size))[0] = htonl(s.l[0]);

    payload.size += sizeof (uint32_t);
}

int app::message::get_int()
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
    case E_TIMER: return "timer";
    case E_TRACK: return "track";
    case E_STICK: return "stick";
    case E_PAINT: return "paint";
    case E_FLEEP: return "fleep";
    case E_CLOSE: return "close";
    }
    return "UNKNOWN";
}

void app::message::send(SOCKET s)
{
    // Send the payload on the given socket.

    if (::send(s, &payload, payload.size + 2, 0) == -1)
        throw std::runtime_error(strerror(errno));
}

void app::message::recv(SOCKET s)
{
    // Block until receipt of the payload head and data.

    if (::recv(s, &payload, 2, 0) == -1)
        throw std::runtime_error(strerror(errno));

    // TODO: repeat until payload size is received.

    if (payload.size > 0)
        if (::recv(s,  payload.data, payload.size, 0) == -1)
            throw std::runtime_error(strerror(errno));

    // Reset the unpack pointer to the beginning.

    index = 0;
}

//=============================================================================

app::eye::eye(mxml_node_t *node, int w, int h) : w(w), h(h), back(0)
{
    // Set some reasonable defaults.

    V[0] = P[0] = 0;
    V[1] = P[1] = 0;
    V[2] = P[2] = 0;

    color[0] = color[1] = color[2] = 0xFF;

    // If we have an XML configuration node...

    if (node)
    {
        const char *c;

        if ((c = mxmlElementGetAttr(node, "x"))) V[0] = P[0] = atof(c);
        if ((c = mxmlElementGetAttr(node, "y"))) V[1] = P[1] = atof(c);
        if ((c = mxmlElementGetAttr(node, "z"))) V[2] = P[2] = atof(c);

        if ((c = mxmlElementGetAttr(node, "r"))) color[0] = atof(c);
        if ((c = mxmlElementGetAttr(node, "g"))) color[1] = atof(c);
        if ((c = mxmlElementGetAttr(node, "b"))) color[2] = atof(c);
    }
}

app::eye::~eye()
{
//  if (back) ::glob->free_frame(back);
}

void app::eye::set_head(const double *p,
                        const double *x,
                        const double *y,
                        const double *z)
{
    // Cache the eye positions in the head's coordinate system.

    P[0] = p[0] + V[0] * x[0] + V[1] * y[0] + V[2] * z[0];
    P[1] = p[1] + V[0] * x[1] + V[1] * y[1] + V[2] * z[1];
    P[2] = p[2] + V[0] * x[2] + V[1] * y[2] + V[2] * z[2];
}

void app::eye::draw(const int *rect, bool focus)
{
    double frag_d[2] = { 0, 0 };
    double frag_k[2] = { 1, 1 };

    // Set the eye position.

    ::view->set_P(P);

    // If we are rendering directly on-screen...

    if (::view->get_type() == view::type_mono)
    {
        // Compute the on-screen to off-screen fragment transform.

        frag_d[0] =            -double(rect[0]);
        frag_d[1] =            -double(rect[1]);
        frag_k[0] = double(w) / double(rect[2]);
        frag_k[1] = double(h) / double(rect[3]);

        // Draw the scene.

        glViewport(rect[0], rect[1], rect[2], rect[3]);

        ::prog->draw(frag_d, frag_k);
    }
    else
    {
        // Make sure the off-screen buffer exists.

        if (back == 0)
            back = ::glob->new_frame(w, h, GL_TEXTURE_RECTANGLE_ARB,
                                           GL_RGBA8, true, false);
        if (back)
        {
            back->bind();
            {
                if (::view->get_mode() == view::mode_norm)
                {
                    // Draw the scene normally.

                    glClear(GL_COLOR_BUFFER_BIT);
                    ::prog->draw(frag_d, frag_k);
                }
                if (::view->get_mode() == view::mode_test)
                {
                    float r = color[0] * (focus ? 1.0f : 0.5f);
                    float g = color[1] * (focus ? 1.0f : 0.5f);
                    float b = color[2] * (focus ? 1.0f : 0.5f);

                    // Draw the test pattern.

                    glPushAttrib(GL_COLOR_BUFFER_BIT);
                    glClearColor(r, g, b, 1);
                    glClear(GL_COLOR_BUFFER_BIT);
                    glPopAttrib();
                }
            }
            back->free();
        }
    }
}

//=============================================================================

app::tile::tile(mxml_node_t *node) : eye_index(-1), varrier(0)
{
    double a = double(DEFAULT_PIXEL_WIDTH) / double(DEFAULT_PIXEL_HEIGHT);

    bool got_BL = false;
    bool got_BR = false;
    bool got_TL = false;
    bool got_TR = false;

    // Set some reasonable defaults.

    BL[0] = -0.5 * a; BL[1] = -0.5; BL[2] = -1.0;
    BR[0] = +0.5 * a; BR[1] = -0.5; BR[2] = -1.0;
    TL[0] = -0.5 * a; TL[1] = +0.5; TL[2] = -1.0;
    TR[0] = +0.5 * a; TL[1] = +0.5; TL[2] = -1.0;

    window_rect[0] = 0;
    window_rect[1] = 0;
    window_rect[2] = DEFAULT_PIXEL_WIDTH;
    window_rect[3] = DEFAULT_PIXEL_HEIGHT;

    varrier_pitch = 200.0000;
    varrier_angle =   0.0000;
    varrier_thick =   0.0160;
    varrier_shift =   0.0000;
    varrier_cycle =   0.8125;

    // If we have an XML configuration node...

    if (node)
    {
        mxml_node_t *elem;
        mxml_node_t *curr;

        const char *c;

        // Extract the window viewport rectangle.

        if ((elem = mxmlFindElement(node, node, "viewport",
                                    0, 0, MXML_DESCEND_FIRST)))
        {
            if ((c = mxmlElementGetAttr(elem, "x"))) window_rect[0] = atoi(c);
            if ((c = mxmlElementGetAttr(elem, "y"))) window_rect[1] = atoi(c);
            if ((c = mxmlElementGetAttr(elem, "w"))) window_rect[2] = atoi(c);
            if ((c = mxmlElementGetAttr(elem, "h"))) window_rect[3] = atoi(c);
        }

        // Extract the screen corners.

        MXML_FORALL(elem, curr, "corner")
        {
            double *v = 0;

            // Determine which corner is being specified.

            if (const char *name = mxmlElementGetAttr(curr, "name"))
            {
                if      (strcmp(name, "BL") == 0) { v = BL; got_BL = true; }
                else if (strcmp(name, "BR") == 0) { v = BR; got_BR = true; }
                else if (strcmp(name, "TL") == 0) { v = TL; got_TL = true; }
                else if (strcmp(name, "TR") == 0) { v = TR; got_TR = true; }
            }
            if (v)
            {
                // Extract the position.

                if ((c = mxmlElementGetAttr(curr, "x"))) v[0] = atof(c);
                if ((c = mxmlElementGetAttr(curr, "y"))) v[1] = atof(c);
                if ((c = mxmlElementGetAttr(curr, "z"))) v[2] = atof(c);

                // Convert dimensions as necessary.

                if ((c = mxmlElementGetAttr(curr, "dim")))
                {
                    if (std::string(c) == "mm")
                    {
                        v[0] /= 304.8;
                        v[1] /= 304.8;
                        v[2] /= 304.8;
                    }
                }
            }
        }

        // Check for an eye specifier.

        if ((c = mxmlElementGetAttr(node, "eye"))) eye_index = atoi(c);

        // Extract the Varrier linescreen parameters.

        if ((varrier = mxmlFindElement(node, node, "varrier",
                                       0, 0, MXML_DESCEND_FIRST)))
        {
            if ((c = mxmlElementGetAttr(varrier, "p"))) varrier_pitch =atof(c);
            if ((c = mxmlElementGetAttr(varrier, "a"))) varrier_angle =atof(c);
            if ((c = mxmlElementGetAttr(varrier, "t"))) varrier_thick =atof(c);
            if ((c = mxmlElementGetAttr(varrier, "s"))) varrier_shift =atof(c);
            if ((c = mxmlElementGetAttr(varrier, "c"))) varrier_cycle =atof(c);
        }
    }

    // Compute the remaining screen corner.

    if (got_TR == false)
    {
        TR[0] = BR[0] + TL[0] - BL[0];
        TR[1] = BR[1] + TL[1] - BL[1];
        TR[2] = BR[2] + TL[2] - BL[2];
    }

    // Compute the screen extents.

    W = distance(BR, BL);
    H = distance(TL, BL);
}

//-----------------------------------------------------------------------------

void app::tile::apply_varrier(const double *P) const
{
    double R[3];
    double U[3];
    double N[3];
    double v[3];
    double w[3];

    double dx, dy;
    double pp, ss;

    // Compute the screen space basis.

    R[0] = BR[0] - BL[0];
    R[1] = BR[1] - BL[1];
    R[2] = BR[2] - BL[2];

    U[0] = TL[0] - BL[0];
    U[1] = TL[1] - BL[1];
    U[2] = TL[2] - BL[2];

    crossprod(N, R, U);

    normalize(R);
    normalize(U);
    normalize(N);

    // Find the vector from the center of the screen to the eye.

    v[0] = P[0] - (TL[0] + BR[0]) * 0.5;
    v[1] = P[1] - (TL[1] + BR[1]) * 0.5;
    v[2] = P[2] - (TL[2] + BR[2]) * 0.5;

    // Transform this vector into screen space.

    w[0] = v[0] * R[0] + v[1] * R[1] + v[2] * R[2];
    w[1] = v[0] * U[0] + v[1] * U[1] + v[2] * U[2];
    w[2] = v[0] * N[0] + v[1] * N[1] + v[2] * N[2];

    // Compute the parallax due to optical thickness.

    dx = varrier_thick * w[0] / w[2];
    dy = varrier_thick * w[1] / w[2];

    // Compute the pitch and shift reduction due to optical thickness.

    pp = varrier_pitch * (w[2] - varrier_thick) / w[2];
    ss = varrier_shift * (w[2] - varrier_thick) / w[2];

    // Compose the line screen transformation matrix.

    glMatrixMode(GL_TEXTURE);
    {
        glLoadIdentity();
        glScaled(pp, pp, 1);
        glRotated(-varrier_angle, 0, 0, 1);
        glTranslated(dx - ss, dy, 0);
    }
    glMatrixMode(GL_MODELVIEW);
}

static void set_attr(mxml_node_t *node, const char *name, double k)
{
    std::ostringstream value;
    
    value << std::right << std::setw(8) << std::setprecision(5) << k;

    mxmlElementSetAttr(node, name, value.str().c_str());
}

void app::tile::set_varrier_pitch(double d)
{
    varrier_pitch += d;

    if (varrier) set_attr(varrier, "p", varrier_pitch);
}

void app::tile::set_varrier_angle(double d)
{
    varrier_angle += d;

    if (varrier) set_attr(varrier, "a", varrier_angle);
}

void app::tile::set_varrier_shift(double d)
{
    varrier_shift += d;

    if (varrier) set_attr(varrier, "s", varrier_shift);
}

void app::tile::set_varrier_thick(double d)
{
    varrier_thick += d;

    if (varrier) set_attr(varrier, "t", varrier_thick);
}

//-----------------------------------------------------------------------------

bool app::tile::pick(double *p, double *v, int x, int y)
{
    // Determine whether the given pointer position lies within this tile.

    if (window_rect[0] <= x && x < window_rect[0] + window_rect[2] &&
        window_rect[1] <= y && y < window_rect[1] + window_rect[3])
    {
        double kx = double(x - window_rect[0]) / double(window_rect[2]);
        double ky = double(y - window_rect[1]) / double(window_rect[3]);

        // It does.  Compute the eye-space vector given by the pointer.

        p[0] = 0.0;
        p[1] = 0.0;
        p[2] = 0.0;

        v[0] = TL[0] * (1 - kx - ky) + TR[0] * kx + BL[0] * ky;
        v[1] = TL[1] * (1 - kx - ky) + TR[1] * kx + BL[1] * ky;
        v[2] = TL[2] * (1 - kx - ky) + TR[2] * kx + BL[2] * ky;

        normalize(v);

        return true;
    }
    return false;
}

void app::tile::draw(std::vector<eye *>& eyes, bool focus)
{
    // Apply the tile corners.

    ::view->set_V(BL, BR, TL, TR);

    // Render the view from each eye.

    std::vector<eye *>::iterator i;
    int e;

    for (e = 0, i = eyes.begin(); i != eyes.end(); ++i, ++e)
        if (eye_index < 0 || eye_index == e)
            (*i)->draw(window_rect, focus);

    // Render the onscreen exposure.

    if (const ogl::program *prog = ::view->get_prog())
    {
        double frag_d[2] = { 0, 0 };
        double frag_k[2] = { 1, 1 };

        int w = ::host->get_buffer_w();
        int h = ::host->get_buffer_h();
        int t;

        // Bind the eye buffers.  Apply the Varrier transform for each.

        for (t = 0, i = eyes.begin(); i != eyes.end(); ++i, ++t)
        {
            (*i)->bind(GL_TEXTURE0 + t);
        
            glActiveTextureARB(GL_TEXTURE0 + t);
            apply_varrier((*i)->get_P());
            glActiveTextureARB(GL_TEXTURE0);
        }

        // Compute the on-screen to off-screen fragment transform.

        frag_d[0] =            -double(window_rect[0]);
        frag_d[1] =            -double(window_rect[1]);
        frag_k[0] = double(w) / double(window_rect[2]);
        frag_k[1] = double(h) / double(window_rect[3]);

        // Draw a tile-filling rectangle.

        prog->bind();
        {
            prog->uniform("L_map", 0);
            prog->uniform("R_map", 1);

            prog->uniform("cycle", varrier_cycle);
            prog->uniform("offset", -W / (3 * window_rect[2]), 0,
                                     W / (3 * window_rect[2]));

            prog->uniform("frag_d", frag_d[0], frag_d[1]);
            prog->uniform("frag_k", frag_k[0], frag_k[1]);

            glViewport(window_rect[0], window_rect[1],
                       window_rect[2], window_rect[3]);

            glMatrixMode(GL_PROJECTION);
            {
                glLoadIdentity();
                glOrtho(-W / 2, +W / 2, -H / 2, +H / 2, -1, +1);
            }
            glMatrixMode(GL_MODELVIEW);
            {
                glLoadIdentity();
            }

            glBegin(GL_QUADS);
            {
                glVertex2f(-W / 2, -H / 2);
                glVertex2f(+W / 2, -H / 2);
                glVertex2f(+W / 2, +H / 2);
                glVertex2f(-W / 2, +H / 2);
            }
            glEnd();
        }
        prog->free();

        // Free the eye buffers...

        for (t = GL_TEXTURE0, i = eyes.begin(); i != eyes.end(); ++i, ++t)
            (*i)->free(t);
    }
}

//=============================================================================

static const char *save_cb(mxml_node_t *node, int where)
{
    std::string name(node->value.element.name);

    switch (where)
    {
    case MXML_WS_AFTER_OPEN:  return "\n";
    case MXML_WS_AFTER_CLOSE: return "\n";
        
    case MXML_WS_BEFORE_OPEN:
        if      (name == "eye")      return "  ";
        else if (name == "gui")      return "  ";
        else if (name == "node")     return "  ";
        else if (name == "buffer")   return "  ";

        else if (name == "tile")     return "    ";
        else if (name == "window")   return "    ";

        else if (name == "viewport") return "      ";
        else if (name == "varrier")  return "      ";
        else if (name == "corner")   return "      ";
        break;

    case MXML_WS_BEFORE_CLOSE:
        if      (name == "gui")      return "  ";
        else if (name == "node")     return "  ";
        else if (name == "tile")     return "    ";
        break;
    }

    return NULL;
}

void app::host::save()
{
    if (dirty)
    {
        char *buff;

        if ((buff = mxmlSaveAllocString(head, save_cb)))
        {
            ::data->save(file, buff);
            free(buff);
        }

        dirty = false;
    }
}

void app::host::load(std::string& tag)
{
    if (head) mxmlDelete(head);

    head = 0;
    node = 0;

    const char *buff;

    try
    {
        // Load the named host config file.

        if ((buff = (const char *) ::data->load(file)))
        {
            head = mxmlLoadString(NULL, buff, MXML_TEXT_CALLBACK);
            node = mxmlFindElement(head, head, "node", "name",
                                   tag.c_str(), MXML_DESCEND);
        }
        ::data->free(file);
    }
    catch (app::find_error& e)
    {
        // If the host config file is missing, rely upon defaults.
    }
}

//-----------------------------------------------------------------------------

static unsigned long lookup(const char *hostname)
{
    struct hostent *H;
    struct in_addr  A;

    if ((H = gethostbyname(hostname)) == 0)
        throw host_error(hostname);

    memcpy(&A.s_addr, H->h_addr_list[0], H->h_length);

    return A.s_addr;
}

static void nodelay(int sd)
{
    socklen_t len = sizeof (int);
    int       val = 1;
        
    if (setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &val, len) < 0)
        throw std::runtime_error(strerror(errno));
}

//-----------------------------------------------------------------------------

void app::host::fork_client(const char *addr, const char *name)
{
    const char *args[4];
    char line[256];

    if ((fork() == 0))
    {
        std::string dir = ::conf->get_s("exec_dir");

        sprintf(line, "cd %s; ./thumb %s", dir.c_str(), name);

        // Allocate and build the client's ssh command line.

        args[0] = "ssh";
        args[1] = addr;
        args[2] = line;
        args[3] = NULL;

//      printf("ssh %s %s\n", addr, line);

        execvp("ssh", (char * const *) args);

        exit(0);
    }
}

//-----------------------------------------------------------------------------

void app::host::init_listen()
{
    const char *port = mxmlElementGetAttr(node, "port");

    // If we have a port assignment then we must listen on it.

    if (port)
    {
        socklen_t  addresslen = sizeof (sockaddr_t);
        sockaddr_t address;

        address.sin_family      = AF_INET;
        address.sin_port        = htons(atoi(port));
        address.sin_addr.s_addr = INADDR_ANY;

        // Create a socket, set no-delay, bind the port, and listen.

        if ((listen_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            throw std::runtime_error(strerror(errno));
        
        nodelay(listen_sd);

        while (bind(listen_sd, (struct sockaddr *) &address, addresslen) < 0)
            if (errno == EINVAL)
            {
                std::cerr << "Waiting for port expiration" << std::endl;
                usleep(250000);
            }
            else throw std::runtime_error(strerror(errno));

        listen(listen_sd, 16);
    }
}

void app::host::poll_listen()
{
    // NOTE: This must not occur between a host::send/host::recv pair.

    if (listen_sd != INVALID_SOCKET)
    {
        struct timeval tv = { 0, 0 };

        fd_set sd;
        
        FD_ZERO(&sd);
        FD_SET(listen_sd, &sd);

        // Check for an incomming client connection.

        if (int n = select(listen_sd + 1, &sd, NULL, NULL, &tv))
        {
            if (n < 0)
            {
                if (errno != EINTR)
                    throw sock_error("select");
            }
            else
            {
                // Accept the connection.

                if (int sd = accept(listen_sd, 0, 0))
                {
                    if (sd < 0)
                        throw sock_error("accept");
                    else
                    {
                        nodelay(sd);
                        client_sd.push_back(sd);
                    }
                }
            }
        }
    }
}

void app::host::fini_listen()
{
    if (listen_sd != INVALID_SOCKET)
        ::close(listen_sd);

    listen_sd = INVALID_SOCKET;
}

//-----------------------------------------------------------------------------

void app::host::init_server()
{
    // If we have a server assignment then we must connect to it.

    if (mxml_node_t *server = mxmlFindElement(node, node, "server",
                                              0, 0, MXML_DESCEND_FIRST))
    {
        socklen_t  addresslen = sizeof (sockaddr_t);
        sockaddr_t address;

        const char *addr = mxmlElementGetAttr(server, "addr");
        const char *port = mxmlElementGetAttr(server, "port");

        // Look up the given host name.

        address.sin_family      = AF_INET;
        address.sin_port        = htons (atoi(port ? port : DEFAULT_PORT));
        address.sin_addr.s_addr = lookup(     addr ? addr : DEFAULT_HOST);

        // Create a socket and connect.

        if ((server_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            throw sock_error(addr);

        while (connect(server_sd, (struct sockaddr *) &address, addresslen) <0)
            if (errno == ECONNREFUSED)
            {
                std::cerr << "Waiting for " << addr << std::endl;
                usleep(250000);
            }
            else throw sock_error(addr);

        nodelay(server_sd);
    }
}

void app::host::fini_server()
{
    if (server_sd != INVALID_SOCKET)
        ::close(server_sd);

    server_sd = INVALID_SOCKET;
}

//-----------------------------------------------------------------------------

void app::host::init_client()
{
    mxml_node_t *curr;

    // Launch all client processes.

    MXML_FORALL(node, curr, "client")
    {
        const char *name = mxmlElementGetAttr(curr, "name");
        const char *addr = mxmlElementGetAttr(curr, "addr");

        fork_client(addr, name);
    }
}

void app::host::fini_client()
{
    for (unsigned int i = 0; i < client_sd.size(); ++i)
    {
        char c;

        // Wait for EOF (orderly shutdown by the remote).

        while (::recv(client_sd[i], &c, 1, 0) > 0)
            ;

        // Close the socket.

        ::close(client_sd[i]);
    }
}

//-----------------------------------------------------------------------------

static void overlay(mxml_node_t *node, double *M, double *I, int w, int h)
{
    double a = double(DEFAULT_PIXEL_WIDTH) / double(DEFAULT_PIXEL_HEIGHT);
    const char *c;

    double BL[3];
    double BR[3];
    double TL[3];

    BL[0] = -0.5 * a; BL[1] = -0.5; BL[2] = -1.0;
    BR[0] = +0.5 * a; BR[1] = -0.5; BR[2] = -1.0;
    TL[0] = -0.5 * a; TL[1] = +0.5; TL[2] = -1.0;

    // Extract overlay corners from the given node.

    mxml_node_t *curr;

    MXML_FORALL(node, curr, "corner")
    {
        double *v = 0;

        if (const char *name = mxmlElementGetAttr(curr, "name"))
        {
            if      (strcmp(name, "BL") == 0) v = BL;
            else if (strcmp(name, "BR") == 0) v = BR;
            else if (strcmp(name, "TL") == 0) v = TL;
        }
        if (v)
        {
            if ((c = mxmlElementGetAttr(curr, "x"))) v[0] = atof(c);
            if ((c = mxmlElementGetAttr(curr, "y"))) v[1] = atof(c);
            if ((c = mxmlElementGetAttr(curr, "z"))) v[2] = atof(c);
        }
    }

    // Compose a transform taking overlay coordinates to eye coordinates.

    double G[16];
    double R[3];
    double U[3];
    double B[3];

    R[0] = BR[0] - BL[0];
    R[1] = BR[1] - BL[1];
    R[2] = BR[2] - BL[2];

    U[0] = TL[0] - BL[0];
    U[1] = TL[1] - BL[1];
    U[2] = TL[2] - BL[2];

    crossprod(B, R, U);
    set_basis(G, R, U, B);

    load_idt(M);

    Rmul_xlt_mat(M, BL[0], BL[1], BL[2]);
    mult_mat_mat(M, M, G);
    Rmul_scl_inv(M, w, h, 1);

    if (I) load_inv(I, M);
}

app::host::host(std::string file,
                std::string tag) :
    server_sd(INVALID_SOCKET),
    listen_sd(INVALID_SOCKET),
    mods(0),
    logo_text(0),
    file(file),
    head(0),
    node(0),
    dirty(false),
    varrier_index(0)
{
    const char *c;

    // Set some reasonable defaults.

    window_rect[0] = 0;
    window_rect[1] = 0;
    window_rect[2] = DEFAULT_PIXEL_WIDTH;
    window_rect[3] = DEFAULT_PIXEL_HEIGHT;

    buffer_w = DEFAULT_PIXEL_WIDTH;
    buffer_h = DEFAULT_PIXEL_HEIGHT;

    gui_w = DEFAULT_PIXEL_WIDTH;
    gui_h = DEFAULT_PIXEL_HEIGHT;

    load_idt(gui_M);
    load_idt(gui_I);
    load_idt(tag_M);

    // Read host.xml and configure using tag match.

    load(tag);

    if (head)
    {
        mxml_node_t *curr;

        // Extract GUI overlay configuration.

        if (mxml_node_t *gui = mxmlFindElement(head, head, "gui",
                                               0, 0, MXML_DESCEND))
        {
            if (const char *w = mxmlElementGetAttr(gui, "w")) gui_w = atoi(w);
            if (const char *h = mxmlElementGetAttr(gui, "h")) gui_h = atoi(h);

            overlay(gui, gui_M, gui_I, gui_w, gui_h);
        }

        // Extract tag overlay configuration.

        if (mxml_node_t *tag = mxmlFindElement(head, head, "tag",
                                               0, 0, MXML_DESCEND))
        {
            if (const char *name = mxmlElementGetAttr(tag, "name"))
                logo_name = std::string(name);

            overlay(tag, tag_M, 0, 1, 1);
        }

        // Extract global off-screen buffer configuration.

        if (mxml_node_t *buffer = mxmlFindElement(head, head, "buffer",
                                                  0, 0, MXML_DESCEND))
        {
            if ((c = mxmlElementGetAttr(buffer, "w"))) buffer_w = atoi(c);
            if ((c = mxmlElementGetAttr(buffer, "h"))) buffer_h = atoi(c);
        }

        // Extract eye config parameters

        MXML_FORALL(head, curr, "eye")
            eyes.push_back(new eye(curr, buffer_w, buffer_h));
    }

    if (node)
    {
        mxml_node_t *curr;

        // Extract the window parameters.

        if (mxml_node_t *window = mxmlFindElement(node, node, "window",
                                                  0, 0, MXML_DESCEND_FIRST))
        {
            if ((c = mxmlElementGetAttr(window, "x"))) window_rect[0] =atoi(c);
            if ((c = mxmlElementGetAttr(window, "y"))) window_rect[1] =atoi(c);
            if ((c = mxmlElementGetAttr(window, "w"))) window_rect[2] =atoi(c);
            if ((c = mxmlElementGetAttr(window, "h"))) window_rect[3] =atoi(c);
        }

        // Extract local off-screen buffer configuration.

        if (mxml_node_t *buffer = mxmlFindElement(node, node, "buffer",
                                                  0, 0, MXML_DESCEND_FIRST))
        {
            if ((c = mxmlElementGetAttr(buffer, "w"))) buffer_w = atoi(c);
            if ((c = mxmlElementGetAttr(buffer, "h"))) buffer_h = atoi(c);
        }

        // Extract tile config parameters.

        MXML_FORALL(node, curr, "tile")
            tiles.push_back(tile(curr));

        // Start the network syncronization.

        init_listen();
        init_server();
        init_client();
    }

    // If no eyes or tiles were defined, instance defaults.

    if ( eyes.empty())  eyes.push_back(new eye(0, buffer_w, buffer_h));
    if (tiles.empty()) tiles.push_back(tile(0));

    // Start the timer.

    tock = SDL_GetTicks();
}

app::host::~host()
{
    std::vector<eye *>::iterator i;

    for (i = eyes.begin(); i != eyes.end(); ++i)
        delete (*i);
    
    if (node)
    {
        fini_client();
        fini_server();
        fini_listen();
    }
}

//-----------------------------------------------------------------------------

void app::host::root_loop()
{
    std::vector<tile>::iterator i;

    double p[3];
    double v[3];

    while (1)
    {
        SDL_Event e;

        // While there are available events, dispatch event handlers.

        while (SDL_PollEvent(&e))
            switch (e.type)
            {
            case SDL_MOUSEMOTION:
                for (i = tiles.begin(); i != tiles.end(); ++i)
                    if (i->pick(p, v, e.motion.x, e.motion.y))
                        point(p, v);
                break;

            case SDL_MOUSEBUTTONDOWN:
                click(e.button.button, true);
                break;

            case SDL_MOUSEBUTTONUP:
                click(e.button.button, false);
                break;

            case SDL_KEYDOWN:
                keybd(e.key.keysym.unicode,
                      e.key.keysym.sym, SDL_GetModState(), true);
                break;

            case SDL_KEYUP:
                keybd(e.key.keysym.unicode,
                      e.key.keysym.sym, SDL_GetModState(), false);
                break;

            case SDL_QUIT:
                save();
                close();
                return;
            }

        // Handle tracker events.

        if (tracker_status())
        {
            double R[3][3];
            double p[3];
            double a[2];
            bool   b;

            if (tracker_button(0, b)) click(0, b);
            if (tracker_button(1, b)) click(1, b);
            if (tracker_button(2, b)) click(2, b);

            if (tracker_sensor(0, p, R)) track(0, p, R[0], R[2]);
            if (tracker_sensor(1, p, R)) track(1, p, R[0], R[2]);

            if (tracker_values(0, a)) stick(0, a);
        }

        // Call the timer handler for each jiffy that has passed.

        while (SDL_GetTicks() - tock >= JIFFY)
        {
            tock += JIFFY;
            timer(JIFFY);
        }

        // Call the paint handler.

        paint();
        fleep();
    }
}

void app::host::node_loop()
{
    int    a;
    int    b;
    int    c;
    bool   d;
    double p[3];
    double v[3];
    double x[3];
    double z[3];

    while (1)
    {
        message M(0);

        M.recv(server_sd);

        switch (M.type())
        {
        case E_TRACK:
            a    = M.get_int();
            p[0] = M.get_double();
            p[1] = M.get_double();
            p[2] = M.get_double();
            x[0] = M.get_double();
            x[1] = M.get_double();
            x[2] = M.get_double();
            z[0] = M.get_double();
            z[1] = M.get_double();
            z[2] = M.get_double();
            track(a, p, x, z);
            break;

        case E_STICK:
            a    = M.get_int();
            p[0] = M.get_double();
            p[1] = M.get_double();
            stick(a, p);
            break;

        case E_POINT:
            p[0] = M.get_double();
            p[1] = M.get_double();
            p[2] = M.get_double();
            v[0] = M.get_double();
            v[1] = M.get_double();
            v[2] = M.get_double();
            point(p, v);
            break;

        case E_CLICK:
            a = M.get_int ();
            d = M.get_bool();
            click(a, d);
            break;

        case E_KEYBD:
            a = M.get_int ();
            b = M.get_int ();
            c = M.get_int ();
            d = M.get_bool();
            keybd(a, b, c, d);
            break;

        case E_TIMER:
            a = M.get_int();
            timer(a);
            break;

        case E_PAINT:
            paint();
            break;

        case E_FLEEP:
            fleep();
            break;

        case E_CLOSE:
            close();
            return;
        }
    }
}

void app::host::loop()
{
    // Handle all events.

    if (root())
        root_loop();
    else
        node_loop();
}

//-----------------------------------------------------------------------------

bool app::host::get_plane(double *F, const double *A,
                                     const double *B,
                                     const double *C,
                                     const double *V, int n) const
{
    const double small = 1e-3;

    // Create a plane from the given points.

    make_plane(F, A, B, C);

    // Reject any plane that closely matches one in the given list.

    for (int k = 0; k < n; ++k)
    {
        const double *G = V + k * 4;

        if (DOT3(F, G) >= 1 - small && fabs(F[3] - G[3]) < small)
            return false;
    }

    // Reject any plane with a tile position behind it.

    std::vector<tile>::const_iterator i;

    for (i = tiles.begin(); i != tiles.end(); ++i)
    {
        const double *BL = i->get_BL();
        const double *BR = i->get_BR();
        const double *TL = i->get_TL();
        const double *TR = i->get_TR();

        if (DOT3(F, BL) + F[3] < -small) return false;
        if (DOT3(F, BR) + F[3] < -small) return false;
        if (DOT3(F, TL) + F[3] < -small) return false;
        if (DOT3(F, TR) + F[3] < -small) return false;
    }

    // Reject any plane with an eye position behind it.

    std::vector<eye *>::const_iterator j;

    for (j = eyes.begin(); j != eyes.end(); ++j)
    {
        const double *P = (*j)->get_P();

        if (DOT3(F, P) + F[3] < -small) return false;
    }

    // This forms part of the convex hull of the host frustum.

    return true;
}

int app::host::get_frustum(double *F) const
{
    int n = 0;

    // TODO: near plane

    std::vector<tile>::const_iterator i;

    // Iterate over all [tile, eye] pairs.

    for (i = tiles.begin(); i != tiles.end(); ++i)
    {
        const double *BL = i->get_BL();
        const double *BR = i->get_BR();
        const double *TL = i->get_TL();
        const double *TR = i->get_TR();

        std::vector<eye *>::const_iterator j;

        for (j = eyes.begin(); j != eyes.end(); ++j)
        {
            const double *P = (*j)->get_P();

            // Store the planes forming the convex hull of the frustum.

            if (get_plane(F + 4 * n, P, BL, TL, F, n)) n++;
            if (get_plane(F + 4 * n, P, TR, BR, F, n)) n++;
            if (get_plane(F + 4 * n, P, BR, BL, F, n)) n++;
            if (get_plane(F + 4 * n, P, TL, TR, F, n)) n++;
        }
    }

    return n;
}

#ifdef SNIP

void app::host::get_plane(double *R, const double *A,
                                     const double *B,
                                     const double *C) const
{
    const double small = 1e-3;

    // Create a plane from the given points.

    double P[4];

    make_plane(P, A, B, C);

    // Reject the plane if any screen corner falls behind it.

    std::vector<tile>::const_iterator i;

    for (i = tiles.begin(); i != tiles.end(); ++i)
    {
        if (DOT3(i->get_BL(), P) + P[3] < -small) return;
        if (DOT3(i->get_BR(), P) + P[3] < -small) return;
        if (DOT3(i->get_TL(), P) + P[3] < -small) return;
        if (DOT3(i->get_TR(), P) + P[3] < -small) return;
    }

    // Reject the plane if any eye falls in front of it.

    std::vector<eye*>::const_iterator j;

    for (j = eyes.begin(); j != eyes.end(); ++j)
    {
        if (DOT3((*j)->get_P(), P) + P[3] > small) return;
    }

    // This plane forms part of the frustum union.  Note it.

    R[0] = P[0];
    R[1] = P[1];
    R[2] = P[2];
    R[3] = P[3];
}

void app::host::get_frustum(double *F) const
{
    // Construct a 4-plane frustum union of all eyes and screen corners.

    std::vector<tile>::const_iterator i;
    std::vector<eye*>::const_iterator j;

    for (i = tiles.begin(); i != tiles.end(); ++i)
    {
        const double *BL = i->get_BL();
        const double *BR = i->get_BR();
        const double *TL = i->get_TL();
        const double *TR = i->get_TR();

        for (j = eyes.begin(); j != eyes.end(); ++j)
        {
            const double *P = (*j)->get_P();

            // Find four planes with all eyes behind and all corners in front.

            get_plane(F +  0, BL, TL, P);
            get_plane(F +  4, TR, BR, P);
            get_plane(F +  8, BR, BL, P);
            get_plane(F + 12, TL, TR, P);
        }
    }

    // Push the four planes back to ensure all eyes are in front.

    for (j = eyes.begin(); j != eyes.end(); ++j)
    {
        const double *P = (*j)->get_P();

        for (int k = 0; k < 4; ++k)
        {
            double *Q = F + 4 * k;

            if (Q[3] < -DOT3(P, Q))
                Q[3] = -DOT3(P, Q);
        }
    }
/*
    printf("%d\n", 4);

    for (int k = 0; k < 4; ++k)
        printf("%+8.3f %+8.3f %+8.3f %f\n",
               F[4 * k + 0],
               F[4 * k + 1],
               F[4 * k + 2],
               F[4 * k + 3]);
*/
}

#endif

//-----------------------------------------------------------------------------

void app::host::draw()
{
    // Determine the frustum union and preprocess the app.

    double F[32];

    get_frustum(F);
    ::prog->prep(F, 4);

    // Render all tiles.
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    std::vector<tile>::iterator i;
    int index;

    for (index = 0, i = tiles.begin(); i != tiles.end(); ++i, ++index)
        i->draw(eyes, (index == varrier_index));

    // If doing network sync, wait until the rendering has finished.

    if (server_sd != INVALID_SOCKET || !client_sd.empty())
        glFinish();
}

int app::host::get_window_m() const
{
    return (server_sd == INVALID_SOCKET) ? 0 : SDL_NOFRAME;
}

//-----------------------------------------------------------------------------

void app::host::send(message& M)
{
    // Send a message to all clients.

    for (SOCKET_i i = client_sd.begin(); i != client_sd.end(); ++i)
        M.send(*i);
}

void app::host::recv(message& M)
{
    // Receive a message from all clients (should be E_REPLY).

    for (SOCKET_i i = client_sd.begin(); i != client_sd.end(); ++i)
        M.recv(*i);
}

//-----------------------------------------------------------------------------

void app::host::stick(int d, const double *p)
{
    if (!client_sd.empty())
    {
        message M(E_STICK);

        M.put_int   (d);
        M.put_double(p[0]);
        M.put_double(p[1]);

        send(M);
    }
    ::prog->stick(d, p);
}

void app::host::point(const double *p, const double *v)
{
    if (!client_sd.empty())
    {
        message M(E_POINT);

        M.put_double(p[0]);
        M.put_double(p[1]);
        M.put_double(p[2]);
        M.put_double(v[0]);
        M.put_double(v[1]);
        M.put_double(v[2]);

        send(M);
    }
    ::prog->point(p, v);
}

void app::host::click(int b, bool d)
{
    if (!client_sd.empty())
    {
        message M(E_CLICK);

        M.put_int (b);
        M.put_bool(d);

        send(M);
    }
    ::prog->click(b, d);
}

void app::host::keybd(int c, int k, int m, bool d)
{
    mods = m;

    if (!client_sd.empty())
    {
        message M(E_KEYBD);

        M.put_int (c);
        M.put_int (k);
        M.put_int (m);
        M.put_bool(d);

        send(M);
    }
    ::prog->keybd(k, d, c);
}

void app::host::timer(int d)
{
    if (!client_sd.empty())
    {
        message M(E_TIMER);

        M.put_int(d);

        send(M);
    }
    ::prog->timer(d / 1000.0);

    poll_listen();
}

void app::host::track(int d, const double *p, const double *x, const double *z)
{
    if (!client_sd.empty())
    {
        message M(E_TRACK);

        M.put_int   (d);
        M.put_double(p[0]);
        M.put_double(p[1]);
        M.put_double(p[2]);
        M.put_double(x[0]);
        M.put_double(x[1]);
        M.put_double(x[2]);
        M.put_double(z[0]);
        M.put_double(z[1]);
        M.put_double(z[2]);

        send(M);
    }
    ::prog->track(d, p, x, z);
}

void app::host::paint()
{
    if (!client_sd.empty())
    {
        message M(E_PAINT);

        send(M);
        draw( );
        recv(M);
    }
    else draw();

    if (server_sd != INVALID_SOCKET)
    {
        message R(E_REPLY);
        R.send(server_sd);
    }
}

void app::host::fleep()
{
    if (!client_sd.empty())
    {
        message M(E_FLEEP);

        send(M);
    }

    SDL_GL_SwapBuffers();
    ::perf->step();
}

void app::host::close()
{
    if (!client_sd.empty())
    {
        message M(E_CLOSE);

        send(M);
        recv(M);
    }
    if (server_sd != INVALID_SOCKET)
    {
        message R(E_REPLY);
        R.send(server_sd);
    }
}

//-----------------------------------------------------------------------------

void app::host::set_head(const double *p,
                         const double *x,
                         const double *y,
                         const double *z)
{
    std::vector<eye *>::iterator i;

    for (i = eyes.begin(); i != eyes.end(); ++i)
        (*i)->set_head(p, x, y, z);
}

//-----------------------------------------------------------------------------

void app::host::gui_pick(int& x, int& y, const double *p,
                                         const double *v) const
{
    // Compute the point (x, y) at which the vector V from P hits the GUI.

    double q[3];
    double w[3];

    mult_mat_vec3(q, gui_I, p);
    mult_xps_vec3(w, gui_I, v);

    normalize(w);

    x = int(nearestint(q[0] - q[2] * w[0] / w[2]));
    y = int(nearestint(q[1] - q[2] * w[1] / w[2]));
}

void app::host::gui_size(int& w, int& h) const
{
    // Return the configured resolution of the GUI.

    w = gui_w;
    h = gui_h;
}

void app::host::gui_view() const
{
    // Apply the transform taking GUI coordinates to eye coordinates.

    glMultMatrixd(gui_M);
}

void app::host::tag_draw()
{
    ::view->push();
    {
        if (logo_text == 0)
            logo_text = ::glob->load_texture(logo_name);

        glMatrixMode(GL_TEXTURE);
        {
            glLoadIdentity();
        }
        glMatrixMode(GL_PROJECTION);
        {
            glLoadIdentity();
            ::view->mult_P();
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glLoadMatrixd(tag_M);
        }

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glDisable(GL_LIGHTING);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        logo_text->bind();
        {
            glColor3f(1.0f, 1.0f, 1.0f);
            glBegin(GL_QUADS);
            {
                glTexCoord2i(0, 0); glVertex2i(0, 0);
                glTexCoord2i(1, 0); glVertex2i(1, 0);
                glTexCoord2i(1, 1); glVertex2i(1, 1);
                glTexCoord2i(0, 1); glVertex2i(0, 1);
            }
            glEnd();
        }
        logo_text->free();
    }
    ::view->pop();
}

//-----------------------------------------------------------------------------

void app::host::set_varrier_index(int d)
{
    varrier_index += d;
}

void app::host::set_varrier_pitch(double d)
{
    if (0 <= varrier_index && varrier_index < int(tiles.size()))
    {
        tiles[varrier_index].set_varrier_pitch(d);
        dirty = true;
    }
}

void app::host::set_varrier_angle(double d)
{
    if (0 <= varrier_index && varrier_index < int(tiles.size()))
    {
        tiles[varrier_index].set_varrier_angle(d);
        dirty = true;
    }
}

void app::host::set_varrier_shift(double d)
{
    if (0 <= varrier_index && varrier_index < int(tiles.size()))
    {
        tiles[varrier_index].set_varrier_shift(d);
        dirty = true;
    }
}

void app::host::set_varrier_thick(double d)
{
    if (0 <= varrier_index && varrier_index < int(tiles.size()))
    {
        tiles[varrier_index].set_varrier_thick(d);
        dirty = true;
    }
}

//-----------------------------------------------------------------------------
