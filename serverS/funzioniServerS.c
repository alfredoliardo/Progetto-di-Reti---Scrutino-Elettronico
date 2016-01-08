//
//  funzioniServerS.c
//  serverS
//


#include "funzioniServerS.h"

int creaAppello(int fd){
    
    app_t appello, tmp;
    ssize_t nwrite;
    comm_t command = FAIL;
    FILE *fp;
    
    FullRead(fd, &appello, sizeof(app_t));//legge le info appello
    
    if ((fp = fopen("appelli.dat", "ab+")) == NULL) {//apre il file in scrittura e se non esiste lo crea
        command = FILE_ERR;
        write(fd, &command, sizeof(comm_t));//invio esito al clientP
        perror("Errore apertura file: appelli.dat");
        return 0;
    }
    appello.id = 1;
    fseek(fp, -sizeof(app_t), SEEK_CUR);//sposto il cursore di appello indietro
    if(1 == fread(&tmp, sizeof(app_t), 1, fp)){//se riesco a leggere un appello
        appello.id = tmp.id + 1;//imposto l'id del nuovo appello sommando 1 a quello letto
    }
    
    nwrite = fwrite(&appello, sizeof(app_t), 1, fp);//registra appello nel file
    
    fclose(fp);
    
    if (nwrite < 0) {
        write(fd, &command, sizeof(comm_t));
        perror("errore scrittura file");
        return 0;
    }else{
        command = SUCCESS;
        write(fd, &command, sizeof(comm_t));//invio esito al clientP
        return 1;
    }
}

int connectToServerD(){
    int fd;
    comm_t command = IMS;
    
    struct sockaddr_in servAddr;
    //create socket
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Errore creazione socket");
        exit(-1);
    }
    
    //initialize address
    memset((void *) &servAddr, 0, sizeof(servAddr));//clear server address
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERVERD_PORT);//2345
    //build address using inet_pton(presentation to network)
    if (inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr) <= 0) {
        perror("Errore creazione indirizzo");
        exit(-1);
    }
    
    //extablish connection
    if (connect(fd, (struct sockaddr *) &servAddr, sizeof(servAddr) )< 0) {
        perror("Errore connessione");
        exit(-1);
    }printf("Connessione... ok\n");
    
    write(fd, &command, sizeof(comm_t));//dico al serverD chi sono
    
    return fd;
}

int registraEsame(int fd,user_t *connected){
    int n;
    size_t nread;
    FILE *fp,*fpAppelli;
    comm_t command = FAIL;
    exam_t *daRegistrare;
    app_t tmp;
    
    read(fd, &n, sizeof(int));//leggo quanti esami devo registrare
    daRegistrare = malloc(sizeof(exam_t) * (n+1));
    FullRead(fd, daRegistrare, sizeof(exam_t) * n);//memorizzo gli esami ricevuti
    if ((fp = fopen("esami.dat", "ab+")) == NULL) {
        command = FILE_ERR;
        write(fd, &command, sizeof(comm_t));//invio esito negativo
        perror("Errore apertura file: esami.dat");
        return 0;
    }
    /**DEVO CONTROLLARE SE L'APPELLO E CHIUSO*/
    if ((fpAppelli = fopen("appelli.dat", "rb")) == NULL) {
        command = FILE_ERR;
        write(fd, &command, sizeof(comm_t));//invio esito negativo
        perror("appelli.dat");
        return 0;
    }
    command = SUCCESS;
    write(fd, &command, sizeof(comm_t));//invio esito positio per i file
    while (1 == (nread = fread(&tmp, sizeof(app_t), 1, fpAppelli))) {//cerco appello
        if (daRegistrare[0].id == tmp.id) {//se ho trovato l'appello
            if (tmp.stato) {//se è aperto
                fwrite(daRegistrare, sizeof(exam_t), n, fp);//scrivo nel file
                command = SUCCESS;
                write(fd, &command, sizeof(comm_t));//invio esito positivo
                fclose(fp);
                free(daRegistrare);
                return 1;
            }else{//è chiuso
                fclose(fp);
                free(daRegistrare);
                command = CLOSE_APP;
                write(fd, &command, sizeof(comm_t));//invio esito negativo
                return 0;
            }
        }
    }//appello non trovato se arrivati a questo punto
    command = NOT_EXIST;
    write(fd, &command, sizeof(comm_t));//invio esito negativo
    fclose(fpAppelli);
    fclose(fp);
    free(daRegistrare);
    return 0;
}

