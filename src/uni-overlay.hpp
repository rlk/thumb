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

//-----------------------------------------------------------------------------

namespace uni
{
    class overlay
    {
        // Overlay control messages

        void m_moveto(const char *, char *);
        void m_lookup(const char *, char *);
        void m_get_position(const char *, char *);

        void m_model_create  (const char *, char *);
        void m_model_delete  (const char *, char *);
        void m_model_position(const char *, char *);
        void m_model_rotation(const char *, char *);
        void m_model_scale   (const char *, char *);

        void m_image_create  (const char *, char *);
        void m_image_delete  (const char *, char *);
        void m_image_position(const char *, char *);
        void m_image_rotation(const char *, char *);
        void m_image_scale   (const char *, char *);

        void m_capture_color (const char *, char *);
        void m_capture_radius(const char *, char *);

        void m_data_hide(const char *, char *);
        void m_data_show(const char *, char *);

    public:

        overlay();
       ~overlay();

        void script(const char *, char *);

        void step();

        void draw_images();
        void draw_models();
    };
}

//-----------------------------------------------------------------------------

#endif
