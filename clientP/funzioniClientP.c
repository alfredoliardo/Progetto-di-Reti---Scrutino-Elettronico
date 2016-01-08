//
//  funzioniClientP.c
//  clientP
//


#include "funzioniClientP.h"

user_t account;

int login(int socket){
    comm_t command = LOGIN;
    comm_t response = FAIL;
    char buffer[BUFF_SIZE];
    ssize_t nwrite;
    int try = 0;
    do {
        //acquisisco le credenziali
        memset(&account, 0, sizeof(account));
        printf("Matricola: ");
        if(fgets(buffer, sizeof(buffer), stdin) != NULL){
            sscanf(buffer, "%10s",account.matricola);
        }
        //scanf("%10s", account.matricola);
        printf("Password: ");
        //scanf("%13s", account.password);
        if(fgets(buffer, sizeof(buffer), stdin) != NULL){
            sscanf(buffer, "%13s",account.password);
        }
        
        //avviso al server che sto per autenticarmi
        if((nwrite = write(socket, &command, sizeof(comm_t)) ) < 0){
            perror("errore scrittura");
            exit(1);
        }
        account.categoria = P;
        //gli invio le credenziali
        write(socket, &account, sizeof(user_t));
        //leggola risposta del server
        read(socket, &response, sizeof(comm_t));
        switch (response) {
            case SUCCESS:
                return 1;
                break;
            case WRONG_PASS:
            case NOT_EXIST:
                printf("Utente e/o password errati.\n");
                break;
            case FILE_ERR:
                printf("Impossibile loggarsi al momento, riprova più tardi.\n");
                break;
            case FAIL:
            default:
                printf("Qualcosa e' andato storto\n");
                break;
        }
    } while (try++ < 3 && (response == WRONG_PASS || response ==NOT_EXIST));//fino a tre tentativi
    return 0;//autenticazione fallita
}

void printMenu(){
    printf("1 - Crea appello\n2 - Registra esame\n3 - Chiudi appello\n0 - Termina\n$-%s: ",account.matricola);
}

int creaAppello(int socket){
    app_t new_app;
    char data[11], orario[6], buffer[BUFF_SIZE];
    struct tm data_t;
    char *token;
    comm_t command = NEW_APP , response = FAIL;
    
    write(socket, &command, sizeof(comm_t));//invio il comando per creare appelli
    
    strcpy(new_app.matricolaProfessore, account.matricola);//salvo la matricola di chi lo ha creato
    /*prendo le informazioni dell'appello*/
    printf("Dipartimeto: ");
    if(fgets(new_app.dipartimento, sizeof(new_app.dipartimento), stdin) != NULL){
        new_app.dipartimento[strlen(new_app.dipartimento)-1] = '\0';
    }
    
    printf("Nome CDL: ");
    //fflush(stdin); // svuoto il buffer da tastiera
    if(fgets(new_app.corsoDiLaurea, sizeof(new_app.corsoDiLaurea), stdin) != NULL){
        new_app.corsoDiLaurea[strlen(new_app.corsoDiLaurea)-1] = '\0';
    }
    printf("Nome esame: ");
    //fflush(stdin); // svuoto il buffer da tastiera
    if(fgets(new_app.nomeEsame, sizeof(new_app.nomeEsame), stdin)){
        new_app.nomeEsame[strlen(new_app.nomeEsame) -1] = '\0';
    }
    
    printf("Data appello(gg/mm/yyyy): ");
    //fflush(stdin);// svuoto il buffer da tastiera
    //scanf("%10s",data);
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        sscanf(buffer, "%10s",data);
    }
    data_t.tm_mday = atoi((token = strtok(data, "-/")));//estrae il giorno
    data_t.tm_mon = atoi((token = strtok(NULL, "-/")));//estrae il mese
    data_t.tm_year = atoi((token = strtok(NULL, "-/"))) - 1900;//estrae l'anno
    
    printf("Orario(hh:mm): ");
    //fflush(stdin); // svuoto il buffer da tastiera
    //scanf("%5s",orario);
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        sscanf(buffer, "%5s",orario);
    }
    data_t.tm_hour = atoi((token = strtok(orario, ":")));//estrae l'ora
    data_t.tm_min = atoi((token = strtok(NULL, ":")));//estrae i minuti
    data_t.tm_sec = 0;
    data_t.tm_isdst = 0;
    new_app.data = mktime(&data_t);
    
    new_app.stato = 1;
    
    /*Fine inserimento dati*/
    
    FullWrite(socket, &new_app, sizeof(app_t));//invio info appello
    
    read(socket, &response, sizeof(comm_t));//leggo l'esito
    
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
}

int visualizzaAppelli(int socket,app_t *appelliRicevuti){
    comm_t command = SHOW, response = FILE_ERR;
    int n = 0, i,flag = 0;//numero di appelli da ricevere
    app_t a;
    
    if (appelliRicevuti != NULL) {//se gli ho passato un array
        flag = 1; //allora lo uso per l'output
    }
    write(socket, &command, sizeof(comm_t));//chiedo al server di mostrarmi gli appelli
    read(socket, &response, sizeof(comm_t));//leggo se l'apertura del file nel server è andata a buon fine
    if (response == FILE_ERR) {//se c'è stato un errore nell'aprire il file
        printf("Impossibile accedere agli appelli\n");//stampa errore
        return 0;//ed esci
    }
    //altrimenti
    sleep(1);//aspetto che calcoli n
    read(socket, &n, sizeof(int));//leggo il numero di appelli che sto per ricevere
    printf("%-5s%-31s%-21s%-31s%-10s%-30s\n","ID","Dipartimento","CDL","Nome Esame","Stato","Data e Ora");
    for (i = 0; i<n; i++) {//legge n appelli
        FullRead(socket, &a, sizeof(app_t));//leggo appello i-esimo
        if (flag) {//se flag è stato impostato
            appelliRicevuti[i] = a;//uso l'array per salvarmi gli appelli letti
        }
        printf("%-5ld%-31s%-21s%-31s%-10s%-30s\n",a.id,a.dipartimento,a.corsoDiLaurea,a.nomeEsame,a.stato?"Aperto":"Chiuso",ctime(&a.data));
    }
    return i;//ritorno il numero di appelli ricevuti
}

