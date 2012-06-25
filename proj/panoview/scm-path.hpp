//  Copyright (C) 2005-2012 Robert Kooima
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

#ifndef SCM_PATH_HPP
#define SCM_PATH_HPP

#include <vector>
#include <string>

#include "scm-step.hpp"

//------------------------------------------------------------------------------

class scm_path
{
public:

    scm_path();

    void clear();

    void fore(bool);
    void back(bool);
    void next();
    void prev();
    void home();
    void jump();

    void get(scm_step&);
    void set(scm_step&);
    void ins(scm_step&);
    void del();

    void save();
    void load();

    void time(double);
    void draw();

    bool playing() const { return (head_d != 0.0); }

private:

    double head_t;
    double head_d;
    int    curr;

    std::string filename;

    std::vector<scm_step> step;
};

//------------------------------------------------------------------------------

#endif
