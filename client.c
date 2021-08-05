#include "common.h"

#include <unistd.h>

#define FIFO1 "/tmp/fifo.1"
#define FIFO2 "/tmp/fifo.2"

int main (){

    char *buffer = (char*) malloc(MAXBUFF*sizeof(char));

    int readfd , writefd;

    writefd = open(FIFO1, 1);
    readfd = open(FIFO2, 0);

    int pid = getppid();

    write(writefd, &pid, sizeof(pid));
    strcpy(buffer, "started");
    write(writefd, buffer, strlen(buffer)+1);

    int first = 0;

    while(1) {

        if (first) {

            buffer = (char *) malloc(MAXBUFF * sizeof(char));
            scanf("%s", buffer);

            fflush(stdin);

            if (strcmp(buffer, "exit") == 0) {
                break;
            }

            write(writefd, &pid, sizeof(pid));
            write(writefd, buffer, strlen(buffer) + 1);

        }

        read(readfd, buffer, MAXBUFF);

        if (strcmp(buffer, "isyou") == 0) {

            int pid2;
            read(readfd, &pid2, sizeof(int));

            char *yn = (char*) malloc(10*sizeof(char));
            if (pid == pid2) {
                strcpy(yn, "yes");
                write(writefd, yn, 11);

                read(readfd, buffer, MAXBUFF);
            } else {
                strcpy(yn, "no");
                write(writefd, yn, 11);
            }

        }
        first = 1;

        printf("%s\n", buffer);
        fflush(stdout);

    }

}
