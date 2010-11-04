/* 
dan Sandin 8-22-10
*/

#include "danglobs.cpp"
__constant__ float  refldata[REFL_DATA_MUNB][REFL_DATA_ROWS][REFL_DATA_ROW_ELEM];
__constant__ float  injdata[INJT_DATA_MUNB][INJT_DATA_ROWS][INJT_DATA_ROW_ELEM];


///////////////////////////////////////////////////////////////////////////////
//! Simple partical system
//! @param data  data in global memory
///////////////////////////////////////////////////////////////////////////////

__device__ void  injector0(unsigned int arrayLoc,unsigned int posLoc,float time,float4* pos, float* pdata){
	//ovels
	//  sin(time) + x index/width, x  y are randomly selected because of randon nature of age
	// x afects angular velocity distribution x,y afects liniar velocity distribution
//     pdata[arrayLoc+1] = 0.02 * (sin(time/5 + (float)x/(float)width/10.0) * (float)(x * y )/ (float)(width * height)/1.0f ) ;//x velocity  sin(time) + x index/width, x is randomly selected because of randon nature of age
     pdata[arrayLoc+1] = 0.02 * (sin(time/5  + (pdata[arrayLoc+5] + 1)/50) * ( ((pdata[arrayLoc+5]  +1)/1 ) * (pdata[arrayLoc+4] + 1.0)/1)  ) ;//x velocity  sin(time) + x index/width, x is randomly selected because of randon nature of age

     pdata[arrayLoc+2] = 0;
	//ovels       
//       	pdata[arrayLoc+3] = 0.02 * (cos(time/5 + (float)x/(float)width/10.0) * (float)(x *y) / (float)(width * height)/1.0f );// y velocity
       	pdata[arrayLoc+3] = 0.02 * (cos(time/5  + (pdata[arrayLoc+5] + 1)/50)  *( ((pdata[arrayLoc+5]  +1)/1 ) * (pdata[arrayLoc+4] + 1.0)/1));// y velocity


       // maybe move the generation point around?
		
       
			{
			pos[posLoc].x = 0;
       		pos[posLoc].y = 0.5;
       		pos[posLoc].z = 0;
			}
}
__device__ float  distRnd1( float seed, int iter){

	unsigned int rndint1;
	rndint1 = (unsigned int)(((seed +1.0)/2.0) *32768) % 32768;
	float sum ;
	sum =0;
	for ( int i = 0;i<iter;i++)
		{
		rndint1 = ((rndint1 * 1103515245 + 12345)/65536) % 32768;
		sum = sum +  0.0002 * (rndint1 % 10000) -1.0;
		}

	return	sum/iter;		
}

