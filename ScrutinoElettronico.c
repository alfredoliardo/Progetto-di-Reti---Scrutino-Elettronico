//
//  ScrutinoElettronico.c
//  serverD
//

#include "ScrutinoElettronico.h"

ssize_t FullWrite(int fd, const void *buf, size_t count)
{
    size_t nleft;
    ssize_t nwritten;
    
    nleft = count;
    while (nleft > 0) {             /* repeat until no left */
        if ( (nwritten = write(fd, buf, nleft)) < 0) {
            if (errno == EINTR) {   /* if interrupted by system call */
                continue;           /* repeat the loop */
            } else {
                return(nwritten);   /* otherwise exit with error */
            }
        }
        nleft -= nwritten;          /* set left to write */
        buf +=nwritten;             /* set pointer */
    }
    return (nleft);
}



ssize_t FullRead(int fd, void *buf, size_t count)
{
    size_t nleft;
    ssize_t nread;
    
    nleft = count;
    while (nleft > 0) {             /* repeat until no left */
        if ( (nread = read(fd, buf, nleft)) < 0) {
            if (errno == EINTR) {   /* if interrupted by system call */
                continue;           /* repeat the loop */
            } else {
                return(nread);      /* otherwise exit */
            }
        } else if (nread == 0) {    /* EOF */
            break;                  /* break loop here */
        }
        nleft -= nread;             /* set left to read */
        buf +=nread;                /* set pointer */
    }
    return (nleft);
}

long compareApp_t (const app_t * a, const app_t * b)
{
    return ( a->id - b->id);
}

int compareints (const void * a, const void * b)
{
    return ( *(int*)a - *(int*)b );
}


