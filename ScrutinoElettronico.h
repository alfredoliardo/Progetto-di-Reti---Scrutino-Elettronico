//
//  ScrutinoElettronico.h
//  serverD
//


#ifndef __serverD__ScrutinoElettronico__
#define __serverD__ScrutinoElettronico__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>

#define max(x, y) ({typeof (x) x_ = (x); typeof (y) y_ = (y); \
x_ > y_ ? x_ : y_;})

#define BUFF_SIZE 1024
#define SERVERD_PORT 2345
#define SERVERS_PORT 2346

enum category_t{P,D,S};
typedef enum category_t category_t;

typedef char mat_t[11];

struct user_t{
    mat_t matricola;
    char password[14];
    category_t categoria;
};
typedef struct user_t user_t;



struct app_t{
    long id;
    char matricolaProfessore[11];
    char dipartimento[31];
    char corsoDiLaurea[21];
    char nomeEsame[31];
    time_t data;
    int stato;
};
typedef struct app_t app_t;

struct exam_t{
    long id;
    char nome[31];
    char cognome[31];
    mat_t matricola;
    int voto;
};
typedef struct exam_t exam_t;

enum commands{
    LOGIN,CONNECT,SUCCESS,FAIL,WRONG_PASS,FILE_ERR,NOT_EXIST,NEW_APP,NEW_EX,CLOSE_APP,SHOW,REC,SHOW_EX,IMS,IMD,MOD_APP,MOD_EX,CLOSE
};
typedef enum commands comm_t;



ssize_t FullWrite(int fd, const void *buf, size_t count);
ssize_t FullRead(int fd, void *buf, size_t count);
long compareApp_t (const app_t * a, const app_t * b);
int compareints (const void * a, const void * b);



#endif /* defined(__serverD__ScrutinoElettronico__) */
