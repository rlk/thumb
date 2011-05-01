//  Copyright (C) 2009-2011 Robert Kooima
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

#ifndef OGL_TERRAIN_HPP
#define OGL_TERRAIN_HPP

#include <string>

#include <ogl-opengl.hpp>
#include <ogl-range.hpp>
#include <ogl-aabb.hpp>

//-----------------------------------------------------------------------------

namespace ogl
{
    class binding;
    class uniform;
}

//-----------------------------------------------------------------------------

namespace ogl
{
    struct head_s;
    struct page_s;
    struct vert_s;

    class terrain
    {
        std::string name;
        std::string data;

        const uint8_t *buff;

        const head_s *head;
        const page_s *page;
        const vert_s *vert;

        const ogl::binding *land_bind;
        const ogl::binding *gras_bind;

        ogl::uniform *uniform_terrain_size;

        std::vector<ogl::aabb> bound;

        GLuint  vbo;
        GLuint  p_ibo[16];
        GLsizei p_ibc[16];
        GLuint  g_ibo[8][4];
        GLsizei g_ibc[8];

        double s;
        double h;
        double M[16];
        double I[16];

        GLsizei rand_index(GLuint *);
        GLsizei calc_index(GLuint, bool, bool, bool, bool);
        void    calc_bound(int16_t, int16_t,
                           int16_t, int16_t, uint32_t);

        bool  page_test(int, const double *) const;
        void  page_sub0(int, int, int, int, int, 
                        const double *, const double *, int) const;
        void  page_sub1(int, int, int, int, int, 
                        const double *, const double *, int) const;
        void  page_sub2(int, int, int, int, int, 
                        const double *, const double *, int) const;
        void  page_sub3(int, int, int, int, int, 
                        const double *, const double *, int) const;
        void  page_subn(int, int, int, int, int, 
                        const double *, const double *, int) const;
        void  page_subs(int, int, int, int, int, 
                        const double *, const double *, int) const;
        void  page_draw(int, int, int, int, int, 
                        const double *, const double *, int) const;

        bool  gras_test(int, const double *) const;
        void  gras_sub0(int, int, int, int, int, 
                        const double *, const double *, int) const;
        void  gras_sub1(int, int, int, int, int, 
                        const double *, const double *, int) const;
        void  gras_sub2(int, int, int, int, int, 
                        const double *, const double *, int) const;
        void  gras_sub3(int, int, int, int, int, 
                        const double *, const double *, int) const;
        void  gras_subn(int, int, int, int, int, 
                        const double *, const double *, int) const;
        void  gras_subs(int, int, int, int, int, 
                        const double *, const double *, int) const;
        void  gras_draw(int, int, int, int, int, 
                        const double *, const double *, int) const;

    public:

        const std::string& get_name() const { return name; }

        terrain(std::string);
       ~terrain();

        ogl::range view(                const double *, int) const;
        void       draw(const double *, const double *, int) const;

        void init();
        void fini();
    };
}

//-----------------------------------------------------------------------------

#endif
