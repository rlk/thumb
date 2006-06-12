#include <SDL.h>
#include <png.h>

#include <stdexcept>

#include "main.hpp"
#include "opengl.hpp"
#include "prog.hpp"

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

void app::prog::keybd(int k, bool d, int c)
{
    SDL_Event user = { SDL_USEREVENT };

    if (d)
    {
        switch (k)
        {
        case SDLK_F9:
            snap(::conf->get_s("screenshot_file"),
                 ::conf->get_i("window_w"),
                 ::conf->get_i("window_h"));
            break;

        case SDLK_F12:
            SDL_PushEvent(&user);
            break;
        }
    }
}

//-----------------------------------------------------------------------------