int registraEsame(int fd){
    comm_t command = SHOW,response = FAIL;
    int i = 0, chose = 0, napp, id;
    app_t appelliAperti[100];
    char buffer[BUFF_SIZE];
    exam_t esamiDaRegistrare[100];
    
    printf("Seleziona id dell'appello in cui registrare esami(0 - per terminare): \n");
    napp = visualizzaAppelli(fd,appelliAperti);
    printf("$-%s: ",account.matricola);
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        id = atoi(buffer);
    }
    if (id == 0) {
        return 0;
    }else if (bsearch(&id, appelliAperti, napp, sizeof(app_t), (int(*)(const void*,const void*))compareApp_t)== NULL) {
        printf("Impossibile: l'appello selezionato è inesisente.\n");
        return 0;
    }
    do{
        printf("Esame %d:\n",i+1);
        esamiDaRegistrare[i].id = id;
        printf("Nome studente: ");
        fgets(esamiDaRegistrare[i].nome, 31, stdin);
        esamiDaRegistrare[i].nome[strlen(esamiDaRegistrare[i].nome) - 1] = '\0';
        printf("Cognome: ");
        fgets(esamiDaRegistrare[i].cognome, 31, stdin);
        esamiDaRegistrare[i].cognome[strlen(esamiDaRegistrare[i].cognome) - 1] = '\0';
        printf("Matricola: ");
        fgets(buffer, sizeof(buffer), stdin);
        sscanf(buffer, "%10s",esamiDaRegistrare[i].matricola);
        printf("Voto: ");
        fgets(buffer, sizeof(buffer), stdin);
        esamiDaRegistrare[i++].voto = atoi(buffer);
        printf("Vuoi continuare:\n1 - continua\n0 - termina\n$-%s: ",account.matricola);
        fgets(buffer, sizeof(buffer), stdin);
        chose = atoi(buffer);
    }while (chose != 0 && i < 100);
    printf("Invio dati al server...\n");
    command = REC;
    write(fd, &command, sizeof(comm_t));//chiedo la registrazione degli esami
    write(fd, &i, sizeof(int));//invio il numero di esami da regisrare
    FullWrite(fd, esamiDaRegistrare, sizeof(exam_t) * i);//gli invio l'array con gli esami da registrare
    read(fd, &response, sizeof(comm_t));//leggo l'esito fopen
    switch (response) {
        case SUCCESS:
            break;
        case FILE_ERR:
            printf("Accesso negato\n");
            return 0;
        case FAIL:
        default:
            printf("Qualcosa è andato storto\n");
            return 0;
            break;
    }
    read(fd, &response, sizeof(comm_t));//leggo esito
    switch (response) {
        case SUCCESS:
            printf("esam%c registrat%c con successo\n",(i-1)?'i':'e',(i-1)?'i':'o');
            return 1;
            break;
        case CLOSE_APP:
            printf("Impossibile registrare :appello chiuso\n");
            return 0;
            break;
        case NOT_EXIST:
            printf("Impossibile registrare: appello inesistente\n");
            return 0;
            break;
        case FAIL:
        default:
            printf("Qualcosa è andato storto\n");
            return 0;
            break;
    }
}

int chiudiAppello(int fd){
    comm_t command = SHOW, response = FAIL;
    app_t appelliAperti[100];
    int  napp;
    long chose = 0;
    char buffer[BUFF_SIZE];
    
    printf("Seleziona l'appello che vuoi chiudere:\n");
    napp = visualizzaAppelli(fd, appelliAperti);
    printf("$-%s: ",account.matricola);
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        chose = atol(buffer);
    }
    if (bsearch(&chose, appelliAperti, napp, sizeof(app_t), (int(*)(const void*,const void*))compareApp_t)== NULL) {
        printf("Impossibile: l'appello selezionato è inesisente\n");//controllo se è un appello valido
        return 0;
    }
    command = CLOSE_APP;
    write(fd, &command, sizeof(comm_t));//invio il comando per chiudere un appello
    read(fd, &response, sizeof(comm_t));//leggo responso fopen
    if (response == FILE_ERR || response == FAIL) {
        printf("Impossibile accedere apli appelli\n");
        return 0;
    }
    write(fd, &chose, sizeof(long));//invio l'id dell'appello da chiudere
    read(fd, &response, sizeof(comm_t));
    switch (response) {
        case SUCCESS:
            return 1;
            break;
        case CLOSE_APP:
            printf("Impossibile: appello gia' chiuso.\n");
            return 0;
            break;
        case NOT_EXIST:
            printf("Impossibile: appello non esistente.\n");
            return 0;
            break;
        case FAIL:
        default:
            printf("Qualcosa e' andato storto\n");
            return 0;
            break;
    }
}
