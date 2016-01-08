//
//  funzioniClientD.h
//  clientD
//


#ifndef __clientD__funzioniClientD__
#define __clientD__funzioniClientD__

#include "../ScrutinoElettronico.h"

int visualizzaAppelli(int socket,app_t *appelliRicevuti);
int visualizzaEsami(int socket,long id,exam_t *esamiRicevuti);

#endif /* defined(__clientD__funzioniClientD__) */
