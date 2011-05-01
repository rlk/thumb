//  Copyright (C) 2009-2011 Robert Kooima
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

#ifndef OGL_SH_BASIS_HPP
#define OGL_SH_BASIS_HPP

#include <ogl-cubelut.hpp>

//-----------------------------------------------------------------------------

namespace ogl
{
    class sh_basis : public cubelut
    {
        int l;
        int m;

    protected:

        void fill(float *, const double *,
                           const double *,
                           const double *,
                           const double *) const;
    public:

        sh_basis(const std::string&, int);
       ~sh_basis();
    };
}

//-----------------------------------------------------------------------------

#endif
