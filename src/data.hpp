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

#ifndef DATA_HPP
#define DATA_HPP

#include <stdexcept>
#include <string>
#include <list>
#include <map>

#include <string.h>
#include <errno.h>

#include "dir.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    //-------------------------------------------------------------------------
    // Data exceptions.

    class open_error : public std::runtime_error
    {
        std::string mesg(std::string& s) {
            return "Error opening file: " + s + ": " + strerror(errno);
        }

    public:
        open_error(std::string& s) : std::runtime_error(mesg(s)) { }
    };

    class stat_error : public std::runtime_error
    {
        std::string mesg(std::string& s) {
            return "Error stating file: " + s + ": " + strerror(errno);
        }

    public:
        stat_error(std::string& s) : std::runtime_error(mesg(s)) { }
    };

    class read_error : public std::runtime_error
    {
        std::string mesg(std::string& s) {
            return "Error reading file: " + s + ": " + strerror(errno);
        }

    public:
        read_error(std::string& s) : std::runtime_error(mesg(s)) { }
    };
}

//-----------------------------------------------------------------------------

namespace app
{
    //-------------------------------------------------------------------------
    // Data buffer

    class buffer
    {
    protected:

        unsigned char *ptr;
        size_t         len;

    public:

        buffer();
       ~buffer();

        const void *get(size_t *) const;
    };

    typedef buffer *buffer_p;

    //-------------------------------------------------------------------------
    // Data archive interface

    class archive
    {
    public:

        virtual bool     find(std::string)                     = 0;
        virtual buffer_p load(std::string)                     = 0;
        virtual void     list(std::string, strings&, strings&) = 0;

        virtual ~archive() { }
    };

    typedef archive *archive_p;

    //-------------------------------------------------------------------------
    // File system data archive

    class file_buffer : public buffer
    {
    public:
        file_buffer(std::string);
    };

    class file_archive : public archive
    {
        std::string path;

        bool     find(std::string);
        buffer_p load(std::string);
        void     list(std::string, strings&, strings&);

    public:

        file_archive(std::string path) : path(path) { }
    };

    //-------------------------------------------------------------------------
    // Packaged data archive
/*
    class pack : public archive
    {
        virtual member_p load(std::string);

    public:

        pack();
    };
*/
    //-------------------------------------------------------------------------
    // Statically linked data archive
/*
    class link : public archive
    {
        virtual member_p load(std::string);

    public:

        link();
    };
*/
}

//-----------------------------------------------------------------------------

namespace app
{
    class data
    {
        std::list<archive_p>            archives;
        std::map<std::string, buffer_p> buffers;

    public:

        data();
       ~data();

        const void *load_dat(std::string name, size_t *size=0);
        void        free_dat(std::string name);
    };
}

/*
namespace app
{
    class data
    {
        struct img
        {
            void *p;
            int   w;
            int   h;
            int   b;
        };

        std::string path;

        std::map<std::string, int>         obj_map;
        std::map<std::string, std::string> txt_map;
        std::map<std::string, img>         img_map;

    public:

        data(std::string);
       ~data();

        const void *load_dat(const char *, size_t *);
        void        free_dat(const char *);

        int         get_obj(std::string);
        std::string get_txt(std::string);
        void       *get_img(std::string, int&, int&, int&);

        std::string get_absolute(std::string);
        std::string get_relative(std::string);
    };
}
*/
//-----------------------------------------------------------------------------

#endif
