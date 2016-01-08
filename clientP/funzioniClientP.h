//
//  funzioniClientP.h
//  clientP
//


#ifndef __clientP__funzioniClientP__
#define __clientP__funzioniClientP__

#include <stdio.h>
#include "../ScrutinoElettronico.h"

int login(int socket);
void printMenu();
int creaAppello(int socket);
int registraEsame(int socket);
int visualizzaAppelli(int socket,app_t *appelliRicevuti);
int chiudiAppello(int socket);

#endif /* defined(__clientP__funzioniClientP__) */
