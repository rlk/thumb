#ifndef DANSOUNDCLIENT_HPP
#define DANSOUNDCLIENT_HPP


/*
int audioWriteFileToServer(char* filename1);
*/
int audioConectToServer( const char* hostName);

int audioGetHand (const char* fileName);
int audioGetHandKludge (const char* fileName);
void audioPlay (int handel,float start);
void audioPlayOnCnOnly (int handel,float start);

void audioTest (int t);
void audioStop (int handel);
void audioStopOnCnOnly (int handel);
void audioPos (int handel,float x,float y,float z);
void audioPosOnCnOnly (int handel,float x,float y,float z);

void audioGain (int handel,float g);
void audioGlobalGain(float);
void audioGainOnCnOnly (int handel,float g);

void audioPitch (int handel,float g);

void audioLoop (int handel,int g);
void audioLoopOnCnOnly (int handel,int g);

void audioVelocity (int handel,float g);

void audioDirectionDeg (int handel,float g);

void audioDirectionVec (int handel,float x,float y,float z);

void audioDirectionDegGain (int handel,float d,float g);

void audioDirectionDegRelHead (int handel,float d);
/*
void audioDirectionDegRelHeadGain (int handel,float d,float g);
*/
void audioReleasesHand (int handel);

void audioQuit();

void audioProcess();
void audioFadeOut(int handel,float time,int releaseFileHandel);
void audioFadeOutStop(int handel,float time,int releaseFileHandel);
void audioFadeUp(int handel,float time,float targetGain,int AttackFileHandel);
void audioFadeUpPlay(int handel,float time,float targetGain,int AttackFileHandel);

#endif
