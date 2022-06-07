#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

// Número máximo de caracteres na mensagem
#define MAX 5
// Porta padrão para comunicação
#define PORT 8080
#define SA struct sockaddr

void myWrite(int sockfd, char *msg){
    int tam = strlen(msg);
    char temp[MAX];
    printf("%x\t%s", msg, msg);
    for(int i=0; i < tam/MAX; i++){
        strncpy(temp, msg, MAX);
        write(sockfd, temp, MAX);
        printf("debug: %s\n", temp);
        msg += MAX;
    }
    write(sockfd, msg, tam%MAX);
    printf("debug: %s\n", msg);
    msg -= (tam/MAX) * MAX;
    printf("original: %s", msg);
}

void myChat(int sockfd){
    char *msg = NULL;
    msg = (char*)calloc(MAX, sizeof(char));
    
    int n = 0;
    for(;;){
        // Setando o buffer da mensagem para zero
        n = 0;
        printf("Mensagem: ");
        while((msg[n++] = getchar()) != '\n'){
            if(n%MAX == 0){
                msg = (char *)realloc(msg, (n/MAX + 1) * MAX * sizeof(char));
                printf("realocado!, %d %d\n", strlen(msg), (n/MAX + 1) * MAX);
            }
        }

        myWrite(sockfd, msg);
        if((strncmp(msg, "sair", 4)) == 0){
            printf("Cliente saiu...\n");
            break;
        }
        bzero(msg, strlen(msg));

        read(sockfd, msg, strlen(msg));
        printf("mensagem = %s\n", msg);
        printf("Do servidor: %s", msg);
        if((strncmp(msg, "sair", 4)) == 0){
            printf("Cliente saiu...\n");
            break;
        }
        bzero(msg, strlen(msg));
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
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;
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