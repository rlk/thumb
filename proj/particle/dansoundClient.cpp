#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include <signal.h>
#include <setjmp.h>
#include "dansoundClient.hpp"

//-----------------------------------------------------------------------------

float audioState[256][10];// index is handel,row is play state, loop state,playing gain,x,y,zposition,
int aPlayInd ;
int aLoopInd ;
int aGainInd ;
int aMaxHandel;
int aPosInd;
int aFadeStateInd,aFadeIncrimentInd,aFadeTargetGainInd,aFadeStopOnOutInd;
    
float aGlobalGain =1;
float aOldGlobalGain =0;

void error(char *msg);
void zerotmpbuffer();

//-----------------------------------------------------------------------------

int sockfd;
char tmpbuffer[256];
void error(char *msg)
{
    perror(msg);
    exit(0);
}
void zerotmpbuffer()
{
    
    for (int i=0;i<256;i++)
    {
        tmpbuffer[i] = '\0';
      
    }

}
/*
  int audioWriteFileToServer(char* filename1)
  {
  FILE *inputFilePtr;
  int lFileLen =0, n;
  char filename[256];
  sprintf(filename,"thunder22.wav");
  printf(" opining wave file \n");
  //inputFilePtr = fopen("harrypotter.wav", "rb");
  inputFilePtr = fopen(filename, "rb");
  if (inputFilePtr == 0) printf ( "cant open file\n ");
  fseek(inputFilePtr, 0L, SEEK_END);  // Position to end of file 
  lFileLen = ftell(inputFilePtr);     // Get file length 
  rewind(inputFilePtr);               // Back to start of file 
  printf(" length of file %i\n",lFileLen);
  char *cFile; 
  cFile = calloc(lFileLen + 1, sizeof(char));
  printf ("callioc \n");
  if(cFile == NULL )
  {
  printf("\nInsufficient memory to read file.\n");
  return 0;
  }
  //printf ("going to  read file into memory");

  fread(cFile, lFileLen, 1, inputFilePtr); // Read the entire file into cFile 
  printf ("file read into memory\n");
  char msg[256];
  sprintf(msg,  "PTFI %s %lu\r", filename, (unsigned long)lFileLen);
  printf ( " nessage %s \n",msg);
  n = write(sockfd,msg,strlen(msg));
  if (n < 0) error("ERROR writing to socket");
  n = write(sockfd,cFile,lFileLen );
  printf ("n %i \n",n);
  if (n < 0) error("ERROR writing to socket");

	

  }
*/

static jmp_buf env;
void we_get_signal(int)
{
    printf("Signal!\n");
    longjmp(env, 1);
}

int stockfd =0;
int audioConectToServer( const char* hostName)

{
    int  portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    portno = 31231;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        printf("ERROR opening socket");
    //server = gethostbyname("137.110.118.239");
    server = gethostbyname(hostName);
    if (server == NULL) {
        printf("ERROR, no such host\n");
        exit(0);
    }
    printf ( "socketmade \n");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);

    if (setjmp(env) == 0)
    {
        signal(SIGALRM, we_get_signal);
        alarm(2);
        if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        {
            printf("ERROR connecting to audio server\n");
        }
    }
    else
    {
        printf("AUDIO IS DISABLED\n");
        return 0;
    }
    alarm(0);

    printf ( "connected to audio server \n");
    //initualize sound varables 
    aPlayInd = 0;
    aLoopInd = 1;
    aGainInd = 2;
    aPosInd = 3;
    aFadeStateInd =6;aFadeIncrimentInd =7;aFadeTargetGainInd =8;aFadeStopOnOutInd =9;
    aMaxHandel = -1;
	
    for (int i =0;i < 255;i++)
    {
        audioState [i][aPlayInd]=0;
        audioState [i][aLoopInd]=0;
        audioState [i][aGainInd]=1;
        audioState [i][aPosInd]=0;
        audioState [i][aPosInd + 1]=0;
        audioState [i][aPosInd + 2]=0;

        audioState [i][aFadeStateInd]=0;
        audioState [i][aFadeIncrimentInd]=0;
        audioState [i][aFadeTargetGainInd]=1;
        audioState [i][aFadeStopOnOutInd]=0;
    }
    //float audioState[256][ 6];// index is handel,row is play state, loop state,playing gain, x,y,zPosition
    return 1;
}

//--------------------------------------------
int audioGetHand (const char* fileName)
{
    int handel;
    zerotmpbuffer();
    sprintf(tmpbuffer,"GHDL %s\n",fileName); 
//printf ("%s",tmpbuffer);
    int  	n = write(sockfd,tmpbuffer,strlen(tmpbuffer));
    n = read(sockfd,tmpbuffer,255);
    if (n < 0) printf("ERROR reading from socket");
    handel = atoi(tmpbuffer);
    if (handel > aMaxHandel){aMaxHandel = handel;}
    return handel;
}


