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

#include "app-glob.hpp"
#include "uni-progset.hpp"

//-----------------------------------------------------------------------------

uni::progset::progset(std::string name) :
    plate(glob->load_program(name + "-plate.vert",
                             name + "-plate.frag")),
    polar(glob->load_program(name + "-polar.vert",
                             name + "-polar.frag")),
    strip(glob->load_program(name + "-strip.vert",
                             name + "-strip.frag"))
{
}

uni::progset::~progset()
{
    if (strip) glob->free_program(strip);
    if (polar) glob->free_program(polar);
    if (plate) glob->free_program(plate);
}

//-----------------------------------------------------------------------------

void uni::progset::bind(int type) const
{
    switch (type)
    {
    case prog_plate:
        plate->bind();
        break;

    case prog_north:
        polar->bind();
        polar->uniform("up", +1.0);
        break;

    case prog_south:
        polar->bind();
        polar->uniform("up", -1.0);
        break;

    case prog_strip:
        strip->bind();
        break;
    }
}

void uni::progset::free(int type) const
{
    switch (type)
    {
    case prog_plate: plate->free(); break;
    case prog_north: polar->free(); break;
    case prog_south: polar->free(); break;
    case prog_strip: strip->free(); break;
    }
}

//-----------------------------------------------------------------------------

void uni::progset::uniform(std::string name, int d) const
{
    if (plate)
    {
        plate->bind();
        plate->uniform(name, d);
        plate->free();
    }
    if (polar)
    {
        polar->bind();
        polar->uniform(name, d);
        polar->free();
    }
    if (strip)
    {
        strip->bind();
        strip->uniform(name, d);
        strip->free();
    }
}

void uni::progset::uniform(std::string name, double a) const
{
    if (plate)
    {
        plate->bind();
        plate->uniform(name, a);
        plate->free();
    }
    if (polar)
    {
        polar->bind();
        polar->uniform(name, a);
        polar->free();
    }
    if (strip)
    {
        strip->bind();
        strip->uniform(name, a);
        strip->free();
    }
}

void uni::progset::uniform(std::string name, double a, double b) const
{
    if (plate)
    {
        plate->bind();
        plate->uniform(name, a, b);
        plate->free();
    }
    if (polar)
    {
        polar->bind();
        polar->uniform(name, a, b);
        polar->free();
    }
    if (strip)
    {
        strip->bind();
        strip->uniform(name, a, b);
        strip->free();
    }
}

//-----------------------------------------------------------------------------
