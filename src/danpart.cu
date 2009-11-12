/*
 * Copyright 1993-2007 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and
 * international Copyright laws.  Users and possessors of this source code
 * are hereby granted a nonexclusive, royalty-free license to use this code
 * in individual and commercial software.
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION,  ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOURCE CODE.
 *
 * U.S. Government End Users.   This source code is a "commercial item" as
 * that term is defined at  48 C.F.R. 2.101 (OCT 1995), consisting  of
 * "commercial computer  software"  and "commercial computer software
 * documentation" as such terms are  used in 48 C.F.R. 12.212 (SEPT 1995)
 * and is provided to the U.S. Government only as a commercial end item.
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
 * source code with only those rights set forth herein.
 *
 * Any use of this source code in individual and commercial software must
 * include, in the user documentation and internal comments to the code,
 * the above Disclaimer and U.S. Government End Users Notice.
 */

 /* This example demonstrates how to use the Cuda OpenGL bindings with the
  * runtime API.
  * Device code.
  */

//#ifndef _SIMPLEGL_KERNEL_H_
//#define _SIMPLEGL_KERNEL_H_

///////////////////////////////////////////////////////////////////////////////
//! Simple kernel to modify vertex positions in sine wave pattern
//! @param data  data in global memory
///////////////////////////////////////////////////////////////////////////////

extern "C"
__global__ void danpart(float4* pos, float * pdata, unsigned int width,
unsigned int height, int max_age, float time, float r1, float r2, float r3)
{
	// r1,r2,r3 curently not used
    unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;
    unsigned int y = blockIdx.y*blockDim.y + threadIdx.y;

    unsigned int arrayLoc = y*width*4 + x*4;
    unsigned int posLoc = y*width+x;

    if (pdata[arrayLoc] >= max_age)
       {
       pdata[arrayLoc] = 0;
       pdata[arrayLoc+1] = 0.002 * (sin(time) + (float)x / (float)width/10.0f ) ;

       pdata[arrayLoc+2] = 0;
       pdata[arrayLoc+3] = 0.002 * (cos(time) + (float)y / (float)height/10.0f );

       // maybe move the generation point around?

       pos[posLoc].x = 0;
       pos[posLoc].y = .5;
       pos[posLoc].z = 0;
       }
      
       float newX = pos[posLoc].x + pdata[arrayLoc+1];
       float newY = pos[posLoc].y + pdata[arrayLoc+2];
       float newZ = pos[posLoc].z + pdata[arrayLoc+3];

       pdata[arrayLoc] += 1;        // increase age
       pdata[arrayLoc+2] -= 0.0001; // gravity

       // tabletop surface
       if ((newY <= 0) && fabs(pos[posLoc].x)<5 && fabs(pos[posLoc].z)<5)
           {
           pdata[arrayLoc+2] = -0.7 * pdata[arrayLoc+2];
           }

      // now need to modify the color info in the array
//      pos[width*height + posLoc].x = 0.0f;//red
//      pos[width*height + posLoc].y = 1.0f;//green
//      pos[width*height + posLoc].z = 0.0f;//blue
	float colorFreq = 16.0f;
    pos[width*height + posLoc].y = (cos(colorFreq * 2.0 * pdata[arrayLoc]/max_age))/2.0f + 0.5f ;
    pos[width*height + posLoc].x = (cos(colorFreq * 1.0 * pdata[arrayLoc]/max_age))/2.0f + 0.5f ;
    pos[width*height + posLoc].z = (cos(colorFreq * 4.0 * pdata[arrayLoc]/max_age))/2.0f + 0.5f ;


    // write output vertex
     pos[posLoc] = make_float4(newX, newY, newZ, 1.0f);
}


//#endif // #ifndef _SIMPLEGL_KERNEL_H_
