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
#include <vector>
#include <list>
#include <map>
#include <set>

#include <string.h>
#include <errno.h>

#include "util.hpp"
#include "app-serial.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    //-------------------------------------------------------------------------
    // Data exceptions.

    class find_error : public std::runtime_error
    {
        std::string mesg(const std::string& s) {
            return "Error finding file: " + s + ": " + strerror(errno);
        }

    public:
        find_error(const std::string& s) : std::runtime_error(mesg(s)) { }
    };

    class open_error : public std::runtime_error
    {
        std::string mesg(const std::string& s) {
            return "Error opening file: " + s + ": " + strerror(errno);
        }

    public:
        open_error(const std::string& s) : std::runtime_error(mesg(s)) { }
    };

    class stat_error : public std::runtime_error
    {
        std::string mesg(const std::string& s) {
            return "Error stating file: " + s + ": " + strerror(errno);
        }

    public:
        stat_error(const std::string& s) : std::runtime_error(mesg(s)) { }
    };

    class read_error : public std::runtime_error
    {
        std::string mesg(const std::string& s) {
            return "Error reading file: " + s + ": " + strerror(errno);
        }

    public:
        read_error(const std::string& s) : std::runtime_error(mesg(s)) { }
    };

    class write_error : public std::runtime_error
    {
        std::string mesg(const std::string& s) {
            return "Error writing file: " + s + ": " + strerror(errno);
        }

    public:
        write_error(const std::string& s) : std::runtime_error(mesg(s)) { }
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

    typedef buffer                         *buffer_p;
    typedef std::map<std::string, buffer_p> buffer_m;

    //-------------------------------------------------------------------------
    // Data archive interface

    class archive
    {
    public:

        virtual bool     find(std::string)                         const = 0;
        virtual buffer_p load(std::string)                         const = 0;
        virtual bool     save(std::string, const void *, size_t *) const = 0;
        virtual void     list(std::string, str_set&, str_set&)      const = 0;

        virtual ~archive() { }
    };

    typedef archive                             *archive_p;
    typedef std::list<archive_p>                 archive_l;
    typedef std::list<archive_p>::iterator       archive_i;
    typedef std::list<archive_p>::const_iterator archive_c;

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
        bool        writable;

        virtual bool     find(std::string)                         const;
        virtual buffer_p load(std::string)                         const;
        virtual bool     save(std::string, const void *, size_t *) const;
        virtual void     list(std::string, str_set&, str_set&)     const;

    public:

        file_archive(std::string path, bool writable=false) :
            path(path), writable(writable) { }
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
        std::string filename;
        app::file   file;

        std::string translate(const std::string&) const;

        archive_l archives;
        buffer_m  buffers;

    public:

        data(const std::string&);
       ~data();

        void init();

        const void *load(const std::string&,               size_t * = 0);
        bool        save(const std::string&, const void *, size_t * = 0);
        void        free(const std::string&);
        void        list(const std::string&, str_set&, str_set&) const;
    };
}

//-----------------------------------------------------------------------------

extern app::data *data;

//-----------------------------------------------------------------------------

#endif
