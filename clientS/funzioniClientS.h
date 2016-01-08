//
//  funzioniClientS.h
//  clientS
//


#ifndef __clientS__funzioniClientS__
#define __clientS__funzioniClientS__

#include "../ScrutinoElettronico.h"

int visualizzaAppelli(int socket,app_t *appelliRicevuti);
int visualizzaEsami(int socket,long id,exam_t *esamiRicevuti);
int modificaDatiAppello(int socket);
int modificaDatiEsame(int socket);
void visionaAppelliEdEsami(int socket);

#endif /* defined(__clientS__funzioniClientS__) */
