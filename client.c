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

char *readline(FILE *stream) {
    char *string = (char *)calloc(MAX + 1, sizeof(char));
    int pos = 0;
    
    do{
		if (pos % MAX == 0) {
			string = (char *)realloc(string, (pos / MAX + 1) * MAX);
        }
    	string[pos] = (char)fgetc(stream);
    }while(string[pos++] != '\n' && !feof(stream));
    string[strlen(string)-1] = '\n';
    
    return string;
}

void myWrite(int sockfd, char *msg){
    int tam = strlen(msg);
    char temp[MAX];

    for(int i=0; i < tam/MAX; i++){
        bzero(temp, MAX);
        strncpy(temp, msg, MAX);
        write(sockfd, temp, MAX);
        //printf("debug: %s\n", temp);
        msg += MAX;
    }
    write(sockfd, msg, tam%MAX);
    //printf("debug: %s\n", msg);
    msg -= (tam/MAX) * MAX;
    //printf("original: %s", msg);
}

void myChat(int sockfd){
    char *msg = (char*)calloc(MAX + 1, sizeof(char));
    
    int n = 0;
    for(;;){
        // Setando o buffer da mensagem para zero
        bzero(msg, strlen(msg));
        n = 0;
        printf("Mensagem: ");
        msg = readline(stdin);

        myWrite(sockfd, msg);
        if((strncmp(msg, "sair", 4)) == 0){
            printf("Cliente saiu...\n");
            break;
        }

        do{
            bzero(msg, strlen(msg));
            
            read(sockfd, msg, sizeof(msg));
            printf("Do servidor: %s\n", msg);
            if((strncmp(msg, "sair", 4)) == 0){
                printf("Cliente saiu...\n");
                break;
            }

        }while(msg[strlen(msg)-1] != '\n');
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