int audioGetHandKludge (const char* fileName)
{
    int handel;
    zerotmpbuffer();
    sprintf(tmpbuffer,"GHDL %s\n",fileName); 
//printf ("%s",tmpbuffer);
    int  	n = write(sockfd,tmpbuffer,strlen(tmpbuffer));
    n = read(sockfd,tmpbuffer,255);
    if (n < 0) printf("ERROR reading from socket");
    handel = atoi(tmpbuffer);
    if (handel > aMaxHandel){aMaxHandel = handel;}
    audioGain(handel,0);audioPos(handel,0,0,0);audioLoop(handel,1);//audioPlay(handel,1.0);KLUDGE SEQUENCE MABY ONLY REQUIRES POS
    audioGain(handel,1);audioLoop(handel,0);

    return handel;
} 
//--------------------------------------
void audioPlay (int handel,float start)
{
    zerotmpbuffer();
    sprintf(tmpbuffer,"PLAY %i %f\n",handel,start); 
//PLAY <handle>  undocument start pos in file	Plays the sound referenced by the handle numbe
    write(sockfd,tmpbuffer,strlen(tmpbuffer));
//printf ("%s",tmpbuffer);
    audioState[handel][aPlayInd] = 1;
 
}
void audioPlayOnCnOnly (int handel,float start)
{
    if (audioState[handel][aPlayInd] != 1){audioPlay ( handel, start);}
}
//----------------------------------------------
void audioTest (int t)
{
//TEST <number> 	Plays different sounds assigned to the Windows standard events.
    zerotmpbuffer();
    sprintf(tmpbuffer,"TEST %i\n", t); 

    write(sockfd,tmpbuffer,strlen(tmpbuffer));
 
}
void audioStop (int handel)
{
//STOP <handle> 	Stops the sound referenced by the handle number
    zerotmpbuffer();
    sprintf(tmpbuffer,"STOP %i\n",handel); 
    write(sockfd,tmpbuffer,strlen(tmpbuffer));
    audioState[handel][aPlayInd] = 0;
}
void audioStopOnCnOnly (int handel)
{
    if (audioState[handel][aPlayInd] != 0){audioStop ( handel);}
}
//------------------------------------------------
void audioPos (int handel,float x,float y,float z)
{
//SSPO <handle> <x> <y> <z> 	Set sound position to {x,y,z} coordinates.
//printf (" x y z %f %f %f \n",x,y,z);
//float tz = z;z=y;y =-tz;// convert from Zback to Z up
    zerotmpbuffer();
    sprintf(tmpbuffer,"SSPO %i %f %f %f\n",handel,x,y,z); 
//printf (tmpbuffer);
    write(sockfd,tmpbuffer,strlen(tmpbuffer));
    audioState[handel][aPosInd] = x; audioState[handel][aPosInd + 1 ]= y;audioState[handel][aPosInd+2] = z;
}

void audioPosOnCnOnly (int handel,float x,float y,float z)
{
    if ((audioState [handel][aPosInd] != x) || (audioState [handel][aPosInd + 1] != y) || (audioState [handel][aPosInd +2] != z)){audioPos ( handel, x, y, z);} 
}


//-------------------------------------------------------
void audioGain (int handel,float g)
{
//SSVO <handle> <gain> 	Sets the sound gain, values from 0.0 to 1.0
    zerotmpbuffer();
    sprintf(tmpbuffer,"SSVO %i %f\n",handel,g*g *aGlobalGain*aGlobalGain); 
//printf("%s",tmpbuffer);
    write(sockfd,tmpbuffer,strlen(tmpbuffer));
    audioState[handel][aGainInd] = g;
}
void audioGainOnCnOnly (int handel,float g)
{
    if (audioState [handel][aGainInd] != g) {audioGain ( handel, g);}	
}
//-----------------------------------------------------


void audioGlobalGain(float g)
{
    aGlobalGain =g;
}

void audioPitch (int handel,float g)
{
//SSPI <handle> <gain> undocumented pitch change.
//Not Working
    zerotmpbuffer();
    sprintf(tmpbuffer,"SSPI %i %f\n",handel,g); 

    write(sockfd,tmpbuffer,strlen(tmpbuffer));
 
}

void audioLoop (int handel,int g)
{
//SSLP <handle> <loop> 	Enables (loop=1) or disables (loop=0) sound looping.
    zerotmpbuffer();
    sprintf(tmpbuffer,"SSLP %i %i\n",handel,g); 
//printf ("%s",tmpbuffer);
    write(sockfd,tmpbuffer,strlen(tmpbuffer));
    audioState[handel][aLoopInd] = g;
}
void audioLoopOnCnOnly (int handel,int g)
{
    if (audioState[handel][aLoopInd] != g){audioLoop ( handel, g);}
}

