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

#include <SDL.h>
#include <png.h>

#include <stdexcept>

#include "opengl.hpp"
#include "prog.hpp"
#include "app-conf.hpp"
#include "host.hpp"

//-----------------------------------------------------------------------------

void app::prog::snap(std::string filename, int w, int h) const
{
    FILE       *filep  = NULL;
    png_structp writep = NULL;
    png_infop   infop  = NULL;
    png_bytep  *bytep  = NULL;

    // Initialize all PNG export data structures.

    if (!(filep = fopen(filename.c_str(), "wb")))
        throw std::runtime_error("Failure opening PNG file for writing");

    if (!(writep = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        throw std::runtime_error("Failure creating PNG write structure");

    if (!(infop = png_create_info_struct(writep)))
        throw std::runtime_error("Failure creating PNG info structure");

    // Enable the default PNG error handler.

    if (setjmp(png_jmpbuf(writep)) == 0)
    {
        unsigned char *p = NULL;

        // Initialize the PNG header.

        png_init_io (writep, filep);
        png_set_compression_level(writep, 9);
        png_set_IHDR(writep, infop, w, h, 8, PNG_COLOR_TYPE_RGB,
                                             PNG_INTERLACE_NONE,
                                             PNG_COMPRESSION_TYPE_DEFAULT,
                                             PNG_FILTER_TYPE_DEFAULT);

        // Allocate the pixel buffer and copy pixels there.

        if ((p = new unsigned char[w * h * 3]))
        {
            glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, p);

            // Allocate and initialize the row pointers.

            if ((bytep = (png_bytep *)png_malloc(writep, h*sizeof(png_bytep))))
            {
                for (int i = 0; i < h; ++i)
                    bytep[h - i - 1] = (png_bytep) (p + i * w * 3);

                // Write the PNG image file.

                png_set_rows  (writep, infop, bytep);
                png_write_info(writep, infop);
                png_write_png (writep, infop, 0, NULL);

                free(bytep);
            }
            else throw std::runtime_error("Failure allocating PNG row array");

            delete [] p;
        }
    }

    // Release all resources.

    png_destroy_write_struct(&writep, &infop);
    fclose(filep);
}

//-----------------------------------------------------------------------------

app::prog::prog() : options(0)
{
    key_snap = ::conf->get_i("key_snap");
    key_exit = ::conf->get_i("key_exit");
    key_init = ::conf->get_i("key_init");
}

void app::prog::keybd(int c, int k, int m, bool d)
{
    SDL_Event user = { SDL_USEREVENT };
    SDL_Event quit = { SDL_QUIT      };

    if (d)
    {
        if (k == key_snap)
            snap(::conf->get_s("screenshot_file"),
                 ::host->get_window_w(),
                 ::host->get_window_h());

        else if (k == key_exit) SDL_PushEvent(&quit);
        else if (k == key_init) SDL_PushEvent(&user);

        else if (m & KMOD_SHIFT)
        {
            if (SDLK_F1 <= k && k <= SDLK_F12)
                options ^= (1 << (k - SDLK_F1));
        }
    }
}

//-----------------------------------------------------------------------------

bool app::prog::option(int b) const
{
    return (options & (1 << b)) ? true : false;
}

//-----------------------------------------------------------------------------
