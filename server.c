#include "common.h"

#include <pthread.h>

#define FIFO1 "/tmp/fifo.1"
#define FIFO2 "/tmp/fifo.2"
#define PERMS 0666


fila *filas = NULL;

folder *folders = NULL;



int *connecteds;
int totConnecteds = 0;


int *pidwork;
int *whatwork;
int *firstjob;
char **firstjobstring;
int totPidwork = 0;



int readfd, writefd;



void createFolder(folder *folder1)  {
    mkdir(folder1->name, 0777);
    if (folders != NULL) {
        folder *head = folders;
        while (folders->nseg != NULL) {
            folders = folders->nseg;
        }
        folders->nseg = folder1;
        folders = head;
        return;
    }
    folders = folder1;
}

int havePassword(char* name) {
    int res = -1;
    if (folders != NULL) {
        folder *head = folders;
        while (folders != NULL) {
            if (strcmp(folders->name, name) == 0) {
                if (folders->pass == 1) {
                    res = 1;
                } else {
                    res = 0;
                }
                break;
            }
            folders = folders->nseg;
        }
        folders = head;
    }
    return res;
}

char* cdFolder(char* name, char* password) {
    char *resultString = malloc(MAXBUFF* sizeof(char));
    strcpy(resultString, "Pasta não encontrada.\n");

    if (folders != NULL) {
        folder* head = folders;

        while (folders != NULL) {
            if (strcmp(folders->name, name) == 0) {
                if (folders->pass == 1) {
                    if (strcmp(folders->password, password) == 0) {
                        strcpy(resultString, "Acesso concedido.\n");
                    } else {
                        strcpy(resultString, "Password errada.\n");
                    }
                } else {
                    strcpy(resultString, "Acesso concedido.\n");
                }
                break;
            }
            folders = folders->nseg;
        }

        folders = head;
        return resultString;
    }

    strcpy(resultString, "Nenhuma pasta encontrada.\n");
    return resultString;
}


char* concatFila(int idFila, node *node1) {

    char *result = (char*) malloc(MAXBUFF* sizeof(char));

    char *temp = (char*) malloc(MAXBUFF* sizeof(char));
    sprintf(temp, "%d", idFila);

    strcpy(result, "Fila com (id=");
    strcat(result, temp);
    strcat(result, ") -> ");

    while (node1 != NULL) {

        temp = (char*) malloc(MAXBUFF* sizeof(char));
        sprintf(temp, "%d", node1->msgId);
        strcat(result, temp);
        strcat(result, ",'");
        temp = (char*) malloc(MAXBUFF* sizeof(char));
        sprintf(temp, node1->content);
        strcat(result, temp);

        if (node1->nseg != NULL)
            strcat(result, "' -> ");
        else
            strcat(result, "'\n");

        node1 = node1->nseg;
    }

    return result;

}
char* showFila (int idFila) {

    char *temp = (char*) malloc(MAXBUFF* sizeof(char));
    sprintf(temp, "%d", idFila);

    char *result = malloc(MAXBUFF* sizeof(char));

    fila *head = filas;
    while(filas != NULL) {

        if (idFila == filas->id) {

            if (filas->nodes != NULL) {
                strcpy(result, concatFila(idFila, filas->nodes));
                break;
            } else {

                strcpy(result, "A fila com o id ");
                strcat(result, temp);
                strcat(result, " está vazia.\n");
                break;
            }

        }

        filas = filas->nseg;
    }

    if (filas == NULL) {
        strcpy(result, "Não existe uma fila com o id ");
        strcat(result, temp);
        strcat(result, ".\n");
    }

    filas = head;
    return result;
}

char* getMessage(int idFila, int idMsg) {

    char *result = malloc(MAXBUFF* sizeof(char));

    fila *head = filas;

    while(filas != NULL) {
        if (filas->id == idFila) {
            node *node1 = filas->nodes;
            while(node1 != NULL) {
                if (node1->msgId == idMsg) {
                    strcpy(result, node1->content);
                }
                node1 = node1->nseg;
            }
        }
        filas = filas->nseg;
    }

    filas = head;
    return result;

}