__device__ void  injector1(unsigned int arrayLoc,unsigned int posLoc,float time,float4* pos, float* pdata){
	float rnd1,rnd2,rnd3;
	rnd1 = distRnd1(pdata[arrayLoc+4] , 5);
	rnd2 = distRnd1(pdata[arrayLoc+5] , 5);
	rnd3 = distRnd1(pdata[arrayLoc+6] , 5);

	
	pdata[arrayLoc+1] = 0.02 * (sin(time/5  + (rnd1)/50) * (rnd2 +1)) ;//x vloocity 
    pdata[arrayLoc+2] = 0.002 * rnd3;
    pdata[arrayLoc+3] = 0.02 * (cos(time/5  + (rnd1)/50)  *(rnd2 +1));	//y volocity

	pos[posLoc].x = 0;
    //pos[posLoc].y = pdata[7];
    pos[posLoc].y = 0;
     pos[posLoc].z = 0;
			
}
__device__ void  injector2(unsigned int arrayLoc,unsigned int posLoc,int injNum,float time,float4* pos, float* pdata,float* debugData){
	float rnd1,rnd2,rnd3,rnd4,rnd5;
	float dt,du,dx,dy,dz,dx2,dy2,dz2,len,vx,vy,vz,dxt,dyt,dzt,dxu,dyu,dzu;
// float dv
/*
	injdata[injNum][1][0]// type, injection ratio ie streem volume, ~
	injdata[injNum][2][0];//x,y,z position
	injdata[injNum][3][0];//x,y,z velocity
	injdata[injNum][4][0];//x,y,z size
	injdata[injNum][5][0];//t,u,v jiter v not implimented = speed 
	injdata[injNum][6][0];//speed jiter ~ 
	injdata[injNum][7][0];//centrality of rnd distribution speed dt tu 

*/
	if ((pdata[arrayLoc+4] +1) /2 >  injdata[injNum][1][1]){ return;}// reterns without injection ?????
	
	rnd1 = (distRnd1(pdata[arrayLoc+4] , (int)injdata[injNum][7][0])+1)/2;
	rnd2 = (distRnd1(pdata[arrayLoc+5] , (int)injdata[injNum][7][1])+1)/2;
	rnd3 = (distRnd1(pdata[arrayLoc+6] , (int)injdata[injNum][7][2])+1)/2;
	rnd4 = (distRnd1(pdata[arrayLoc+4],1) );
	rnd5 = (distRnd1(pdata[arrayLoc+5],1) );
	//float rnd6 = (distRnd1(pdata[arrayLoc+6],1) );
	vx = injdata[injNum][3][0];vy = injdata[injNum][3][1];vz = injdata[injNum][3][2];//direction of spray	

	dt = injdata[injNum][5][0];du = injdata[injNum][5][1];// dv = injdata[injecti +17] * 0;// z component not implimented jitterelitive to direction of spreay

	// vector vx,vy,vz X 0,1,0
	dx = -vz;dy = 0;dz = vx;//  dt directon

	len = sqrt(dx*dx +dy*dy + dz*dz);
	if (len ==0)
		{
		dx = 0;dy =0;dz =0;
		}
	else{
		dx =dx/len;dy =dy/len;dz =dz/len;
		}
		//scale by dt amout of jitter in dt direction
		dxt = dx *dt;dyt = dy * dt;dzt = dz *dt;

	// vector vx,vy,vz X 0,1,0 X vx,vy,vz 
	dx2 = vy*vx;dy2 = vz*vz-vx*vx;dz2 = vy*vz;// du direction
	len = sqrt(dx2*dx2 +dy2*dy2 + dz2*dz2);
	if (len ==0)
		{
		dx2 = 0;dy2 =0;dz2 =0;
		}
	else{
		dx2 =dx2/len;dy2 =dy2/len;dz2 =dz2/len;
		}
		//scale by du amout of jutter in du direction
		dxu = dx2 *du;dyu = dy2 * du;dzu = dz2 *du;

	
	//indesices num injectors =0,position =6,velosity =9, size =12 tuv jiter = 15,speed = 18,centrality of randum  
	//         3 +             speed component                          velocity          t jitter u jitter

	pdata[arrayLoc+1] = ( rnd1 * injdata[injNum][6][0]) * (injdata[injNum][3][0] + dxt * rnd2 + dxu * rnd3) ;//x vloocity 
    pdata[arrayLoc+2] = ( rnd1  * injdata[injNum][6][0]) * (injdata[injNum][3][1] + dyt * rnd2+ dyu * rnd3) ; // y velocity
    pdata[arrayLoc+3] = ( rnd1  * injdata[injNum][6][0]) * (injdata[injNum][3][2] + dzt * rnd2+ dzu * rnd3);	//z volocity

	// size computation  xform  to dt du dv

	dt = injdata[injNum][4][0];du = injdata[injNum][4][1];//dv = injdata[injecti +14] * 0;//re use varables z component not implimented jitterelitive to direction of spreay
	dxt = dx *dt;dyt = dy * dt;dzt = dz *dt;
	dxu = dx2 *du;dyu = dy2 * du;dzu = dz2 *du;

	pos[posLoc].x = injdata[injNum][2][0] +  dxt * rnd4 + dxu * rnd5;
   
	pos[posLoc].y = injdata[injNum][2][1] + dyt * rnd4 + dyu * rnd5 ;
 
    pos[posLoc].z = injdata[injNum][2][2]  + dzt * rnd4+ dzu * rnd5;
if (DEBUG == 1)
	{
	
	int dbi =0;
	debugData[dbi + 0] = (float)injNum ;debugData[dbi + 1] =  injdata[injNum][1][1];debugData[dbi + 2] =0;
	dbi=dbi+3;
	debugData[dbi + 0] = dx;debugData[dbi + 1] = dy;debugData[dbi + 2] = dz;
	dbi=dbi+3;
	debugData[dbi + 0] = dx2;debugData[dbi + 1] = dy2;debugData[dbi + 2] = dz2;
	dbi=dbi+3;
	debugData[dbi + 0] = dxt;debugData[dbi + 1] = dyt;debugData[dbi + 2] = dzt;
	dbi=dbi+3;
	debugData[dbi + 0] = dxu;debugData[dbi + 1] = dyu;debugData[dbi + 2] = dzu;
	
	}
			
}
	__device__ void  planeReflector1(float posX,float posY,float posZ,float newX, float newY,float newZ,unsigned int arrayLoc,unsigned int posLoc,int reflNum,float time,float4* pos, float* pdata,float* debugData)
{
	float xn =1,yn =1,zn =0, rad =1,damping =.7;
	float xp,yp,zp;

	//indexices num injectors =0,position =[reflNum][1][0],normal =[reflNum][2][0], size =[reflNum][3][0] tuv jiter = [reflNum ][4][0],damping = [reflNum ][4][0],centrality of randum = 21

//dataorginization  refldata[reflNum][rownum][quardinare numbr x=0,1=y,2=z]
//dataorginization  type rownum 0 ~~ ,position 1,normal 2,radis 3,reflection coef 5,

	xn = refldata[reflNum][2][0];yn = refldata[reflNum][2][1];zn = refldata[reflNum][2][2];//normal
	rad = refldata[reflNum][3][0];
	damping = refldata[reflNum][5][0];
	xp = refldata[reflNum][1][0];yp = refldata[reflNum][1][1];zp = refldata[reflNum][1][2];//reflector position



	float length = sqrt(xn * xn + yn * yn + zn * zn);
    xn = xn/length;
    yn = yn/length;
    zn = zn/length;

	float distx = newX - xp;//point position - reflector position
	float disty = newY - yp;
	float distz = newZ - zp;


	 	float xv = pdata[arrayLoc+1];float yv = pdata[arrayLoc+2];float zv = pdata[arrayLoc+3];

	   	if ((fabs(distx) <= rad) && (fabs(disty)<= rad) && (fabs(distz) <= rad))
		{
	
	   		if ((distx * xn + disty * yn + distz * zn) <=0)

			{

		  		float ndotv = xv * xn + yv * yn + zv * zn;
				
	      		float newVX =(xv - 2.0*ndotv*xn);
		       	float newVY =(yv - 2.0*ndotv*yn);
		     	float newVZ =(zv - 2.0*ndotv*zn);
				// experments to lower traping  did not work
				//damping =1;
	      		//pos[posLoc].x  = posX + newVX;
       			//pos[posLoc].y  = posY + newVY;
       			//pos[posLoc].z  = posZ + newVZ;
	      		//pos[posLoc].x  = posX - (newX - posX) + newVX;
       			//pos[posLoc].y  = posY - (newY - posY) + newVY;
       			//pos[posLoc].z  = posZ - (newZ - posZ  + newVZ);
		      	pdata[arrayLoc+1] =newVX*damping;
		       	pdata[arrayLoc+2] =newVY*damping;
		     	pdata[arrayLoc+3] =newVZ*damping;
				
				//pdata[arrayLoc] = 0;// temp set age to 0
				
			}
	   }

}

