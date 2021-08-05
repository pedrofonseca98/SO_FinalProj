#include <stdio.h> 
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define MAXBUFF 1024

typedef struct Node {
    int msgId;
    void *content;
    struct Node *nseg;
    struct Node *nant;
} node;

typedef struct Fila {
    int id;
    node *nodes;
   struct Fila *nseg;
} fila;

typedef struct Message {
    int client;
    char *text;
} message;

typedef struct Folder {
    char *name;
    int pass;
    char *password;
    struct Folder *nseg;
} folder;