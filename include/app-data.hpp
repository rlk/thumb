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

#ifndef APP_DATA_HPP
#define APP_DATA_HPP

#include <stdexcept>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>

#include <cstring>
#include <errno.h>

#include <app-file.hpp>

//-----------------------------------------------------------------------------

namespace app
{
    //-------------------------------------------------------------------------
    // Handy string containers.

    typedef std::vector<std::string> str_vec;
    typedef std::set   <std::string> str_set;

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

        archive(int p=0) : priority(p) { }

        virtual bool     find(std::string)                         const = 0;
        virtual buffer_p load(std::string)                         const = 0;
        virtual bool     save(std::string, const void *, size_t *) const = 0;
        virtual void     list(std::string, str_set&, str_set&)     const = 0;

        virtual ~archive() { }

        const int priority;
    };

    typedef archive *archive_p;

    struct archive_lt
    {
        bool operator()(const archive_p a, const archive_p b) const
        {
            return (a->priority < b->priority);
        }
    };

    typedef std::set<archive_p, archive_lt>                 archive_l;
    typedef std::set<archive_p, archive_lt>::iterator       archive_i;
    typedef std::set<archive_p, archive_lt>::const_iterator archive_c;

    //-------------------------------------------------------------------------
    // Data manager

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

        void add_file_archive(const std::string&, bool, int=0);
        void add_pack_archive(const void *, size_t, int=50);

        const void *load(const std::string&,               size_t * = 0);
        bool        save(const std::string&, const void *, size_t * = 0);
        bool        find(const std::string&);
        void        free(const std::string&);
        void        list(const std::string&, str_set&, str_set&) const;
    };
}

//-----------------------------------------------------------------------------

extern app::data *data;

//-----------------------------------------------------------------------------

#endif
