#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_ARGS 10
#define MAX_PROGS 10

int sepProg(char *progs[], char strProgs[]) {
    int progc = 0;
    char *ptr = strdup(strProgs);
    while ((progs[progc] = strsep(&ptr, "&")) != NULL && progc < MAX_PROGS) {
        if (progs[progc][0] != '\0' && strspn(progs[progc], " ") != strlen(progs[progc])) {
            progc++;
        }
    }
    return progc;
}

int sepArgs(char *argsv[], char strArgs[]) {
    int argsc = 0;
    char *ptr =  strdup(strArgs);
    while ((argsv[argsc] = strsep(&ptr, "  >")) != NULL && argsc < MAX_ARGS) {
        if (argsv[argsc][0] != '\0') {
            argsc++;
        }
    }
    return argsc;
}

void errorMsg() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void buildFullPath(char fullPath[], char path[], char prog[]) {
    strcpy(fullPath, path);
    strcat(fullPath, prog);
}

int verifyRed(char args[]) {
    int count = 0;
    int i = 0;
    while (i < strlen(args) && count <= 1) {
        if (args[i] == '>') {
            count++;
        }
        i++;
    }
    if (count > 1) {
        errorMsg();
        return 0;
    } else {
        int argsc = 0;
        char *argsv[MAX_ARGS];
        char *ptr = strdup(args);
        while ((argsv[argsc] = strsep(&ptr, " ")) != NULL && argsc < MAX_ARGS) {
            if (argsv[argsc][0] != '\0') {
                argsc++;
            }
        }
        if (strcmp(argsv[argsc-2], ">") != 0) {
            errorMsg();
            return 0;
        }
        return 1;
    }
}

int main(int argc, char *argv[]) {
    char strArgs[100];
    char *argsv[MAX_ARGS];
    char *progs[MAX_PROGS];
    char path[] = "/bin/";

    printf("Digite o programa e os argumentos:\n");
    scanf(" %[^\n]s", strArgs);
    
    int progc = sepProg(progs, strArgs);

    for (int i = 0; i < progc; i++) {
        int argsc = sepArgs(argsv, progs[i]);
        int rc = fork();
        if (rc < 0) {
            fprintf(stderr, "fork failed\n");
            exit(1);
        } else if (rc == 0) { 
            if (strchr(progs[i], '>') != NULL) {
                if (verifyRed(progs[i])) {
                    close(STDOUT_FILENO);
                    open(argsv[argsc - 1], O_CREAT|O_TRUNC|O_WRONLY, S_IRWXU);
                    char fullPath[10];
                    buildFullPath(fullPath, path, argsv[0]);
                    argsv[argsc - 1] = NULL;
                    execv(fullPath, argsv);
                } else {
                    exit(1);
                }
            } else {
                char fullPath[10];
                buildFullPath(fullPath, path, argsv[0]);
                argsv[argsc] = NULL;
                execv(fullPath, argsv);
            }
        }
    }
    for (int i = 0; i < progc; i++) {
        wait(NULL);
    }
    return 0;
}
