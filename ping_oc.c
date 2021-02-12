#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>


//CLIENTE TCP
int main(int argc, char const *argv[]){
    int sock;
    int port = atoi(argv[2]);  
    char *servIP = argv[1];
    char recv[1024];

    struct sockaddr_in server;

    if(argc != 3){
        printf("ERR: nº de argumentos no válido\n");
        exit(-1);
    }

    if(port < 1023){
        printf("ERR: El nº de puerto debe ser mayor que 1023\n");
        exit(-1);
    }
      
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock < 0) {
        printf("ERR: No se pudo crear el socket\n");
        exit(-1);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(servIP);
    server.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("ERR: No se pudo conectar\n");
        exit(-1);
    }

    if (write(sock, "Prueba de conexión\n", strlen("Prueba de conexión\n")) < 0) {
        printf("ERR: No se pudo escribir\n");
        exit(-1);
    }

    memset(recv, 0, sizeof(recv));

    if (read(sock, recv, sizeof(recv)) < 0) {
        printf("ERR: No se pudo leer\n");
        exit(-1);
    }

    printf("Recived: %s\n", recv);


    printf("Fin de conexión");
    close(sock);

    return 0;
}