extern "C"
__global__ void Point1(float4* pos, float * pdata,float * debugData ,unsigned int width,
unsigned int height, int max_age, float time, float r1, float r2, float r3)
{

	// r1,r2,r3 curently not used
    unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;
    unsigned int y = blockIdx.y*blockDim.y + threadIdx.y;
	
    unsigned int arrayLoc = y*width*7 + x*7;
    unsigned int posLoc = y*width+x;
	float newX,newY,newZ,posX,posY,posZ;

 /*
  	arrayLoc is data index of partical in pdata
    pdata [arrayLoc] = age ,pdata[arrayLoc +1 +2  +3 = x ,y,z velocity +rd1 rnd2 rnd3

    posLoc is index of partical location and[width*height + posLoc] index of color
    pos[posLoc].x .y .z  is x,y,z pos
    pos[ [width*height + posLoc].x .y .z is red green blue color
	In lines mode vbo is twice as big with tail and head psitions
*/
	

  	if (pdata[arrayLoc] >= max_age)
		{
		
		int injecNum = ((arrayLoc/7) % (int) injdata[0][0][0]) +1;// pdata row mod number of injectors 
		injector2(arrayLoc,posLoc,injecNum,time,pos,pdata,debugData);
		pdata[arrayLoc] = 0;//set age to 0

        }

		posX=pos[posLoc].x;posY=pos[posLoc].y;posZ=pos[posLoc].z;
 
  // add velocity to position  ie intigrate
		{
      	newX = posX + pdata[arrayLoc+1];
       	newY = posY + pdata[arrayLoc+2];
       	newZ = posZ + pdata[arrayLoc+3];
		}


       pdata[arrayLoc] += 1;        // increase age
       pdata[arrayLoc+2] -= 0.00001; // gravity

 
	// reflector

		for (int reflNum = 1;reflNum <= refldata[0][0][0]  ;reflNum ++)
			{
				
				//planeReflector1( pos[posLoc].x, pos[posLoc].y, pos[posLoc].z,arrayLoc,posLoc,reflNum,time,pos,pdata,debugData);			
				planeReflector1(posX,posY,posZ, newX, newY, newZ,arrayLoc,posLoc,reflNum,time,pos,pdata,debugData);			
			}
	
 
	
	float colorFreq = 16.0f;
 
			{
   	 		pos[width*height + posLoc].y = (cos(colorFreq * 2.0 * pdata[arrayLoc]/max_age))/2.0f + 0.5f ;//green
    		pos[width*height + posLoc].x = (cos(colorFreq * 1.0 * pdata[arrayLoc]/max_age))/2.0f + 0.5f ;//red
    		pos[width*height + posLoc].z = (cos(colorFreq * 4.0 * pdata[arrayLoc]/max_age))/2.0f + 0.5f ;//blue
    		// write output vertex
    		 pos[posLoc] = make_float4(newX, newY, newZ, 1.0f);
			}

}
extern "C"
__global__ void PointSquars(float4* pos, float * pdata, unsigned int width,
unsigned int height, int max_age, float time, float r1, float r2, float r3)
{

	// r1,r2,r3 curently not used
    unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;
    unsigned int y = blockIdx.y*blockDim.y + threadIdx.y;

    unsigned int arrayLoc = y*width*4 + x*4;
    unsigned int posLoc = y*width+x;
	float newX,newY,newZ;

 /*
  	arrayLoc is data index of partical in pdata
    pdata [arrayLoc] = age ,pdata[arrayLoc +1 +2  +3 = x ,y,z velocity

    posLoc is index of partical location and[width*height + posLoc] index of color
    pos[posLoc].x .y .z  is x,y,z pos
    pos[ [width*height + posLoc].x .y .z is red green blue color
	In lines mode vbo is twice as big with tail and head psitions
*/

	if (pdata[arrayLoc] >= max_age)
		{
		pdata[arrayLoc] = 0;
		
      	 pdata[arrayLoc+1] = 0.002 * (sin(time) + (float)x / (float)width/10.0f ) ;//x velocity  sin(time) + x index/width, x is randomly selected because of randon nature of age


       	pdata[arrayLoc+2] = 0;
		
     	 pdata[arrayLoc+3] = 0.002 * (cos(time) + (float)(y) / (float)( height)/10.0f );// y velocity
	
		

       // maybe move the generation point around?
		

			{
			pos[posLoc].x = 0;
       		pos[posLoc].y = 0.5;
       		pos[posLoc].z = 0;
			}

       }

      // add velocity to position  ie intigrate
 			{
      		newX = pos[posLoc].x + pdata[arrayLoc+1];
       		newY = pos[posLoc].y + pdata[arrayLoc+2];
       		newZ = pos[posLoc].z + pdata[arrayLoc+3];
			}


       pdata[arrayLoc] += 1;        // increase age
       pdata[arrayLoc+2] -= 10.1; // gravity

       // check aganst tabletop surface reverse velocity
 			{

       		if ((newY <= 0) && fabs(pos[posLoc].x)<5 && fabs(pos[posLoc].z)<5)
           		{
           		//pdata[arrayLoc+2] = -0.7 * pdata[arrayLoc+2];
           		}
  			}



      // now need to modify the color info in the array
//      pos[width*height + posLoc].x = 0.0f;//red
//      pos[width*height + posLoc].y = 1.0f;//green
//      pos[width*height + posLoc].z = 0.0f;//blue
	float colorFreq = 16.0f;

			{
   	 		pos[width*height + posLoc].y = (cos(colorFreq * 2.0 * pdata[arrayLoc]/max_age))/2.0f + 0.5f ;
    		pos[width*height + posLoc].x = (cos(colorFreq * 1.0 * pdata[arrayLoc]/max_age))/2.0f + 0.5f ;
    		pos[width*height + posLoc].z = (cos(colorFreq * 4.0 * pdata[arrayLoc]/max_age))/2.0f + 0.5f ;
    		// write output vertex
    		 pos[posLoc] = make_float4(newX, newY, newZ, 1.0f);
			}


}





