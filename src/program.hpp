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

#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <string>

#include "opengl.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class program
    {
        std::string vert_name;
        std::string frag_name;

        GLhandleARB vert;
        GLhandleARB frag;
        GLhandleARB prog;

        void log(GLhandleARB, std::string&);

    public:

        static const program *current;

        const std::string& get_vert_name() const { return vert_name; }
        const std::string& get_frag_name() const { return frag_name; }

        program(std::string, std::string);
       ~program();

        void bind() const;
        void free() const;

        void init();
        void fini();

        void uniform(std::string, int)                            const;
        void uniform(std::string, double)                         const;
        void uniform(std::string, double, double)                 const;
        void uniform(std::string, double, double, double)         const;
        void uniform(std::string, double, double, double, double) const;
        void uniform(std::string, const double *, bool=false)     const;
    };
}

#endif
