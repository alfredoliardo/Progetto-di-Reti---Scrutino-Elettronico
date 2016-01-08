//
//  funzioniServerD.c
//  serverD
//

#include "funzioniServerD.h"

int autentica(int fd,user_t *connected){
    FILE *file;
    comm_t command;
    user_t account, acc_to_cmp;

    read(fd, &account, sizeof(account));//legge l'account ricevuto
    file = fopen("login.dat", "rb");
    if (file == NULL) {
        command = FILE_ERR;
        write(fd, &command, sizeof(comm_t));
        perror("impossibile aprire il file");
        exit(1);
    }
    while (!feof(file)) {//legge finchè non finisce il file
        fread(&acc_to_cmp, sizeof(acc_to_cmp), 1, file);//recupera un account dal file
        if (!strcmp(account.matricola, acc_to_cmp.matricola)) {//se l'utente esiste(strcmp ritorna 0 se sono uguali)
            if (!strcmp(account.password, acc_to_cmp.password)) {//controlla la password
                command = SUCCESS;
                write(fd, &command, sizeof(comm_t));//se coincidono concedi l'accesso
                fclose(file);
                connected[fd] = account;
                return 1;
            }
            else{//se non coincidono
                command =  WRONG_PASS;
                write(fd, &command, sizeof(comm_t));//nega l'accesso
                fclose(file);
                return 0;
            }
        }
    }//se l'utente non esiste
    command = NOT_EXIST;
    write(fd, &command, sizeof(comm_t));//nega accesso
    fclose(file);
    return 0;
}

int creaAppello(int fd,int servSFd){
    
    app_t appello, tmp;
    ssize_t nwrite;
    comm_t command = FAIL, response = FAIL;
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
        /*Invio il tutto al serverS*/
        command = NEW_APP;
        write(servSFd, &command, sizeof(comm_t));//invio al serverS il comando nuovo appello
        FullWrite(servSFd, &appello, sizeof(app_t));//gli invio i dati ricevuti dal client
        read(servSFd, &response, sizeof(comm_t));//leggo l'esito
        switch (response) {
            case FILE_ERR:
                printf("Impossibile accedere al DB\n");
                return 0;
                break;
            case SUCCESS:
                printf("Appello creato con successo!\n");
                return 1;
                break;
            case FAIL:
            default:
                printf("Qualcosa e' andato storto\n");
                return 0;
                break;
        }
        /*fine della comunicazione al serverS*/
    }
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

int registraEsame(int fd,user_t *connected,int servFd){
    int n;
    size_t nread;
    FILE *fp,*fpAppelli;
    comm_t command = FAIL,response;
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
                /*Invio tutto al serverS*/
                command = REC;
                write(servFd, &command, sizeof(comm_t));//invio comando per registrare
                write(servFd, &n, sizeof(int));//invio il numero di esami da regisrare
                FullWrite(servFd, daRegistrare, sizeof(exam_t) * n);//invio array con gli esami da registrare
                read(servFd, &response, sizeof(comm_t));//leggo esito fopen
                free(daRegistrare);
                if (response == FILE_ERR) {
                    printf("ServerS non riesce ad aprire il file\n");
                    return 0;
                }
                read(servFd, &response, sizeof(comm_t));//leggo esito finale
                switch (response) {
                    case SUCCESS:
                        return 1;
                        break;
                    case CLOSE_APP:
                        printf("Impossibile regigistrare: appello chiuso\n");
                        return 0;
                    case FAIL:
                    default:
                        printf("Qualcosa è andato storto\n");
                        return 0;
                        break;
                }
                /*Fine gestione serverS*/
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

int chiudiAppello(int fd,user_t *connected,int servFd){
    comm_t command, response;
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
                /*Invio tutto al severS*/
                command = CLOSE_APP;
                write(servFd, &command, sizeof(comm_t));//invio comando al server
                read(servFd, &response, sizeof(comm_t));//legge esito fopen
                if (response == FILE_ERR) {
                    printf("ServerS non riesce ad aprire file\n");
                    return 0;
                }
                write(servFd, &idDaChiudere, sizeof(long));//invio l'id dell'appello da chiudere
                read(servFd, &response, sizeof(comm_t));
                switch (response) {
                    case CLOSE_APP:
                        printf("Impossibile appello gia' chiuso\n");
                        return 0;
                        break;
                    case NOT_EXIST:
                        printf("Impossibile appello inesistente\n");
                        return 0;
                        break;
                    case SUCCESS:
                        return 1;
                        break;
                    case FAIL:
                    default:
                        printf("Qualcosa e' andato storto\n");
                        return 0;
                        break;
                }
                /*fine gestione serverS*/
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

void creaAccount(){
    FILE *file;
    user_t account;
    int chose = 0;
    char buffer[BUFF_SIZE];
    
    printf("Sever in modalita' manutenzione:\nDesideri aggiungere un account?\n1 - Si\n0 - Termina\n");
    printf("$-%s: ",account.matricola);
    if (fgets(buffer, sizeof(buffer), stdin)) {
        chose = atoi(buffer);
    }
    file = fopen("login.dat","ab+");
    while (chose != 0) {
        printf("matricola:");
        fgets(buffer, sizeof(buffer), stdin);
        sscanf(buffer, "%10s",account.matricola);
        printf("pass:");
        fgets(buffer, sizeof(buffer), stdin);
        sscanf(buffer, "%13s",account.password);
        account.categoria = P;
        fwrite(&account,sizeof(account),1,file);
        printf("Vuoi continuare?\n1 - Si\n0 - Termina\n$-%s: ",account.matricola);
        fgets(buffer, sizeof(buffer), stdin);
        chose = atoi(buffer);
    }
    fclose(file);
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


int connectToServerS(){
    int fd;
    struct sockaddr_in servAddr;
    //create socket
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Errore creazione socket");
        exit(-1);
    }
    
    //initialize address
    memset((void *) &servAddr, 0, sizeof(servAddr));//clear server address
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERVERS_PORT);//2346
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
    
    return fd;
}

int modificaAppello(int fd){
    comm_t command;
    app_t app,app_mod;
    size_t nread;
    FILE *fp;
    
    FullRead(fd, &app_mod, sizeof(app_t));//ricevo appello da modificare
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
            return 1;
        }
    }//appello non trovato
    fclose(fp);
    command = NOT_EXIST;
    write(fd, &command, sizeof(comm_t));//invio esito negativo
    return 0;
}

int modificaEsame(int fd){
    FILE *fp;
    comm_t command;
    exam_t esa_mod, esame;
    size_t nread;
    
    
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
            return 1;
        }
    }//arrivato qui esame non trovato
    command = FAIL;
    write(fd, &command, sizeof(comm_t));//invio esito operazione negativo
    return 0;
}



