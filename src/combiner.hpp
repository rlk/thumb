/*    Copyright (C) 2006 Robert Kooima                                       */
/*                                                                           */
/*    Varrier Combiner is free software;  you can  redistribute it and/or    */
/*    modify  it under  the terms  of the  GNU General Public License  as    */
/*    published by the Free Software Foundation;  either version 2 of the    */
/*    License, or (at your option) any later version                         */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#ifndef VARRIER_COMBINER_H
#define VARRIER_COMBINER_H

/*---------------------------------------------------------------------------*/

struct vc_display
{
    int viewport_x;
    int viewport_y;
    int viewport_w;
    int viewport_h;

    float quality;

    float screen_BL[3];
    float screen_TL[3];
    float screen_BR[3];

    float pitch;
    float angle;
    float thick;
    float shift;
    float cycle;
};

/*---------------------------------------------------------------------------*/

int  vc_init(int, int);
void vc_fini(void);

/*---------------------------------------------------------------------------*/

void vc_prepare(const struct vc_display *, int);
void vc_combine(const struct vc_display *, const float[3], const float[3]);

/*---------------------------------------------------------------------------*/

void vc_frustum(const struct vc_display *, const float[3], float, float);

/*---------------------------------------------------------------------------*/

#endif
