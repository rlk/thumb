//  Copyright (C) 2005 Robert Kooima
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

#ifndef DATA_HPP
#define DATA_HPP

#include <string>
#include <map>

//-----------------------------------------------------------------------------

namespace app
{
    class data
    {
        struct img
        {
            void *p;
            int   w;
            int   h;
            int   b;
        };

        std::string path;

        std::map<std::string, int>         obj_map;
        std::map<std::string, std::string> txt_map;
        std::map<std::string, img>         img_map;

    public:

        data(std::string);
       ~data();

        int         get_obj(std::string);
        std::string get_txt(std::string);
        void       *get_img(std::string, int&, int&, int&);

        std::string get_absolute(std::string);
        std::string get_relative(std::string);
    };
}

//-----------------------------------------------------------------------------

#endif
