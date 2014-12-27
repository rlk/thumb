//  Copyright (C) 2005-2014 Robert Kooima
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

#ifndef APP_DATA_FILE_HPP
#define APP_DATA_FILE_HPP

#include <app-data.hpp>

//-----------------------------------------------------------------------------

namespace app
{
    // File system data buffer

    class file_buffer : public buffer
    {
    public:
        file_buffer(std::string);
    };

    // File system data archive

    class file_archive : public archive
    {
        std::string path;
        bool        writable;

    public:

        file_archive(std::string path, bool writable=false, int p=0);

        virtual bool     find(std::string)                         const;
        virtual buffer_p load(std::string)                         const;
        virtual bool     save(std::string, const void *, size_t *) const;
        virtual void     list(std::string, str_set&, str_set&)     const;
    };
}

//-----------------------------------------------------------------------------

#endif