int chiudiAppello(int fd,user_t *connected){
    comm_t command;
    long idDaChiudere;
    app_t appello;
    FILE *fp;
    size_t nread;
    
    if ((fp = fopen("appelli.dat", "rb+")) == NULL) {
        command = FILE_ERR;
        write(fd, &command, sizeof(comm_t));//invio esito negativo
        perror("Errore apertura file: appelli.dat");
        return 0;
    }
    command = SUCCESS;
    write(fd, &command, sizeof(comm_t));//esito positivo
    read(fd, &idDaChiudere, sizeof(long));//legge l'id dell'appello da chiudere
    while (1 == (nread = fread(&appello, sizeof(appello), 1, fp))) {
        if (appello.id == idDaChiudere) {//se l'appello esiste
            if (appello.stato == 1) {//e se è aperto
                appello.stato = 0;//chiudilo
                fseek(fp, -sizeof(app_t), SEEK_CUR); //mi sposto di un appello all'indietro
                fwrite(&appello, sizeof(app_t), 1, fp);//cosi da poter sovrascrivere l'appello desiderato
                command = SUCCESS;
                write(fd, &command, sizeof(comm_t));//invia responso positivo
                fclose(fp);
                return 1;
            }else{
                command = CLOSE_APP;
                write(fd, &command, sizeof(comm_t));//invio responso appello gia chiuso
                fclose(fp);
                return 0;
            }
        }
    }//se arriviamo fin qui l'appello non esiste
    command = NOT_EXIST;
    write(fd, &command, sizeof(comm_t));//invio respondo appello non trovato
    fclose(fp);
    return 0;
}

void visualizzaAppelli(int fd,user_t *connected){
    FILE *fp;
    ssize_t nread;
    app_t appello;
    app_t * daInviare;
    comm_t command = SUCCESS;
    long n = 0, i;
    
    if ((fp = fopen("appelli.dat", "rb")) == NULL) {//se ho problemi nell'apertura del file
        command = FILE_ERR;
        write(fd, &command, sizeof(comm_t));//invio esito negativo
        perror("Errore apertura file: appelli.dat");
        return ;
    }//altrimenti
    write(fd, &command, sizeof(comm_t));//invio esito positivo
    fseek(fp, 0, SEEK_END);
    n = ftell(fp)/sizeof(app_t); //li conta tutti
    daInviare = malloc(sizeof(app_t) * (n + 1));
    rewind(fp);
    if (connected[fd].categoria == P) {//se è un professore
        n = 0;
        while (1 == (nread = fread(&appello, sizeof(appello), 1, fp))) {//legge ogni appello
            if (!strcmp(connected[fd].matricola, appello.matricolaProfessore)) {//se ne è propritario
                daInviare[n++] = appello;//lo conta e lo inserisce nell'array
            }
        }
        write(fd, &n, sizeof(int));//invia numero di appelli
        for (i = 0; i < n; i++) {
            FullWrite(fd, &daInviare[i], sizeof(app_t));
        }
    }else{
        write(fd, &n, sizeof(int));//invia numero di appelli
        while (1 == (nread = fread(&appello, sizeof(appello), 1, fp))) {
            FullWrite(fd, &appello, sizeof(app_t));//invio n appelli
        }
    }
    fclose(fp);
    free(daInviare);
}

void visualizzaEsami(int fd){
    long appId;
    int n = 0, i;
    exam_t esame, esamiDaInviare[100];
    size_t nread;
    FILE *fp;
    
    read(fd, &appId, sizeof(long));//legge l'id dell'appello da aprire
    if ((fp = fopen("esami.dat", "rb")) == NULL) {
        perror("esami.dat");
        return;
    }
    while (1 == (nread = fread(&esame, sizeof(esame), 1, fp))) {
        if (esame.id == appId) {
            esamiDaInviare[n++] = esame;
        }
    }
    write(fd, &n, sizeof(int));//invio il numero di esami da ricevere
    for (i = 0; i < n; i++) {
        FullWrite(fd, &esamiDaInviare[i], sizeof(exam_t));
    }
}

int modificaAppello(int fd,int servFd){
    comm_t command;
    app_t app,app_mod;
    size_t nread;
    FILE *fp, *log;
    time_t timeval;
    struct sockaddr_in clientAddr;
    socklen_t len;
    long id;
    char buffer[BUFF_SIZE];
    
    FullRead(fd, &app_mod, sizeof(app_t));//ricevo appello da modificare
    id = app_mod.id;
    if ((fp = fopen("appelli.dat", "rb+")) == NULL) {
        command = FILE_ERR;
        write(fd, &command, sizeof(command));//invio esito fopen negativo
        perror("appelli.dat");
        return 0;
    }
    command = SUCCESS;
    write(fd, &command, sizeof(comm_t));//invio esito fopen positivo
    while (1 == (nread = fread(&app, sizeof(app_t), 1, fp))) {//legge ogni appello
        if (app.id == app_mod.id) {//se l'ho trovato
            fseek(fp, -sizeof(app_t), SEEK_CUR); //mi sposto di un appello all'indietro
            fwrite(&app_mod, sizeof(app_t), 1, fp);//cosi da poter sovrascrivere l'appello desiderato
            command = SUCCESS;
            write(fd, &command, sizeof(comm_t));//invia responso positivo
            fclose(fp);
            /*Registro modifica nel file*/
            if ((log = fopen("log.txt", "a+")) == NULL) {
                perror("Errore apertura file: log.dat");
                exit(1);
            }
            timeval = time(NULL);
            len = sizeof(clientAddr);
            getpeername(fd, (struct sockaddr *)&clientAddr, &len);
            inet_ntop(AF_INET, (struct sockaddr *)&clientAddr.sin_addr, buffer, sizeof(buffer));
            fprintf(log, "Modifica effettuata da %s:%d\t%.24s\r\n",buffer,ntohs(clientAddr.sin_port),ctime(&timeval));
            fclose(fp);
            /*Invio tutto al severD*/
            command = MOD_APP;
            write(servFd, &command, sizeof(comm_t));//invio comando MOD_APP
            app_mod.id = id;
            FullWrite(servFd, &app_mod, sizeof(app_t));//invio appello modificato
            read(servFd, &command, sizeof(comm_t));//ricevo esito fopen
            if (command == FILE_ERR) {
                printf("Errore apertura file nel serverD\n");
                return 0;
            }
            read(servFd, &command, sizeof(comm_t));//ricevo esito operazione
            switch (command) {
                case NOT_EXIST:
                    printf("Appello inesistente\n");
                    return 0;
                    break;
                case SUCCESS:
                    printf("Appello modificato!\n");
                    return 1;
                    break;
                default:
                    printf("Qualcosa è andato storto");
                    return 0;
                    break;
            }
            /*FINE invio al serverD*/
        }
    }//appello non trovato
    fclose(fp);
    command = NOT_EXIST;
    write(fd, &command, sizeof(comm_t));//invio esito negativo
    return 0;
}