void insertMessage(int idFila, char *message) {

    fila *head = filas;
    while(filas != NULL) {

        if (filas->id == idFila) {

            node *newNode = (node*) malloc(sizeof(node));
            newNode->nseg = NULL;
            newNode->nant = NULL;
            newNode->content = (char*) malloc(MAXBUFF* sizeof(char));
            strcpy(newNode->content, message);

            if (filas->nodes != NULL) {

                node *nhead = filas->nodes;
                node *node1 = filas->nodes;
                while (node1->nseg != NULL)
                    node1 = node1->nseg;
                newNode->msgId = (node1->msgId + 1);
                newNode->nant = node1;
                node1->nseg = newNode;

                filas->nodes = nhead;

            } else {

                newNode->msgId = 0;
                filas->nodes = newNode;

            }

        }

        filas = filas->nseg;
    }

    filas = head;

}

void removeMsg(int idFila, int idMsg) {

    fila *head = filas;
    while(filas != NULL) {

        if (filas->id == idFila) {

            node *nhead = filas->nodes;
            node *node1 = filas->nodes;

            if (nhead->nseg == NULL && nhead->msgId == idMsg) {
                filas->nodes = NULL;
            } else {
                while (node1 != NULL) {

                    if (node1->msgId == idMsg) {

                        if (node1->nant != NULL)
                            node1->nant->nseg = node1->nseg;
                        if (node1->nseg != NULL)
                            node1->nseg->nant = node1->nant;

                        if (node1->nant == NULL)
                            nhead = node1->nseg;

                    }
                    node1 = node1->nseg;
                }
                filas->nodes = nhead;
            }
        }
        filas = filas->nseg;
    }
    filas = head;

}

void insertInWork(int client, int work) {
    if (totPidwork == 0) {
        pidwork = (int*) malloc(sizeof(int));
        whatwork = (int*) malloc(sizeof(int));
        firstjob = (int*) malloc(sizeof(int));
        firstjobstring = (char**) malloc(sizeof(char*));
        totPidwork++;
    } else {
        pidwork = (int*) realloc(pidwork, ++totPidwork* sizeof(int));
        whatwork = (int*) realloc(whatwork, totPidwork* sizeof(int));
        firstjob = (int*) realloc(firstjob, totPidwork* sizeof(int));
        firstjobstring = (char**) realloc(firstjobstring, totPidwork* sizeof(char*));
    }
    pidwork[totPidwork-1] = client;
    whatwork[totPidwork-1] = work;
    firstjob[totPidwork-1] = -1;
    firstjobstring[totPidwork-1] = "";
}

int isInWork(int client) {
    for(int i = 0; i < totPidwork; i++) {
        if (pidwork[i] == client) {
            return whatwork[i];
        }
    }
    return -1;
}

int getId(int client) {
    for (int i = 0; i < totPidwork; i++) {
        if (pidwork[i] == client) {
            return i;
        }
    }
    return -1;
}

void removeFromWork(int client) {
    int id = getId(client);
    for (int i = id; i < totPidwork; i++) {
        pidwork[i] = pidwork[i+1];
        whatwork[i] = whatwork[i+1];
        firstjob[i] = firstjob[i+1];
        firstjobstring[i] = firstjobstring[i+1];
    }
    pidwork = (int*) realloc(pidwork, --totPidwork* sizeof(int));
    whatwork = (int*) realloc(whatwork, totPidwork* sizeof(int));
    firstjob = (int*) realloc(firstjob, totPidwork* sizeof(int));
    firstjobstring = (char**) realloc(firstjobstring, totPidwork* sizeof(char*));
}

void saveFirstJob(int client, int job) {
    int id = getId(client);
    firstjob[id] = job;
}

void saveFirstJobText(int client, char* job) {
    int id = getId(client);
    char *job1 = (char*) malloc(sizeof(char));
    strcpy(job1, job);
    firstjobstring[id] = job1;
}

int getFirstJob(int client) {
    int id = getId(client);
    return firstjob[id];
}

char* getFirstJobString(int client) {
    int id = getId(client);
    return firstjobstring[id];
}


