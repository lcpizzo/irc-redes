#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

// Número máximo de caracteres na mensagem
<<<<<<< HEAD
#define MAX 4096
=======
#define MAX 5 
>>>>>>> cb6084459f01722a28cc030849d3a8eb069aa020
// Porta padrão para comunicação
#define PORT 8080
#define SA struct sockaddr
#define FALSE 0
#define TRUE 1

int readline(char **msg, FILE *stream) {
    int pos = 0;
    do{
<<<<<<< HEAD
        if(pos % MAX == 0){
            string = (char *)realloc(string, (pos / MAX + 1) * MAX);
        }
        string[pos] = (char) fgetc(stream);
    }while (string[pos++] != '\n' && !feof(stream));
    string[strlen(string)-1] = '\n';
    
    return string;
}

int myRead(int connfd, char *msg, int *exit){
    // loop para receber mensagens de varias partes
    for(;;){
        bzero(msg, sizeof(msg));
        read(connfd, msg, sizeof(msg));
        if(strncmp(msg, "AK", 2) == 0){
            //termina comm
            write(connfd, "AK", 2);
            return FALSE;
        }
        if(strncmp(msg, "sair", 4) == 0)
            *exit = TRUE;
        if(msg[strlen(msg)-1] !="\n") strcat(msg, "\n");
        printf("Mensagem do servidor: %s", msg);
        write(connfd, "AK", 2);
    }

    // confirma que recebeu o fim de msg
    bzero(msg, sizeof(msg));
    read(connfd, msg, sizeof(msg));
    if(strncmp(msg, "AK", 2) !=0){
        printf("erro\n");
        return TRUE;
    }
    write(connfd, "AK", 2);

    return FALSE;
}

int myWrite(int connfd, char *msg, int *exit){
    char temp[MAX];
=======
		if (pos % MAX == 0) {
			*msg = (char *)realloc(*msg, (pos / MAX + 1) * MAX);
        }
    	(*msg)[pos] = (char)fgetc(stream);
    }while((*msg)[pos++] != '\n' && !feof(stream));
    (*msg)[pos-2] = '\n';

    return pos;
}

void myWrite(int sockfd, char *msg, int tam){
    char temp[MAX + 1];
>>>>>>> cb6084459f01722a28cc030849d3a8eb069aa020

    bzero(msg, sizeof(msg));
    printf("Para o cliente: ");
    msg = readline(stdin);

    // checa se o servidor saiu
    if(strncmp(msg, "sair", 4) == 0)
        *exit = TRUE;

    int tam = strlen(msg)-1;
    // loop para dividir a msg em varias partes
    for(int i=0; i < tam/MAX; i++){
        bzero(temp, MAX + 1);
        strncpy(temp, msg, MAX);
<<<<<<< HEAD
=======
        temp[MAX] = '\0';
        write(sockfd, temp, MAX + 1);
        //printf("debug: %s\n", temp);
>>>>>>> cb6084459f01722a28cc030849d3a8eb069aa020
        msg += MAX;
        write(connfd, temp, MAX);
        bzero(temp, sizeof(temp));
        read(connfd, temp, sizeof(temp));
        if(strncmp(temp, "AK", 2) != 0){
            printf("erro\n");
            break;
        }
    }
    write(connfd, msg, tam%MAX);
    // praq?
    //msg -= (tam/MAX) * MAX;
    read(connfd, temp, sizeof(temp));
    if(strncmp(temp, "AK", 2)!= 0){
        printf("erro\n");
        return TRUE;
    }

    // envia a ultima confirmaçao indicando que acabou a mensagem
    write(connfd, "AK", 2);

    bzero(temp, sizeof(temp));
    read(connfd, temp, sizeof(temp));
    if(strncmp(temp, "AK", 2) != 0){
        printf("erro\n");
        return TRUE;
    }

    return FALSE;
}

void myChat(int sockfd){
    int exit = FALSE, erro = FALSE;
    char *msg = (char*)calloc(MAX + 1, sizeof(char));
<<<<<<< HEAD
    
    for(;;){
        // Setando o buffer da mensagem para zero
        erro = myWrite(sockfd, msg, &exit);
        if(erro)
            break;

        if(exit){
            printf("Servidor saiu...");
            break;
        }

        erro = myRead(sockfd, msg, &exit);
        if(erro)
            break;

        if(exit){
            printf("Cliente saiu...");
=======
    int n = 0;
    
    for(;;){
        // Setando o buffer da mensagem para zero
        bzero(msg, strlen(msg));
        printf("Mensagem: ");
        n = readline(&msg, stdin);

        myWrite(sockfd, msg, n);
        if((strncmp(msg, "sair", 4)) == 0){
            printf("Cliente saiu...\n");
            free(msg);
            break;
        }

        do{
            bzero(msg, strlen(msg));
            
            read(sockfd, msg, sizeof(msg));
            printf("\tDo servidor: %s\n", msg);
        }while(msg[strlen(msg)-1] != '\n');

        if((strncmp(msg, "sair", 4)) == 0){
            printf("Cliente saiu...\n");
            free(msg);
>>>>>>> cb6084459f01722a28cc030849d3a8eb069aa020
            break;
        }
    }
}
   
int main(){
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
   
    // criacao e verificacao do socket 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        printf("Criacao do socket falhou...\n");
        exit(0);
    }
    else
        printf("Socket criado com sucesso...\n");
    bzero(&servaddr, sizeof(servaddr));
   
    // define IP, PORT
    servaddr.sin_family = AF_INET;
    // endereço "127.0.0.1" é o localhost (própria máquina)
    char ip[10] = "";
    printf("Digite o IP que gostaria de conectar('' para localhost):\n");
    scanf("%[^\n]s", ip);
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
    if(strlen(ip) <= 3) strcpy(ip, "127.0.0.1");
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(PORT);
   
    // Conecta o socket do cliente ao socket do servidor
    if(connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0){
        printf("Coneccao com o servidor falhou...\n");
        exit(0);
    }
    else
        printf("Conectado com o servidor...\n");
   
    // Função de chat
    myChat(sockfd);
   
    // Fecha o socket
    close(sockfd);
}