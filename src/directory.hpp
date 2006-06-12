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

#ifndef DIR_HPP
#define DIR_HPP

#include <string>
#include <vector>

//-----------------------------------------------------------------------------

typedef std::vector<std::string> strvec;

class directory
{
    strvec path;

public:

    directory(std::string="/");

    std::string cwd();
    void        set(std::string);
    void        get(strvec&, strvec&, std::string&);
};

//-----------------------------------------------------------------------------

#endif