int modificaEsame(int fd,int servFd){
    comm_t command;
    exam_t esa_mod, esame;
    size_t nread;
    FILE *fp, *log;
    time_t timeval;
    struct sockaddr_in clientAddr;
    socklen_t len;
    char buffer[BUFF_SIZE];
    
    if ((fp = fopen("esami.dat", "rb+")) == NULL) {
        command = FILE_ERR;
        write(fd, &command, sizeof(comm_t));//invio esito open negativo
    }
    command = SUCCESS;
    write(fd, &command, sizeof(comm_t));//invio esito fopen positivo
    FullRead(fd, &esa_mod, sizeof(exam_t));//legge esame modificato
    while (1 == (nread = fread(&esame, sizeof(exam_t), 1, fp))) {//leggo ogni esame
        if (esame.id == esa_mod.id && !strcmp(esame.matricola, esa_mod.matricola)) {//esame trovato
            fseek(fp, -sizeof(exam_t), SEEK_CUR); //mi sposto di un appello all'indietro
            fwrite(&esa_mod, sizeof(exam_t), 1, fp);//cosi da poter sovrascrivere l'appello desiderato
            command = SUCCESS;
            write(fd, &command, sizeof(comm_t));//invia responso positivo
            fclose(fp);
            /*Registro modifica nel file*/
            if ((log = fopen("log.txt", "a+")) == NULL) {
                perror("Errore apertura file: log.dat");
                exit(1);
            }
            timeval = time(NULL);
            len = sizeof(clientAddr);
            getpeername(fd, (struct sockaddr *)&clientAddr.sin_addr, &len);
            inet_ntop(AF_INET, (struct sockaddr *)&clientAddr, buffer, sizeof(buffer));
            fprintf(log, "Modifica effettuata da %s:%d\t%.24s\r\n",buffer,ntohs(clientAddr.sin_port),ctime(&timeval));
            fclose(fp);
            /*Invio tutto al severD*/
            command = MOD_EX;
            write(servFd, &command, sizeof(comm_t));//invio comando MOD_APP
            read(servFd, &command, sizeof(comm_t));//leggo esito fopen
            if (command == FILE_ERR) {
                printf("Impossibile, accesso negato\n");
                return 0;
            }
            FullWrite(servFd, &esa_mod, sizeof(exam_t));//invio esame modificato
            read(servFd, &command, sizeof(comm_t));//legge esito operazione
            switch (command) {
                case SUCCESS:
                    printf("Esame modificato\n");
                    return 1;
                    break;
                case FAIL:
                default:
                    printf("Qualcosa è andato storto\n");
                    return 0;
                    break;
            }
            /*FINE invio al serverD*/
        }
    }//arrivato qui esame non trovato
    command = FAIL;
    write(fd, &command, sizeof(comm_t));//invio esito operazione negativo
    return 0;
}

void logit(int fd){
    FILE *fp;
    time_t timeval;
    struct sockaddr_in clientAddr;
    socklen_t len;
    char buffer[BUFF_SIZE];
    
    if ((fp = fopen("log.txt", "a+")) == NULL) {
        perror("Errore apertura file: log.dat");
        exit(1);
    }
    timeval = time(NULL);
    len = sizeof(clientAddr);
    getpeername(fd, (struct sockaddr *)&clientAddr.sin_addr, &len);
    inet_ntop(AF_INET, (struct sockaddr *)&clientAddr, buffer, sizeof(buffer));
    fprintf(fp, "Modifica effettuata da %s:%d\t%.24s\r\n",buffer,ntohs(clientAddr.sin_port),ctime(&timeval));
    fsync(fileno(fp));
    fclose(fp);
}
















