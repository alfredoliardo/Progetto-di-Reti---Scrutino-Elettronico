//
//  funzioniClientS.c
//  clientS
//

#include "funzioniClientS.h"

int visualizzaAppelli(int socket,app_t *appelliRicevuti){
    comm_t command = SHOW,response;
    int n = 0, i,flag = 0;//numero di appelli da ricevere
    app_t a;
    
    if (appelliRicevuti != NULL) {//se gli ho passato un array
        flag = 1; //allora lo uso per l'output
    }
    write(socket, &command, sizeof(comm_t));//chiedo al server di mostrarmi gli appelli
    read(socket, &response, sizeof(comm_t));//leggo responso fopen
    if (response == FILE_ERR) {//se c'è stato un errore nell'aprire il file
        printf("Impossibile accedere agli appelli\n");//stampa errore
        return 0;//ed esci
    }
    //altrimenti
    sleep(1);
    read(socket, &n, sizeof(int));//leggo numero di appelli
    printf("%-5s%-31s%-21s%-31s%-10s%-30s\n","ID","Dipartimento","CDL","Nome Esame","Stato","Data e Ora");
    for (i = 0; i<n; i++) {//legge n appelli
        FullRead(socket, &a, sizeof(app_t));
        if (flag) {
            appelliRicevuti[i] = a;
        }
        printf("%-5ld%-31s%-21s%-31s%-10s%-30s\n",a.id,a.dipartimento,a.corsoDiLaurea,a.nomeEsame,a.stato?"Aperto":"Chiuso",ctime(&a.data));
    }
    return i;
}

int visualizzaEsami(int socket,long id,exam_t *esamiRicevuti){
    comm_t command = SHOW_EX;
    int n = 0, flag = 0, i;
    exam_t esame;
    
    if (esamiRicevuti != NULL) {//se gli ho passato un array
        flag = 1;
    }
    
    write(socket, &command, sizeof(comm_t));//invio la richiesta per visualizzare esami
    write(socket, &id, sizeof(long));//invio l'id dell'esame da aprire
    read(socket, &n, sizeof(int));//leggo quanti esami contiene l'appello
    if (n == 0) {
        printf("L'appello non contiene esami.\n");
        return 0;
    }
    printf("%-31s%-31s%-15s%-10s\n","Nome","Cognome","Matricola","Voto");
    for (i = 0; i<n; i++) {//legge n appelli
        read(socket, &esame, sizeof(exam_t));
        if (flag) {
            esamiRicevuti[i] = esame;
        }
        printf("%-31s%-31s%-15s%-10d\n",esame.nome,esame.cognome,esame.matricola,esame.voto);
    }
    
    return i;
}

int modificaDatiAppello(int socket){
    int napp, i;
    long chose;
    char buffer[BUFF_SIZE],data[11],orario[11],*token;
    app_t appelliDaVisualizzare[100],app_mod;
    comm_t command = MOD_APP;
    struct tm data_t;
    
    do {
        printf("Appelli disponibili:\n");
        napp = visualizzaAppelli(socket, appelliDaVisualizzare);
        printf("Inserisci l'id dell'appello che vuoi aprire(0 - per terminare): ");
        fgets(buffer, sizeof(buffer), stdin);
        chose = atol(buffer);
        if (chose == 0) {
            break;
        }else{
            for (i = 0; i < napp; i++) {//cerca appello
                if (appelliDaVisualizzare[i].id == chose) {//appello valido
                    app_mod = appelliDaVisualizzare[i];
                    /*REGISTRO NUOVE INFORMAZIONI*/
                    strcpy(app_mod.matricolaProfessore, appelliDaVisualizzare[i].matricolaProfessore);//salvo la matricola di chi lo ha creato
                    /*prendo le informazioni dell'appello*/
                    printf("Dipartimeto: ");
                    if(fgets(app_mod.dipartimento, sizeof(app_mod.dipartimento), stdin) != NULL){
                        app_mod.dipartimento[strlen(app_mod.dipartimento)-1] = '\0';
                    }
                    
                    printf("Nome CDL: ");
                    if(fgets(app_mod.corsoDiLaurea, sizeof(app_mod.corsoDiLaurea), stdin) != NULL){
                        app_mod.corsoDiLaurea[strlen(app_mod.corsoDiLaurea)-1] = '\0';
                    }
                    printf("Nome esame: ");
                    if(fgets(app_mod.nomeEsame, sizeof(app_mod.nomeEsame), stdin)){
                        app_mod.nomeEsame[strlen(app_mod.nomeEsame) -1] = '\0';
                    }
                    
                    printf("Data appello(gg/mm/yyyy): ");
                    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
                        sscanf(buffer, "%10s",data);
                    }
                    data_t.tm_mday = atoi((token = strtok(data, "-/")));//estrae il giorno
                    data_t.tm_mon = atoi((token = strtok(NULL, "-/")));//estrae il mese
                    data_t.tm_year = atoi((token = strtok(NULL, "-/"))) - 1900;//estrae l'anno
                    
                    printf("Orario(hh:mm): ");
                    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
                        sscanf(buffer, "%5s",orario);
                    }
                    data_t.tm_hour = atoi((token = strtok(orario, ":"))) -1;//estrae l'ora
                    data_t.tm_min = atoi((token = strtok(NULL, ":")));//estrae i minuti
                    data_t.tm_sec = 0;
                    data_t.tm_isdst = 0;
                    app_mod.data = mktime(&data_t);
                    
                    app_mod.stato = appelliDaVisualizzare[i].stato;
                    /*Fine inserimento informazioni*/
                    write(socket, &command, sizeof(comm_t));//invio comando MOD_APP
                    FullWrite(socket, &app_mod, sizeof(app_t));//invio appello da modificare
                    read(socket, &command, sizeof(comm_t));//ricevo esito fopen
                    if (command == FILE_ERR) {
                        printf("Impossibile ,accesso negato\n");
                        return 0;
                    }
                    read(socket, &command, sizeof(comm_t));//leggo esito operazione
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
                }//
            }//fine for
            //arrivati qui l'id inserito non è valido
            printf("L'id inserito non e' valido,riprova\n$-segreteriaStudenti(0 - per terminare): ");
            fgets(buffer, sizeof(buffer), stdin);
            chose = atol(buffer);
        }
    } while (chose != 0);
    
    return -1;
}


