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

#include <cmath>
#include <cstring>

#include "frustum.hpp"
#include "matrix.hpp"
#include "util.hpp"

//-----------------------------------------------------------------------------

void app::frustum::calc_corner_4(double fov, double rat)
{
    const double x = tan(RAD(fov * 0.5));
    const double y = x / rat;

    BL[0] = -x; BL[1] = -y; BL[2] = -1;
    BR[0] = +x; BR[1] = -y; BR[2] = -1;
    TL[0] = -x; TL[1] = +y; TL[2] = -1;
    TR[0] = +x; TR[1] = +y; TR[2] = -1;
}

void app::frustum::calc_corner_1(double *D, const double *A,
                                            const double *B,
                                            const double *C)
{
    D[0] = B[0] + C[0] - A[0];
    D[1] = B[1] + C[1] - A[1];
    D[2] = B[2] + C[2] - A[2];
}

//-----------------------------------------------------------------------------

app::frustum::frustum(mxml_node_t *node) : node(node), n(1), f(10)
{
    const char *c = 0;
    mxml_node_t *curr;

    bool got_BL = false;
    bool got_BR = false;
    bool got_TL = false;
    bool got_TR = false;

    double fov = 0;
    double rat = 0;

    VP[0] = VP[1] = VP[2] = 0;

    load_idt(T);
    load_idt(M);

    if (node)
    {
        // Extract the screen corners.

        MXML_FORALL(node, curr, "corner")
        {
            double *v = 0;

            // Determine which corner is being specified.

            if (const char *name = mxmlElementGetAttr(curr, "name"))
            {
                if      (strcmp(name, "BL") == 0) { v = BL; got_BL = true; }
                else if (strcmp(name, "BR") == 0) { v = BR; got_BR = true; }
                else if (strcmp(name, "TL") == 0) { v = TL; got_TL = true; }
                else if (strcmp(name, "TR") == 0) { v = TR; got_TR = true; }
            }

            if (v)
            {
                // Extract the position.

                v[0] = get_attr_f(curr, "x");
                v[1] = get_attr_f(curr, "y");
                v[2] = get_attr_f(curr, "z");

                // Convert dimensions if necessary.

                if ((c = mxmlElementGetAttr(curr, "dim")))
                {
                    if (strcmp(c, "mm") == 0)
                    {
                        v[0] /= 304.8;
                        v[1] /= 304.8;
                        v[2] /= 304.8;
                    }
                }
            }
        }

        // Extract field-of-view and aspect ratio.

        if ((curr = mxmlFindElement(node, node, "perspective",
                                    0, 0, MXML_DESCEND)))
        {
            fov = get_attr_f(curr, "fov");
            rat = get_attr_f(curr, "aspect");
        }

        // Extract the calibration matrix.

        if ((curr = mxmlFindElement(node, node, "calibration",
                                    0, 0, MXML_DESCEND)))
        {
            M[ 0] = get_real_attr(curr, "m0");
            M[ 1] = get_real_attr(curr, "m1");
            M[ 2] = get_real_attr(curr, "m2");
            M[ 3] = get_real_attr(curr, "m3");
            M[ 4] = get_real_attr(curr, "m4");
            M[ 5] = get_real_attr(curr, "m5");
            M[ 6] = get_real_attr(curr, "m6");
            M[ 7] = get_real_attr(curr, "m7");
            M[ 8] = get_real_attr(curr, "m8");
            M[ 9] = get_real_attr(curr, "m9");
            M[10] = get_real_attr(curr, "mA");
            M[11] = get_real_attr(curr, "mB");
            M[12] = get_real_attr(curr, "mC");
            M[13] = get_real_attr(curr, "mD");
            M[14] = get_real_attr(curr, "mE");
            M[15] = get_real_attr(curr, "mF");
        }
    }

    // Compute any remaining screen corner.

    if (!got_TR && got_BL && got_BR && got_TL) calc_corner_1(TR, BL, BR, TL);
    if (!got_TL && got_BR && got_TR && got_BL) calc_corner_1(TL, BR, TR, BL);
    if (!got_BL && got_TR && got_TL && got_BR) calc_corner_1(BL, TR, TL, BR);
    if (!got_BR && got_TL && got_BL && got_TR) calc_corner_1(BR, TL, BL, TR);

    // Compute screen corners from field-of-view and aspect ratio.

    if (!got_BL && !got_BR && !got_TL && !got_TR)
    {
        if (fov > 0 && rat > 0)
            calc_corner_4(fov, rat);
        else
            calc_corner_4(90, 0.75);
    }
}

//-----------------------------------------------------------------------------
