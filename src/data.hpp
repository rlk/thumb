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
#include <mxml.h>

#include "util.hpp"

#define DEFAULT_DATA_FILE "data.xml"

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

    class write_error : public std::runtime_error
    {
        std::string mesg(std::string& s) {
            return "Error writing file: " + s + ": " + strerror(errno);
        }

    public:
        write_error(std::string& s) : std::runtime_error(mesg(s)) { }
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
        virtual void     list(std::string, strset&, strset&)       const = 0;

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
        virtual void     list(std::string, strset&, strset&)       const;

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
        std::string  file;
        mxml_node_t *head;
        mxml_node_t *root;

        std::string translate(std::string&) const;

        archive_l archives;
        buffer_m  buffers;

    public:

        data(std::string);
       ~data();

        void load();

        const void *load(std::string name,               size_t *size=0);
        bool        save(std::string name, const void *, size_t *size=0);
        void        list(std::string name, strset& dirs, strset& regs) const;
        void        free(std::string name);
    };
}

//-----------------------------------------------------------------------------

extern app::data *data;

//-----------------------------------------------------------------------------

#endif
