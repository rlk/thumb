//  Copyright (C) 2007-2011 Robert Kooima
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

#ifndef DEV_MOUSE
#define DEV_MOUSE

#include <sstream>

#include <SDL_keyboard.h>

#include <dev-input.hpp>

//-----------------------------------------------------------------------------

namespace dev
{

    class mouse : public input
    {
    public:

        mouse();

        virtual bool process_event(app::event *);

        virtual ~mouse();


    private:

        // A set of keys and their current states

        class keyset {
        public:

            // Initialize a new keyset given a string of key codes.

            keyset(const std::string& s, int d)
            {
                std::stringstream str(s);
                std::string       val;

                while (std::getline(str, val, ','))
                    if (int k = atoi(val.c_str()))
                        v.push_back(key(k, false));

                if (v.empty()) v.push_back(key(d, false));
            }

            // Return 1 if any key in the keyset is currently pressed.

            int check() const
            {
                std::vector<key>::const_iterator i;

                for (i = v.begin(); i != v.end(); ++i)
                    if (i->state)
                        return 1;

                return 0;
            }

            // Update the keyset given a key press or release event.

            bool event(int code, bool state)
            {
                std::vector<key>::iterator i;

                for (i = v.begin(); i != v.end(); ++i)
                    if (i->code == code)
                    {
                        i->state = state;
                        return true;
                    }

                return false;
            }

        private:

            struct key
            {
                int  code;
                bool state;
                key(int c, bool s) : code(c), state(s) { }
            };

            std::vector<key> v;
        };

        // Configuration

        keyset move_L;
        keyset move_R;
        keyset move_D;
        keyset move_U;
        keyset move_F;
        keyset move_B;

        keyset turn_L;
        keyset turn_R;
        keyset turn_D;
        keyset turn_U;

        double filter;
        double speed;
        int    mode;

        // Navigation state

        bool dragging;
        bool modified;

        quat last_q;
        quat curr_q;

        // Filtered differentials.

        vec3      dpos;
        double    dyaw;
        double    dpitch;

        // Event handlers

        bool process_point(app::event *);
        bool process_click(app::event *);
        bool process_tick (app::event *);
        bool process_key  (app::event *);

        // Keyboard handling

        void parse_keyset(      keyset&, const std::string&, int);
        int  check_keyset(const keyset&) const;
    };
}

//-----------------------------------------------------------------------------

#endif
