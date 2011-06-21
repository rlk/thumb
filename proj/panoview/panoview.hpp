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

#include "sph-model.hpp"

//-----------------------------------------------------------------------------

class panoview : public app::prog
{
public:

    panoview(const std::string&);
   ~panoview();

    ogl::range prep(int, const app::frustum * const *);
    void       lite(int, const app::frustum * const *);
    void       draw(int, const app::frustum *);
    
private:

    int n;
    int m;

    sph_model L;
};

//-----------------------------------------------------------------------------

#endif
