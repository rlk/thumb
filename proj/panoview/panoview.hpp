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

#ifndef PANOVIEW_HPP
#define PANOVIEW_HPP

#include <app-prog.hpp>
#include "view-app.hpp"

//-----------------------------------------------------------------------------

class panoview : public view_app
{
public:

    panoview(const std::string&, const std::string&);

    virtual void draw(int, const app::frustum *, int);

    virtual ~panoview();

};

//-----------------------------------------------------------------------------

#endif
