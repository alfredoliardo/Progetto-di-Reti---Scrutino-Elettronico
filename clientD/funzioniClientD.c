//
//  funzioniClientD.c
//  clientD
//


#include "funzioniClientD.h"

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