//
//  main.c
//  clientD
//

#include <stdio.h>
#include "../ScrutinoElettronico.h"
#include "funzioniClientD.h"

int main(int argc, const char * argv[]) {
    
    int sock;
    struct sockaddr_in servAddr;
    comm_t command = CONNECT;
    
    app_t appelliDaVisualizzare[100];
    char buffer[BUFF_SIZE];
    int napp = 0;
    long chose = 0;
    
    //create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Errore creazione socket");
        exit(-1);
    }
    
    //initialize address
    memset((void *) &servAddr, 0, sizeof(servAddr));//clear server address
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERVERD_PORT);//2345
    
    //build address using inet_pton(presentation to network)
    if (inet_pton(AF_INET, argv[optind], &servAddr.sin_addr) <= 0) {
        perror("Errore creazione indirizzo");
        exit(-1);
    }
    
    //extablish connection
    if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr) )< 0) {
        perror("Errore connessione");
        exit(-1);
    }printf("Connessione... ok\n");
    
    write(sock, &command, sizeof(comm_t));//Comando Connect al server per far capire che non un prof
    //gestire la connessione
    printf("Benvento/a ,ecco gli appelli disponibili:\n");
    do{
        napp = visualizzaAppelli(sock, appelliDaVisualizzare);
        printf("Inserisci l'id dell'appello che vuoi aprire(0 - per terminare): ");
        fgets(buffer, sizeof(buffer), stdin);
        chose = atol(buffer);
        if (chose == 0) {
            break;
        }else if (bsearch(&chose, appelliDaVisualizzare, napp, sizeof(app_t), (int(*)(const void*,const void*))compareApp_t)== NULL) {
            printf("Impossibile: l'appello selezionato è inesisente.\n");
            continue;
        }
        //visualizza esami per appello selezionato
        visualizzaEsami(sock, chose, NULL);
        printf("Vuoi controllare un altro appello?\n1 - Si\n0 - Termina\n$-segreteriaDidattica: ");
        fgets(buffer, sizeof(buffer), stdin);
        chose = atol(buffer);
    }while(chose != 0);
    printf("Disconnessione...\n");
    shutdown(sock, SHUT_WR);
    return 0;
}
