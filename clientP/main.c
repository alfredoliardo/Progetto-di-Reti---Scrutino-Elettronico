//
//  main.c
//  clientP
//

#include "funzioniClientP.h"

user_t account;

int main(int argc, char * argv[]) {
    int sock;
    char buffer[BUFF_SIZE];
    struct sockaddr_in servAddr;
    int logged = 0, chose = 0;
    comm_t command;
    
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
    if ((logged = login(sock)) < 0) {//tenta il login
        perror("Errore login");
        exit(1);
    }
    if (logged == 0) { //autenticazione fallita
        printf("Autenticazione fallita\n");
        exit(1);
    }
    //autenticazione andata a buon fine
    printf("Benvento/a ,cosa vuoi fare?\n");
    do {
        printMenu();
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            chose = atoi(buffer);
        }
        //scanf("%d",&chose);
        //while (getchar()!='\n') ;  // svuoto il buffer da tastiera
        switch (chose) {
            case 1://crea appello
                creaAppello(sock);
                break;
            case 2://registra esame
                registraEsame(sock);
                break;
            case 3://chiudi appello
                chiudiAppello(sock);
                break;
            case 0://termina
                command = CLOSE;
                write(sock, &command, sizeof(comm_t));//invio segnale chiusura
                printf("Disconnessione\n");
                shutdown(sock, SHUT_WR);
                return 0;
                break;
            default:
                printf("Comando sconosciuto\n");
                break;
        }
    } while (1);
    
    return 0;
}

