//------------------------------------------------
void audioVelocity (int handel,float g)
{
//SSVE <handle> <velocity> 	Sets the sound velocity, values from -1.0 to 1.0
    zerotmpbuffer();
    sprintf(tmpbuffer,"SSVE %i %f\n",handel,g); 
//printf("%s", tmpbuffer);
    write(sockfd,tmpbuffer,strlen(tmpbuffer));
 
}

void audioDirectionDeg (int handel,float g)
{
//SSDI <handle> <direction>
    zerotmpbuffer();
    sprintf(tmpbuffer,"SSDI %i %f\n",handel,g); 

    write(sockfd,tmpbuffer,strlen(tmpbuffer));
 
}

void audioDirectionVec (int handel,float x,float y,float z)
{
//SSDI <handle> <direction>
    zerotmpbuffer();
    sprintf(tmpbuffer,"SSDI %i %f %f %f\n",handel,x,y,z); 

    write(sockfd,tmpbuffer,strlen(tmpbuffer));
 
}

void audioDirectionDegGain (int handel,float d,float g)
{
//SSDV <handle> <direction> <gain> 	Sets the sound direction and gain in one command.
    zerotmpbuffer();
    sprintf(tmpbuffer,"SSDV %i %f %f\n",handel,d,g*g*aGlobalGain*aGlobalGain); 

    write(sockfd,tmpbuffer,strlen(tmpbuffer));
 
}

void audioDirectionDegRelHead (int handel,float d)
{
//SSDR <handle> <direction> 	Sets the sound direction relative to the listener position. + - 360
    zerotmpbuffer();
    sprintf(tmpbuffer,"SSDR %i %f\n",handel,d); 

    write(sockfd,tmpbuffer,strlen(tmpbuffer));
 
}
/*
  void audioDirectionDegRelHeadGain (int handel,float d,float g)
  {
//SSDV <handle> <direction> <gain> 	Sets the sound direction and gain in one command.
zerotmpbuffer();
sprintf(tmpbuffer,"SSDV %i %f %f\n",handel,d,g); 

write(sockfd,tmpbuffer,strlen(tmpbuffer));
 
}
*/
void audioReleasesHand (int handel)
{
//RHDL <handle>  	Releases handle. The Audio Server unloads the sound assigned to that handle number
    zerotmpbuffer();
    sprintf(tmpbuffer,"RHDL %i\n",handel); 
//printf ("%s",tmpbuffer);
    write(sockfd,tmpbuffer,strlen(tmpbuffer));
 
}

void audioQuit ()
{
//QUIT  	Releases sound sources and closes the connection
//zerotmpbuffer();
//sprintf(tmpbuffer,"QUIT \n"); 
//write(sockfd,tmpbuffer,strlen(tmpbuffer));
    for (int i =0; i <= aMaxHandel;i++)
    {
	audioStop(i);audioReleasesHand(i);
    }
//	for ( float j=0;j<100000000;j++){ float q = sqrt(3.1);}
}

void audioProcess()
{
	
    for (int i =0;i< aMaxHandel+1;i++)
    {
			
        //if (audioState [i][aFadeStateInd] == 0){return;}
        if (audioState [i][aFadeStateInd] == -1)
        {
            audioState [i][aGainInd] += audioState [i][aFadeIncrimentInd];

            if 	( audioState [i][aGainInd] <=0)
            {
                audioState [i][aGainInd]=0.0;
                audioState [i][aFadeStateInd] =0;
                if (audioState [i][aFadeStopOnOutInd] == 1){audioStop(i);audioState [i][aFadeStopOnOutInd]=0;}
            }
            audioGain(i,audioState [i][aGainInd]);//printf (" index audioState [i][aGainInd] %i %f \n",i,audioState [i][aGainInd]);
				
        }

        if (audioState [i][aFadeStateInd] == 1)
        {
            audioState [i][aGainInd] += audioState [i][aFadeIncrimentInd];

            if 	( audioState [i][aGainInd] >= audioState [i][aFadeTargetGainInd] )
            {
                audioState [i][aGainInd]=audioState [i][aFadeTargetGainInd];
                audioState [i][aFadeStateInd] =0;
            }
            audioGain(i,audioState [i][aGainInd]);
				
        }


        if ((aGlobalGain != aOldGlobalGain)&&(audioState [i][aFadeStateInd] == 0))
        {
            audioGain(i,audioState [i][aGainInd]);  
        }

    }
    aOldGlobalGain = aGlobalGain;
}


