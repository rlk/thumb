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

#ifndef UNI_GALAXY_HPP
#define UNI_GALAXY_HPP

#include "app-glob.hpp"
#include "app-frustum.hpp"

//-----------------------------------------------------------------------------

namespace uni
{
    //-------------------------------------------------------------------------

    class star
    {
        GLubyte col[4];
        GLfloat pos[3];
        GLfloat mag;

    public:

        const void *read(const void *);
    };

    typedef std::vector<star>           star_v;
    typedef std::vector<star>::iterator star_i;

    //-------------------------------------------------------------------------

    struct node
    {
        double bound[6];

        int    star0;
        int    starc;
        int    nodeL;
        int    nodeR;

    public:

        const void *read(const void *);

        void view(app::frustum_v&);
        void draw() const;
    };

    typedef std::vector<node>           node_v;
    typedef std::vector<node>::iterator node_i;

    //-------------------------------------------------------------------------

    class galaxy
    {
        double M[16];
        double I[16];

        GLuint  buffer;
        GLuint  texture;

        GLfloat magnitude;

        star_v S;
        node_v N;

        app::frustum_v frusta;

    public:

        galaxy(const char *);
       ~galaxy();

        void view(app::frustum_v&);
        void draw(int i) const;
    };

    //-------------------------------------------------------------------------
}

//-----------------------------------------------------------------------------

#endif
