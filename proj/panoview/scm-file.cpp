//  Copyright (C) 2005-2011 Robert Kooima
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

#include <cstdlib>
#include <cstring>
#include <sstream>

#include "scm-index.hpp"
#include "scm-file.hpp"

//------------------------------------------------------------------------------

#include <sys/types.h>
#include <sys/stat.h>

static bool exists(const std::string& name)
{
    struct stat info;

    if (stat(name.c_str(), &info) == 0)
        return ((info.st_mode & S_IFMT) == S_IFREG);
    else
        return false;
}

static int catcmp(const void *p, const void *q)
{
    const uint64 *a = (const uint64 *) p;
    const uint64 *b = (const uint64 *) q;

    if      (a[0] < b[0]) return -1;
    else if (a[0] > b[0]) return +1;
    else                  return  0;
}

//------------------------------------------------------------------------------

// Construct a file table entry. Open the TIFF briefly to determine its format.

scm_file::scm_file(const std::string& tiff, float n0, float n1, int dd)
    : n0(n0), n1(n1), dd(dd), catc(0), catv(0), minv(0), maxv(0)
{
    // If the given file name is absolute, use it.

    if (exists(tiff))
        name = tiff;

    // Otherwise, search the SCM path for the file.

    else if (char *val = getenv("SCMPATH"))
    {
        std::stringstream list(val);
        std::string       path;
        std::string       temp;

        while (std::getline(list, path, ':'))
        {
            temp = path + "/" + tiff;

            if (exists(temp))
            {
                name = temp;
                break;
            }
        }
    }

    if (!name.empty())
    {
        if (TIFF *T = TIFFOpen(name.c_str(), "r"))
        {
            uint64 n = 0;
            void  *p = 0;

            catc = 0;

            TIFFGetField(T, TIFFTAG_IMAGEWIDTH,      &w);
            TIFFGetField(T, TIFFTAG_IMAGELENGTH,     &h);
            TIFFGetField(T, TIFFTAG_BITSPERSAMPLE,   &b);
            TIFFGetField(T, TIFFTAG_SAMPLESPERPIXEL, &c);
            TIFFGetField(T, TIFFTAG_SAMPLEFORMAT,    &g);

            if (TIFFGetField(T, 0xFFB1, &n, &p))
            {
                if ((catv = (uint64 *) malloc(n * sizeof (uint64))))
                    memcpy(catv, p, n * sizeof (uint64));

                catc = n / 2;
            }
            if (TIFFGetField(T, 0xFFB2, &n, &p))
            {
                if ((minv =  malloc(catc * c * b / 8)))
                    memcpy(minv, p, catc * c * b / 8);
            }
            if (TIFFGetField(T, 0xFFB3, &n, &p))
            {
                if ((maxv =  malloc(catc * c * b / 8)))
                    memcpy(maxv, p, catc * c * b / 8);
            }

            TIFFClose(T);
        }
    }
}

scm_file::~scm_file()
{
    free(maxv);
    free(minv);
    free(catv);
}

//------------------------------------------------------------------------------

// Determine whether page i is given by this file. Push the page reference up
// the tree as dictated by the overdraw parameter. This allows a shallow but
// high-res vertex data set to be applied to a deep but low-res mesh.

bool scm_file::status(uint64 i) const
{
    void *p;

    long long j = i;

    for (int o = 0; o < dd; ++o)
        if (j > 5) j = scm_page_parent(j);

    if (catc && (p = bsearch(&j, catv, catc, 2 * sizeof (uint64), catcmp)))
        return true;
    else
        return false;
}

// Seek page i in the page catalog and return its file offset.

uint64 scm_file::offset(uint64 i) const
{
    void *p;

    if (catc && (p = bsearch(&i, catv, catc, 2 * sizeof (uint64), catcmp)))
        return ((uint64 *) p)[1];
    else
        return 0;
}

// Determine the min and max values of page i. Seek it in the page catalog and
// reference the corresponding page in the min and max caches. If page i is not
// represented, assume its parent provides a useful bound and iterate up.

void scm_file::bounds(uint64 i, float& r0, float& r1) const
{
    void *p;

    r0 = 1.0;
    r1 = 1.0;

    if (catc && catv && minv && maxv)
        while (i >= 0)
            if ((p = bsearch(&i, catv, catc, 2 * sizeof (uint64), catcmp)))
            {
                uint64 j = ((uint64 *) p - catv) / 2;

                if (b == 8)
                {
                    if (g == 2)
                    {
                        r0 = ((char *) minv)[j * c] / 127.f;
                        r1 = ((char *) maxv)[j * c] / 127.f;
                    }
                    else
                    {
                        r0 = ((unsigned char *) minv)[j * c] / 255.f;
                        r1 = ((unsigned char *) maxv)[j * c] / 255.f;
                    }
                }
                else if (b == 16)
                {
                    if (g == 2)
                    {
                        r0 = ((short *) minv)[j * c] / 32767.f;
                        r1 = ((short *) maxv)[j * c] / 32767.f;
                    }
                    else
                    {
                        r0 = ((unsigned short *) minv)[j * c] / 65535.f;
                        r1 = ((unsigned short *) maxv)[j * c] / 65535.f;
                    }
                }
                else if (b == 32)
                {
                    r0 = ((float *) minv)[j * c];
                    r1 = ((float *) minv)[j * c];
                }

                break;
            }
            else i = scm_page_parent(i);

    r0 = n0 + r0 * (n1 - n0);
    r1 = n0 + r1 * (n1 - n0);
}

// Return the buffer length for a page of this file.  24-bit is padded to 32.

size_t scm_file::length() const
{
    if (c == 3 && b == 8)
        return w * h * 4 * b / 8;
    else
        return w * h * c * b / 8;
}

//------------------------------------------------------------------------------
