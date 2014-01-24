//  Copyright (C) 2014 Robert Kooima
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

#include <app-data-pack.hpp>
#include <app-conf.hpp>

//-----------------------------------------------------------------------------

#pragma pack(push,2)

struct local_file_header
{
    uint32_t signature;
    uint16_t reader_version;
    uint16_t flags;
    uint16_t compression;
    uint16_t modified_time;
    uint16_t modified_date;
    uint32_t crc;
    uint32_t sizeof_compressed;
    uint32_t sizeof_uncompressed;
    uint16_t sizeof_name;
    uint16_t sizeof_extra;
};

struct file_header
{
    uint32_t signature;
    uint16_t writer_version;
    uint16_t reader_version;
    uint16_t flags;
    uint16_t compression;
    uint16_t modified_time;
    uint16_t modified_date;
    uint32_t crc;
    uint32_t sizeof_compressed;
    uint32_t sizeof_uncompressed;
    uint16_t sizeof_name;
    uint16_t sizeof_extra;
    uint16_t sizeof_comment;
    uint16_t disk_number_start;
    uint16_t internal_attr;
    uint32_t external_attr;
    uint32_t offset;
};

struct eocd
{
    uint32_t signature;
    uint16_t disk_number;
    uint16_t disk_number_start;
    uint16_t file_count;
    uint16_t file_count_total;
    uint32_t directory_size;
    uint32_t directory_offset;
    uint16_t sizeof_comment;
};

#pragma pack(pop)

//-----------------------------------------------------------------------------

bool app::pack_archive::find(std::string name) const
{
    return false;
}

app::buffer_p app::pack_archive::load(std::string name) const
{
    return 0;
}

bool app::pack_archive::save(std::string name,
                             const void *ptr, size_t *len) const
{
    return false;
}

void app::pack_archive::list(std::string dirname, str_set& dirs,
                                                  str_set& regs) const
{
    const eocd    *d = (const eocd *) (ptr + len - sizeof (eocd));
    const uint8_t *p = ptr + d->directory_offset;

    const std::string path = dirname.empty() ? dirname : dirname + "/";

    if (d->signature == 0x06054B50)
    {
        for (int i = 0; i < d->file_count; ++i)
        {
            // Get a pointer to the file header and a string of the file name.

            const file_header *f = (const file_header *) p;
            std::string pathname((const char *) (f + 1), f->sizeof_name);

            // If the path matches, add the name to the file or directory list.

            if (pathname.compare(0, path.size(), path) == 0)
            {
                std::string name(pathname, path.size());
                std::string::size_type n = name.find('/');

                if (n == std::string::npos)
                    regs.insert(name);
                else
                    dirs.insert(std::string(name, 0, n));
            }

            // Skip past the header, name, extra, and comment.

            p += sizeof (file_header) + f->sizeof_name
                                      + f->sizeof_extra
                                      + f->sizeof_comment;
        }
    }
}

//-----------------------------------------------------------------------------
