//
//  main.c
//  serverS
//

#include "../ScrutinoElettronico.h"
#include "funzioniServerS.h"

#define BACK_LOG 10

int main(int argc,char * argv[]) {
    int logging = 0;
    int reuse = 0;
    
    struct sockaddr_in servAddr, clientAddr;
    int list_fd, fd, servDfd;
    socklen_t len;
    
    char buffer[BUFF_SIZE];
    long nread;
    time_t timeval;
    int opt;//options
    comm_t chose;
    //I/O multiplexing variables
    fd_set rfset;
    int max_fd, i, n;
    int fd_open[FD_SETSIZE];
    user_t connected[FD_SETSIZE];

    
    //handling options
    while ((opt = getopt(argc, argv, "lrc")) != -1) {
        switch (opt) {
            case 'l': //logging option
                logging = 1;
                break;
            case 'r':
                reuse = 1;
                break;
            default:
                break;
        }
    }
    
    //create scocket
    printf("Creazione socket... ");
    if( (list_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Errore creazione socket");
        exit(-1);
    }printf("ok\n");
    
    //initialize address
    memset((void *)&servAddr, 0, sizeof(servAddr));//clear server address
    servAddr.sin_family = AF_INET; //address type is INET
    servAddr.sin_port = htons(SERVERS_PORT);//SeverD port is 2346
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); //connect from anywhere
    
    
    setsockopt(list_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    //bind socket
    printf("Assegnazione indirizzo... ");
    if (bind(list_fd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        perror("\nErrore assegnazione indirizzo");
        exit(-1);
    }printf("ok\n");
    
    //listen on socket
    printf("Server in ascolto... ");
    if (listen(list_fd, BACK_LOG) < 0) {
        perror("\nErrore ,impossibile mettere in ascolto");
        exit(-1);
    }printf("ok\n");
    
    servDfd = connectToServerD();//mi connetto al serverD
    
    printf("Attendo connessioni...  \n");
    
    //main loop
    memset(fd_open, 0, FD_SETSIZE);//clear fd_open
    max_fd = list_fd;//the only one list_fd is the max_fd
    fd_open[max_fd] = 1; //mark list_fd as active
    while (1) {
        FD_ZERO(&rfset);//clear rfset
        for (i = list_fd; i <= max_fd; i++) {//initialize fd_open
            if (fd_open[i] != 0) {
                FD_SET(i,&rfset); //insert i-fd into readeable files' set
            }
        }
        while((n = select(max_fd+1, &rfset, NULL, NULL, NULL)) < 0 && (errno == EINTR));
        if (n < 0) { //on real error exit
            perror("errore select");
            exit(1);
        }
        if (FD_ISSET(list_fd,&rfset)) {//a new connection
            n--;//decrement active
            len = sizeof(clientAddr);//and call accept
            if ((fd = accept(list_fd, (struct sockaddr *)&clientAddr, &len)) < 0) {
                perror("errore accept");
                exit(1);
            }
            if(logging){
                timeval = time(NULL);
                inet_ntop(AF_INET, &clientAddr.sin_addr, buffer, sizeof(buffer));
                printf("Connessione da: %s:%d\t%.24s\r\n",buffer,ntohs(clientAddr.sin_port),ctime(&timeval));
            }
            fd_open[fd] = 1;//set new connection socket
            if (max_fd < fd) {//if needed update max_fd
                max_fd = fd;
            }
        }
        
        //loop on open connections
        i = list_fd; //start from list_fd
        while (n != 0) {
            i++; //start after listening socket
            if (fd_open[i] == 0) {//if closed
                continue;//go next
            }
            if (FD_ISSET(i,&rfset)) {//if active, process it
                n--;//decrement active connections
                len = sizeof(clientAddr);
                nread = read(i, &chose, sizeof(comm_t));//leggo il comando
                if (nread < 0) {
                    perror("errore lettura");
                    exit(1);
                }
                if (nread == 0) {//if closed connection
                    close(i); //close file
                    fd_open[i] = 0; //mark as closed in the table
                    if (max_fd == i) {//if was the maximum
                        while(fd_open[i--] == 0);//loop down
                        max_fd = i;//set new maximum
                        break;//and go back to select
                    }
                    continue;//continue loop on open
                }
                switch (chose) {//controllo che tipo di comando ho ricevuto
                    case NEW_APP://crea un appello
                        if (creaAppello(i)) {
                            if (logging) {
                                printf("Nuovo appello creato da %s|%s:%d\t%.24s\r",connected[i].matricola,buffer,ntohs(clientAddr.sin_port),ctime(&timeval));
                            }
                        }
                        break;
                    case SHOW:
                        visualizzaAppelli(i, connected);
                        break;
                    case REC:
                        registraEsame(i, connected);
                        break;
                    case CLOSE_APP:
                        chiudiAppello(i, connected);
                        break;
                    case SHOW_EX:
                        visualizzaEsami(i);
                        break;
                    case CONNECT:
                        connected[i].categoria = S;
                        break;
                    case MOD_APP:
                        modificaAppello(i, servDfd);
                        break;
                    case MOD_EX:
                        modificaEsame(i, servDfd);
                        break;
                    default:
                        printf("Comando sconosciuto\n");
                        break;
                }
            }
        }
        
    }
    return 0;
}
