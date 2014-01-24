//  Copyright (C) 204 Robert Kooima
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

#ifndef ETC_ZIP_HPP
#define ETC_ZIP_HPP

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

#endif