void visionaAppelliEdEsami(int socket){
    int napp;
    long chose;
    char buffer[BUFF_SIZE];
    app_t appelliDaVisualizzare[100];
    do{
        printf("Appelli disponibili:\n");
        napp = visualizzaAppelli(socket, appelliDaVisualizzare);
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
        visualizzaEsami(socket, chose, NULL);
        printf("Vuoi controllare un altro appello?\n1 - Si\n0 - Termina\n$-segreteriaDidattica: ");
        fgets(buffer, sizeof(buffer), stdin);
        chose = atol(buffer);
    }while(chose != 0);
}


int modificaDatiEsame(int fd){
    comm_t command= MOD_EX;
    app_t appelli[100];
    exam_t esami[100], esame;
    mat_t matricola;
    char buffer[BUFF_SIZE];
    int napp,nexa, i, j;
    long chose;
    
        printf("Appelli disponibili:\n");
        napp = visualizzaAppelli(fd, appelli);
        printf("Inserisci l'id dell'appello che vuoi aprire(0 - per terminare): ");
        fgets(buffer, sizeof(buffer), stdin);
        chose = atol(buffer);
        if (chose > 0) {
            for (i = 0; i < napp; i++) {//controlla se l'id e valido
                if (appelli[i].id == chose) {//id appello valido
                    nexa = visualizzaEsami(fd, chose, esami);//li salva nell'array
                    printf("Inserisci la matricola dell'esame da modificare: ");
                    fgets(buffer, sizeof(buffer), stdin);
                    sscanf(buffer, "%10s",matricola);
                    for (j = 0; j<nexa; j++) {
                        if (!strcmp(esami[j].matricola, matricola)) {//se la matricola è valida
                            /*Prendo informazioni esame*/
                            esame.id = chose;
                            printf("Nome studente: ");
                            fgets(esame.nome, 31, stdin);
                            esame.nome[strlen(esame.nome) - 1] = '\0';
                            printf("Cognome: ");
                            fgets(esame.cognome, 31, stdin);
                            esame.cognome[strlen(esame.cognome) - 1] = '\0';
                            printf("Matricola: ");
                            fgets(buffer, sizeof(buffer), stdin);
                            sscanf(buffer, "%10s",esame.matricola);
                            printf("Voto: ");
                            fgets(buffer, sizeof(buffer), stdin);
                            esame.voto = atoi(buffer);
                            
                            write(fd, &command, sizeof(comm_t));//invio comando MOD_EX
                            read(fd, &command, sizeof(comm_t));//leggo esito fopen
                            if (command == FILE_ERR) {
                                printf("Impossibile, accesso negato\n");
                                return 0;
                            }
                            FullWrite(fd, &esame, sizeof(exam_t));//invio esame modificato
                            read(fd, &command, sizeof(comm_t));//legge esito operazione
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
                        }
                    }//arrivati qui matricola non valida
                    printf("Matricola non valida\n");
                    return 0;
                }
            }//arrivati qui id non valido
            printf("Id esame non valido");
            return 0;
        }//0 o < 0
    printf("Annullato\n");
    return 0;
}