void audioFadeOut(int handel,float time,int releaseFileHandel)
{
    audioState [handel][aFadeIncrimentInd] = -audioState [handel][aGainInd]/time;
    audioState [handel][aFadeStateInd]= -1;
    if (releaseFileHandel != -1){audioPlay(releaseFileHandel,.01);}
    audioState [handel][aFadeStopOnOutInd]=0;
}


void audioFadeOutStop( int handel,float time,int releaseFileHandel)
{
    audioFadeOut( handel, time, releaseFileHandel);
    audioState [handel][aFadeStopOnOutInd]=1;
}

void audioFadeUp(int handel,float time,float targetGain,int AttackFileHandel)
{
    audioState [handel][aFadeIncrimentInd] = targetGain/time;
    audioState [handel][aFadeStateInd]=1;
    if (AttackFileHandel != -1){audioPlay(AttackFileHandel,.01);}
    audioState [handel][aFadeStopOnOutInd]=0;
    audioState [handel][aFadeTargetGainInd]=targetGain;
}

void audioFadeUpPlay(int handel,float time,float targetGain,int AttackFileHandel)
{
    audioPlay(handel,.01);
    audioFadeUp( handel, time, targetGain, AttackFileHandel);


}



/*
  QUIT  	Releases sound sources and closes the connection
  RHDL <handle>  	Releases handle. The Audio Server unloads the sound assigned to that handle number
  SSRV <handle> <direction> <gain>
  SSRV <handle> <x> <y> <z> <gain>
*/
/*
  int main(int argc, char *argv[])
  {

  char buffer[256];

  int  portno, n,chimes,pinkNoise,whiteNoise,harmonicAlgorithm,thunder22;

  audioConectToServer( "192.168.2.109");
  //char filename[256];
  //sprintf(filename,"harrypotter.wav");

//	 audioWriteFileToServer("temp");
printf ("back from file load");
thunder22 = audioGetHand("thunder22.wav");
	
chimes =audioGetHand("chimes.wav");
printf(" chimes %i \n",chimes);

pinkNoise =audioGetHand("cdtds.31.pinkNoise.wav");
printf ("pinkNoise %i \n",pinkNoise);
whiteNoise =audioGetHand("cdtds.34.whiteNoise.wav");
printf ("whiteNoise %i \n",whiteNoise);
harmonicAlgorithm = audioGetHand("harmonicAlgorithm.wav");
printf ("harmonicAlgorithm %i \n",harmonicAlgorithm);
printf("Please hit return: ");bzero(buffer,256);fgets(buffer,255,stdin); 
//audioPlay(thunder22,0);
printf("Please hit return: ");bzero(buffer,256);fgets(buffer,255,stdin);
audioLoop(harmonicAlgorithm,1);
audioPlay(harmonicAlgorithm,0);
int i;
for( i=0;i<3;i++){
	
printf("Please hit return: ");bzero(buffer,256);fgets(buffer,255,stdin); 
audioStop (harmonicAlgorithm);
printf("Please hit return: ");bzero(buffer,256);fgets(buffer,255,stdin); 
audioPlay (harmonicAlgorithm,100);
printf("Please hit return: ");bzero(buffer,256);fgets(buffer,255,stdin); 
 
audioPos (harmonicAlgorithm,1,0,1);
printf("Please hit return: ");bzero(buffer,256);fgets(buffer,255,stdin); 
audioVelocity (harmonicAlgorithm,0.5);
printf("Please hit return: ");bzero(buffer,256);fgets(buffer,255,stdin); 
audioVelocity (harmonicAlgorithm,-0.5);
printf("Please hit return: ");bzero(buffer,256);fgets(buffer,255,stdin); 
audioDirectionDegGain (harmonicAlgorithm,0,1);
printf("Please hit return: ");bzero(buffer,256);fgets(buffer,255,stdin); 
audioDirectionDegGain (harmonicAlgorithm,90,1);
printf("Please hit return: ");bzero(buffer,256);fgets(buffer,255,stdin); 
audioDirectionDegGain (harmonicAlgorithm,180,1);
printf("Please hit return: ");bzero(buffer,256);fgets(buffer,255,stdin); 
audioVelocity (harmonicAlgorithm,0);
printf("Please hit return: ");bzero(buffer,256);fgets(buffer,255,stdin); 

audioTest(2);

}


	
	
while (10)
{
printf("Please enter the message: ");
bzero(buffer,256);
fgets(buffer,255,stdin);
n = write(sockfd,buffer,strlen(buffer));
if (n < 0) 
error("ERROR writing to socket");
bzero(buffer,256);

}
// n = read(sockfd,buffer,255);
//if (n < 0) 
//     error("ERROR reading from socket");
//printf("%s\n",buffer);
return 0;
}
*/