void* getResult(void *args) {

    message *input = (message*) args;

    message *result = (message*) malloc(sizeof(message));
    result->client = input->client;

    char *resultText = (char*) malloc(MAXBUFF*sizeof(char));

    strcpy(resultText, "Comando não reconhecido.\n");


    if (isInWork(input->client) != -1) {
        int work = isInWork(input->client);
        int job = getFirstJob(input->client);

        if (work == 3) {
            strcpy(resultText, showFila(atoi(input->text)));
            removeFromWork(input->client);
        }

        if (work == 4 && job == -1) {

            saveFirstJob(input->client, atoi(input->text));
            strcpy(resultText, "Digite o numero da mensagem: ");

        }

        if (work == 4 && job != -1) {

            char *res = malloc(MAXBUFF* sizeof(char));
            strcpy(res, "A mensagem é: ");
            strcat(res, getMessage(getFirstJob(input->client), atoi(input->text)));
            strcat(res, ".");
            strcpy(resultText, res);
            removeFromWork(input->client);

        }

        if (work == 5 && job == -1) {

            saveFirstJob(input->client, atoi(input->text));
            strcpy(resultText, "Digite a mensagem: ");

        }

        if (work == 5 && job != -1) {

            insertMessage(getFirstJob(input->client), input->text);
            strcpy(resultText, "A mensagem foi inserida com sucesso.\n");
            removeFromWork(input->client);

        }

        if (work == 6 && job == -1) {

            saveFirstJob(input->client, atoi(input->text));
            strcpy(resultText, "Digite o numero da mensagem: ");

        }

        if (work == 6 && job != -1) {

            removeMsg(getFirstJob(input->client), atoi(input->text));
            strcpy(resultText, "Mensagem removida com sucesso.\n");
            removeFromWork(input->client);

        }

        if (work == 7) {

            folder *folder1 = (folder*) malloc(sizeof(folder));
            folder1->name = (char*) malloc(MAXBUFF* sizeof(char));
            strcpy(folder1->name, input->text);
            folder1->pass = 0;
            createFolder(folder1);
            strcpy(resultText, "Pasta criada com sucesso.\n");
            removeFromWork(input->client);

        }

        if (work == 8 && job == -1) {

            saveFirstJobText(input->client, input->text);
            saveFirstJob(input->client, 1);
            strcpy(resultText, "Digite a password: ");

        }

        if (work == 8 && job != -1) {

            folder *folder1 = (folder*) malloc(sizeof(folder));
            folder1->name = (char*) malloc(MAXBUFF* sizeof(char));
            strcpy(folder1->name, getFirstJobString(input->client));
            folder1->pass = 1;
            folder1->password = (char*) malloc(MAXBUFF* sizeof(char));
            strcpy(folder1->password, input->text);
            createFolder(folder1);
            strcpy(resultText, "Pasta criada com sucesso.\n");
            removeFromWork(input->client);

        }

        if (work == 9 && job == -1) {

            int fd = open(input->text, O_RDONLY);
            int ch;

            if(fd < 0) {
                strcpy(resultText, "Erro ao abrir ficheiro.\n");
            } else {

                char *res = (char*) malloc(MAXBUFF* sizeof(char));
                int count = 0;
                while (read(fd, &ch, 1)) {
                    strcat(res, &ch);
                    count++;
                    if (count >= (MAXBUFF-2))
                        break;
                }

                //res[MAXBUFF-1] = '\0';

                strcpy(resultText, res);

            }

            close(fd);
            removeFromWork(input->client);

        }

        if (work == 10 && job == -1) {

            int hp = havePassword(input->text);
            if (hp == -1) {
                strcpy(resultText, "Pasta não encontrada.\n");
                removeFromWork(input->client);
            } else if (hp == 0) {
                strcpy(resultText, "Acesso concedido.\n");
                removeFromWork(input->client);
            } else if (hp == 1) {
                strcpy(resultText, "Digite a password: ");
                saveFirstJobText(input->client, input->text);
                saveFirstJob(input->client, 1);
            }

        }

        if (work == 10 && job != -1) {

            strcpy(resultText, cdFolder(getFirstJobString(input->client), input->text));
            removeFromWork(input->client);

        }

        result->text = resultText;

        pthread_exit(result);
    }


    if (strcmp(input->text, "started") == 0 || strcmp(input->text, "menu") == 0) {

        strcpy(resultText,
        "\n\n---- Menu ----\n 1- Visualizar listas\n 2- Criar uma lista\n 3- Visulizar mensagens de uma lista\n 4- Visualizar uma mensagem\n 5- Inserir uma mensagem\n 6- Apagar uma mensagem\n ---- PASTAS ---- \n 7- Criar pasta\n 8- Criar pasta secreta\n 9- Ler ficheiro\n 10- Acesso a pasta\n-----------\n\n"
        );

    } else if (strcmp(input->text, "1") == 0) {

        if (filas == NULL) {

            strcpy(resultText, "Não existem listas criadas.\n");

        } else {
            fila *head = filas;
            char *res = (char*) malloc(MAXBUFF*sizeof(char));
            strcpy(res, "\nFilas existentes: \n");
            while (filas != NULL) {
                char *temp = (char*) malloc(sizeof(char));
                sprintf(temp, "%d", filas->id);
                strcat(res, temp);
                strcat(res, ";\t");
                filas=filas->nseg;
            }
            strcat(res, "\n");
            filas = head;
            strcpy(resultText, res);

        }

    } else if (strcmp(input->text, "2") == 0) {
        //insert last

        fila *fila1 = (fila*) malloc (sizeof(fila));

        if (filas == NULL) {
            fila1->id = 0;
            filas = fila1;
        } else {

            fila *head = filas;

            while( filas->nseg != NULL ) {
                filas = filas->nseg;
            }

            fila1->id = (filas->id + 1);
            filas->nseg = fila1;
            filas=head;

        }

        strcpy(resultText, "Lista criada com sucesso.\n");

    } else if (strcmp(input->text, "3") == 0) {

        strcpy(resultText, "Digite o numero da lista: ");
        insertInWork(input->client, 3);

    } else if (strcmp(input->text, "4") == 0) {

        strcpy(resultText, "Digite o numero da lista: ");
        insertInWork(input->client, 4);

    } else if (strcmp(input->text, "5") == 0) {

        strcpy(resultText, "Digite o numero da lista: ");
        insertInWork(input->client, 5);

    } else if (strcmp(input->text, "6") == 0) {

        strcpy(resultText, "Digite o numero da lista: ");
        insertInWork(input->client, 6);

    } else if (strcmp(input->text, "7") == 0) {

        strcpy(resultText, "Digite o nome da pasta: ");
        insertInWork(input->client, 7);

    } else if (strcmp(input->text, "8") == 0) {

        strcpy(resultText, "Digite o nome da pasta: ");
        insertInWork(input->client, 8);

    } else if (strcmp(input->text, "9") == 0) {

        strcpy(resultText, "Digite o nome do ficheiro: ");
        insertInWork(input->client, 9);

    } else if (strcmp(input->text, "10") == 0) {
        //acesso a pasta
        strcpy(resultText, "Digite o nome da pasta: ");
        insertInWork(input->client, 10);

    }

    result->text = resultText;

    pthread_exit(result);

}

