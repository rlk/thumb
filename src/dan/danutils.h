#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef linux
#include <time.h>
#endif
#ifndef PI
#define PI 3.14159
#endif
#define DEGTORAD   3.14159/180.0
#define LOG2 3.321928

//get current time in secs
double getTimeInSecs();

//if val is out of range, clamps it to min/max
int ilimit(int min, int val, int max);
//returns the distance of the plane to the point
//(xp, yp, zp)
//The 3 points of the viewport plane are given by
//(x1, y1, z1)
//(x2, y2, z2)
//(x3, y3, z3)
float distPointPlane(float x1, float y1, float z1,float x2, float y2, float z2,

            float x3, float y3, float z3,float xp, float yp, float zp);



#endif
float ftToM(float ft);

float spiral(float time,float dist_per_turn,int axis);
