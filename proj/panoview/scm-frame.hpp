//  Copyright (C) 2005-2012 Robert Kooima
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

#ifndef SCM_FRAME_HPP
#define SCM_FRAME_HPP

//------------------------------------------------------------------------------

class scm_frame
{
    bool mono;

    struct image
    {
        int file;
        int shader;
        int channel;
    };

public:

    scm_frame(scm_cache *, app::node);

    int file(int i)
    {
        return images.empty() ? 0 : images[i % images.size()].file;
    }
    int shader(int i)
    {
        return images.empty() ? 0 : images[i % images.size()].shader;
    }
    int channel(int i)
    {
        return images.empty() ? 0 : images[i % images.size()].channel;
    }
    int size()
    {
        return images.size();
    }
    void apply(int c, std::vector<int>& tovert,
                      std::vector<int>& tofrag)
    {
        std::vector<image>::iterator i;

        for (i = images.begin(); i != images.end(); ++i)
            if (mono || c == i->channel)
            {
                if (i->shader)
                    tofrag.push_back(i->file);
                else
                    tovert.push_back(i->file);
            }
    }

private:

    std::vector<image> images;
};

//------------------------------------------------------------------------------

#endif
