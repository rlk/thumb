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

#ifndef UNI_OVERLAY_HPP
#define UNI_OVERLAY_HPP

#include <map>

#include "ogl-pool.hpp"

//-----------------------------------------------------------------------------

namespace uni
{
    class overlay
    {
        double radius;

        double M[16];
        double I[16];

        void  world_to_sphere(double&, double&, double&,
                              double,  double,  double);
        bool screen_to_sphere(double&, double&, double&,
                              double,  double);

        // Overlay control messages

        char buffer[256];

        void m_moveto        (const char *);
        void m_lookup        (const char *); //
        void m_get_position  (const char *); //

        void m_model_create  (const char *); //
        void m_model_delete  (const char *); //
        void m_model_position(const char *); //
        void m_model_rotation(const char *); //
        void m_model_scale   (const char *); //

        void m_image_create  (const char *);
        void m_image_delete  (const char *);
        void m_image_position(const char *);
        void m_image_rotation(const char *);
        void m_image_scale   (const char *);

        void m_capture_color (const char *);
        void m_capture_radius(const char *);

        void m_data_hide     (const char *);
        void m_data_show     (const char *);

        // Model overlay

        class model
        {
            ogl::unit *unit;

            double lat;
            double lon;
            double rad;
            double rot;
            double scl;

            void apply();

        public:

            model(const char *);
           ~model();

            void position(double, double, double);
            void rotation(double);
            void scale   (double);

            ogl::unit *get_unit() { return unit; }
        };

        typedef model *model_p;

        std::map<int, model_p> models;

        ogl::pool *pool;
        ogl::node *node;

    public:

        overlay(double);
       ~overlay();

        const char *script(const char *);

        void transform(const double *,
                       const double *);

        void prep();

        void draw_images();
        void draw_models();
    };
}

//-----------------------------------------------------------------------------

#endif
