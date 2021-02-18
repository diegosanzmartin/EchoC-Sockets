#include <stdio.h>
#include <stdlib.h>
#include <string.h>         //Func con cad de caracteres
#include <locale.h>         //Formato español

#include <signal.h>         //CRTL-C
#include <stdbool.h>        //Variables booleanas

#include <unistd.h>         
#include <arpa/inet.h>      //Funciones orientadas a conexión
#include <sys/time.h>       //Cronometrar tiempos

//-------------------- CTRL-C STOP --------------------
static bool keepRunning = true;
void intHandler(int x) {
    keepRunning = false;
}
//-----------------------------------------------------

//CLIENTE TCP
int main(int argc, char const *argv[]){    
    //Variables sockets
    int cliSock;
    int port = atoi(argv[2]);  
    char *servIP = argv[1];
    char dataRecv[1024];

    char * echo = "abcd";
    int echoLen = strlen(echo);

    struct sockaddr_in server;

    //Variables time
    struct timeval ini;
    struct timeval fin;
    float time;

    if(argc != 3){
        printf("\nERR: nº de argumentos no válido\n");
        exit(-1);
    }

    if(port < 1023){
        printf("\nERR: El nº de puerto debe ser mayor que 1023\n");
        exit(-1);
    }


    for(;;) {
        cliSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(cliSock < 0) {
            printf("\nERR: No se pudo crear el socket\n");
            exit(-1);
        }

        memset(&servAddr, 0, sizeof(servAddr));
        servAddr.sin_family = AF_INET;

        int ctrlVal = inet_pton(AF_INET, servIP, &servAddr.sin_addr.s_addr);
        if(ctrlVal == 0) {
            printf("\nERR: Direccion no válida\n");
            exit(-1);
        }else if(ctrlVal < 0) {
            printf("\nERR: Se produjo un error en la función inet_pton()\n");
            exit(-1);            
        }
        servAddr.sin_port = htons(servPort);

        if(connect(cliSock, (struct sockaddr * ) & servAddr, sizeof(servAddr)) < 0) {
            printf("\nERR: No se pudo conectar\n");
            exit(-1);
        }

        ssize_t bytesSent = send(Sock, echo, echoLen, 0);
        if(bytesSent < 0) {
            printf("\nERR: No se pudo enviar\n");
            exit(-1);
        }else if (bytesSent != echoLen){
            printf("\nERR: No se pudo enviar\n");
            exit(-1);
        }

        unsigned int totalBytesRecv = 0;
        while (totalBytesRecv < echoLen) {
            ssize_t bytesRecv = recv(cliSock, dataRecv, 1023, 0);
            if (bytesRecv < 0) {
                printf("\nERR: No se pudo recivir datos del servidor\n");
                exit(-1);
            }else if(bytesRecv == 0) {
                printf("\nERR: La conexión se cerró bruscamente\n");
                exit(-1);
            }

            totalBytesRecv += bytesRecv;
            dataRecv[bytesRecv] = "\0";
            printf("Recivido: %s\n", dataRecv);
        }
        close(cliSock);

    }

    printf("Fin de conexión\n");

    return 0;
}