#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

void errorMsg() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

int sepArgs(char *argumentos[], char str[]) {
    char *ptr = strdup(str);
    int cont = 0;
    while ((argumentos[cont] = strsep(&ptr, " \t>")) != NULL) {
        if (argumentos[cont][0] != '\0') {
            cont++;
        }
    }
    return cont;
}

void sepRed(char *tokens[], char str[]) {
    char *ptr = strdup(str);
    int i = 0;
    while ((tokens[i] = strsep(&ptr, ">")) != NULL) {
        i++;
    }
}

int verificarRed(char str[]) {
    int cont = 0;
    int i = 0;
    while (i < strlen(str) && cont <= 1) {
        if (str[i] == '>') {
            cont++;
        }
        i++;
    }
    if (cont > 1) {
        return 0;
    } else {
        char *argumentos[100];
        char *partes[2];
        sepRed(partes, str);
        int contArgs = sepArgs(argumentos, partes[1]);
        if (contArgs > 1) {
            return 0;
        }
    }
    return 1;
}

void redirecionar(char *argumentos[], int tamanho) {
    close(STDOUT_FILENO);
    if (open(argumentos[tamanho-1], O_CREAT|O_TRUNC|O_WRONLY, S_IRWXU) == -1) {
        errorMsg();
        exit(1);
    }
    argumentos[tamanho-1] = NULL;
}

int main(int argc, char *argv[]) {
    if (argc == 1){
        printf("wish> ");
        char *buffer = NULL;
        size_t tambuffer = 0;
        ssize_t lidos = getline(&buffer, &tambuffer, stdin);
        buffer[lidos-1] = '\0';
        char *path2 = "/bin/";

        while (strcmp(buffer, "exit") != 0) {
            char *argumentos[100];
            int cont = sepArgs(argumentos, buffer);
            argumentos[cont] = NULL;
            pid_t pid = fork();  //cria um "clone" do processo que chamou
            if (pid == 0) {  //processo filho
                if (strchr(buffer, '>') != NULL) {
                    if (verificarRed(buffer)) {
                        redirecionar(argumentos, cont);
                    } else {
                        errorMsg();
                        exit(1);
                    }
                }
                char aux[100];
                strcpy(aux, path2);
                strcat(aux, argumentos[0]);
                if (execv(aux, argumentos) == -1) {
                    fprintf(stderr, "Execv failed\n");
                    exit(1);
                }
            }
            else if (pid > 0) { //processo pai==shell
                wait(NULL);
                printf("wish> ");
                tambuffer = 0;
                buffer = NULL;
                lidos = getline(&buffer, &tambuffer, stdin);
                buffer[lidos-1] = '\0';
            }
            else { //erro do fork
                fprintf(stderr, "fork failed\n");
                exit(1);
            }

        }
        exit(0);



    }
    else if(argc==2){
        FILE *arquivo= fopen(argv[1], "r");
        char * buffer2;
        buffer2=NULL;
        size_t tambuffer2=0;
        ssize_t lidos= getline(&buffer2, &tambuffer2, arquivo);
        buffer2[lidos-1]= '\0';
        while (lidos!= EOF){

           char * token= strsep(&buffer2, " ");
            char *argumentos[100];
            int cont=0;
            while (token != NULL){
                argumentos[cont]= token;
                cont++;
                token= strsep(&buffer2, " ");
            }
            argumentos[cont]=NULL;
            pid_t pid=fork();  //cria um "clone" do processo que chamou
            if(pid==0){  //processo filho
                char aux[100];
                strcpy(aux, "/bin/");
                strcat(aux, argumentos[0]);
                execv(aux, argumentos);
            }
            else if(pid>0){ //processo pai==shell
                wait(NULL);
                tambuffer2=0;
                buffer2=NULL;
                lidos =getline(&buffer2, &tambuffer2, arquivo);
                buffer2[lidos-1]= '\0';


            }



        }
        exit(0);


    }
    else{
        //error
    }




    return 0;
}
