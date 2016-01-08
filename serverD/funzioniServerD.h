//
//  funzioniServerD.h
//  serverD
//

#ifndef __serverD__funzioniServerD__
#define __serverD__funzioniServerD__

#include "../ScrutinoElettronico.h"

int autentica(int fd,user_t *connected);
int creaAppello(int fd,int servSFd);
void visualizzaAppelli(int fd,user_t *connected);
int registraEsame(int fd,user_t *connected,int servFd);
int chiudiAppello(int fd,user_t *connected,int servfd);
void creaAccount();
void visualizzaEsami(int fd);
int connectToServerS();
int modificaAppello(int fd);
int modificaEsame(int fd);

#endif /* defined(__serverD__funzioniServerD__) */

