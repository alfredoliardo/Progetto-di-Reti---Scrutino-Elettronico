//
//  main.c
//  clientS
//



#include "../ScrutinoElettronico.h"
#include "funzioniClientS.h"

int main(int argc, const char * argv[]) {
    
    int sock;
    struct sockaddr_in servAddr;
    comm_t command = CONNECT;
    
    char buffer[BUFF_SIZE];
    long chose = 0;
    
    //create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Errore creazione socket");
        exit(-1);
    }
    
    //initialize address
    memset((void *) &servAddr, 0, sizeof(servAddr));//clear server address
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERVERS_PORT);//2346
    
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
    printf("Benvento/a ,cosa vuoi fare?\n");
    do{
        printf("1 - Visiona appelli ed esami\n2 - Modifica appello\n3 - modifica esame\n");
        printf("$-%s(0 - per terminare): ","segreteriaStudenti");
        fgets(buffer, sizeof(buffer), stdin);
        chose = atoi(buffer);
        switch (chose) {
            case 1:
                visionaAppelliEdEsami(sock);
                break;
            case 2:
                modificaDatiAppello(sock);
                break;
            case 3:
                modificaDatiEsame(sock);
                break;
            case 0:
                printf("Disconnessione...\n");
                shutdown(sock, SHUT_WR);
                return 0;
                break;
            default:
                printf("Comando sconosciuto\n");
                break;
        }
        printf("Vuoi fare altro?\n1 - Si\n0 - Termina\n$-segreteriaStudenti: ");
        fgets(buffer, sizeof(buffer), stdin);
        chose = atol(buffer);
    }while(chose != 0);
}
