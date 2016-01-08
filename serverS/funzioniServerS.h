//
//  funzioniServerS.h
//  serverS
//

#ifndef __serverS__funzioniServerS__
#define __serverS__funzioniServerS__

#include "../ScrutinoElettronico.h"

int connectToServerD();
int creaAppello(int fd);
int registraEsame(int fd,user_t *connected);
int chiudiAppello(int fd,user_t *connected);
void visualizzaAppelli(int fd,user_t *connected);
void visualizzaEsami(int fd);
int modificaAppello(int fd,int servFd);
int modificaEsame(int fd,int servFd);
void logit(int fd);

#endif /* defined(__serverS__funzioniServerS__) */
