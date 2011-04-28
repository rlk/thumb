#include "danutils.h"

int ilimit(int min, int val, int max)
{
    if(val<=min)
	return min;
    if(val>=max)
	return max;
    return val;
}

#include <sys/time.h>

double getTimeInSecs()
{
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    
    return (double) tv.tv_sec
         + (double) tv.tv_usec / 1000000.0;
}

#if 0
double getTimeInSecs() {
    const double oneNanoSec = 0.000000001;
        timespec temp;
	   //clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &temp);
	   //clock_gettime(CLOCK_THREAD_CPUTIME_ID, &temp);
	  clock_gettime(CLOCK_MONOTONIC, &temp);
	   //clock_gettime(CLOCK_REALTIME, &temp);
	   //printf ( " %f  %f \n",(float)temp.tv_sec,(float)temp.tv_nsec);
        return (((double) temp.tv_sec) +
                (((double) temp.tv_nsec) * oneNanoSec)*1.0);

}
#endif
/*	timespec time1, time2;
	int temp;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);

	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;

*/

//this function calculate the distance of the plane to the point
//(xp, yp, zp)
//The 3 points of the viewport plane are given by
//(x1, y1, z1)
//(x2, y2, z2)
//(x3, y3, z3)
float distPointPlane(float x1, float y1, float z1,float x2, float y2, float z2,
					 float x3, float y3, float z3,float xp, float yp, float zp)
{
	float r,d,xx1,yy1,zz1,xx2,yy2,zz2,xn,yn,zn;
	x1=x1-xp;y1=y1-yp;z1=z1-zp;
	x2=x2-xp;y2=y2-yp;z2=z2-zp;
	x3=x3-xp;y3=y3-yp;z3=z3-zp;
	//calculate vector from pt1 to pt3 and
	//calculate vector from pt2 to pt3
	xx1 = x1-x3; yy1=y1-y3; zz1 = z1-z3;
	xx2 = x2-x3; yy2=y2-y3; zz2 = z2-z3;
	//calculate the unit normal to this plane (xn, yn, zn)
	xn = (yy1*zz2-yy2*zz1);
	yn = (xx2*zz1-xx1*zz2);
	zn = (xx1*yy2-yy1*xx2);
	r = sqrt(xn*xn +yn*yn+zn*zn); 
	xn = xn/r;
	yn = yn/r;
	zn = zn/r;
	//calculate the distance of plane to the point
	d = fabs(x3*xn + y3*yn + z3*zn);
	return d;
}
	float  distRnd1( float seed, int iter){

		unsigned int rndint1;
		rndint1 = (unsigned int)(((seed +1.0)/2.0) *32768) % 32768;
		//printf ("seed  partial rnd rndint1 seed  %f  %i  %i    " ,seed,(unsigned int)(((seed +1.0)/2.0) * 32768) ,rndint1);
		float sum ;
		sum =0;
		for ( int i = 0;i<iter;i++)
			{
			rndint1 = ((rndint1 * 1103515245 + 12345)/65536) % 32768;
			//printf ("rndint1  %i   \n " ,rndint1);
			sum = sum +  0.0002 * (rndint1 % 10000) -1.0;
			}

	return	sum/iter;		
}

float ftToM(float ft)
{
return 0.3048 * ft;
}


float spiral(float time,float dist_per_turn,int axis)
{
if (axis == 0) return dist_per_turn *time/dist_per_turn *  sin(time/(2*PI));
if (axis == 1) return dist_per_turn *time/dist_per_turn *   cos(time/(2*PI));
printf (" in spiral axis is not 10 or 1 \n");
return 0.0;

}