void saveClient(int pid) {
    if (totConnecteds == 0) {
        connecteds = (int*) malloc(sizeof(int));
        connecteds[0] = pid;
        totConnecteds++;
        return;
    }

    connecteds = (int*) realloc(connecteds, ++totConnecteds* sizeof(int));
    connecteds[totConnecteds-1] = pid;
}

int sendMessage(message *msg) {

    char *isyou = (char*) malloc(MAXBUFF*sizeof(char));
    strcpy(isyou, "isyou");
    char *buffer = (char*) malloc(10*sizeof(char));

    for (int i = 0; i < totConnecteds; i++) {

        int cpid = msg->client;

        write(writefd, isyou, MAXBUFF);
        write(writefd, &cpid, sizeof(int));

        read(readfd, buffer, 11);

        fflush(stdout);

        if (strcmp(buffer, "yes") == 0) {

            buffer = (char*) malloc(MAXBUFF* sizeof(char));
            strcpy(buffer, msg->text);
            write(writefd, buffer, strlen(buffer)+1);
            break;

        }

    }

}

int main() {




    char *buffer = (char *) malloc(MAXBUFF * sizeof(char));

    mknod(FIFO1, S_IFIFO | PERMS, 0);
    mknod(FIFO2, S_IFIFO | PERMS, 0);


    while (1) {

        readfd = open(FIFO1, 0);
        writefd = open(FIFO2, 1);

        int pid;

        read(readfd, &pid, sizeof(int));

        printf("pid: %d\n", pid);
        fflush(stdout);

        read(readfd, buffer, MAXBUFF);

        printf("input: %s\n", buffer);
        fflush(stdout);

        if (strcmp(buffer, "exitserver\0") == 0)
            break;

        saveClient(pid);

        message *msg = (message *) malloc(sizeof(message));
        msg->client = pid;
        msg->text = buffer;

        message *result;

        pthread_t th;
        pthread_create(&th, NULL, getResult, msg);
        pthread_join(th, &result);


        printf("output: %s\n\n", result->text);
        fflush(stdout);

        sendMessage(result);

    }


}