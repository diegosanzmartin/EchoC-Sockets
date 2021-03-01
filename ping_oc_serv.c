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
void timeEstad(float *t, float *tmin, float *tmax) {
    if(*tmax == 0 && *tmin == 0) {
        *tmax = *tmin = *t;
    }
    else if(*t > *tmax) {
        *tmax = *t;
    }
    else if(*t < *tmin) {
        *tmin = *t;
    }
}

//SERVIDOR TCP
int main(int argc, char const *argv[]){
    //Variables sockets
    int servSock, cliSock, len;
    int servPort = atoi(argv[1]);
    char cliName[INET_ADDRSTRLEN];
    char dataRecv[1024];
    ssize_t bytesRecv;

    struct sockaddr_in servAddr, cliAdrr;

    //Variables time
    struct timeval ini;
    struct timeval fin;
    float time;
    float tmax = 0.0;
    float tmin = 0.0;
    int numEnv = 0;

    //CTRL-C STOP
    struct sigaction signCTRL;
    signCTRL.sa_handler = intHandler;
    sigaction(SIGINT, &signCTRL, NULL);

    if(argc != 2){
        printf("\nERR: nº de argumentos no válido\n");
        exit(-1);
    }

    if(servPort < 1023){
        printf("\nERR: El nº de puerto debe ser mayor que 1023\n");
        exit(-1);
    }

    servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(servSock < 0) {
        printf("\nERR: No se pudo crear el socket\n");
        exit(-1);
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(servPort);

    if (bind(servSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        printf("\nERR: No se pudo conectar\n");
        exit(-1);
    }

    if (listen(servSock, 10) < 0) {                           //nº máx de solicitudes de conexión pendientes = 10
        printf("\nERR: No se pudo escuchar\n");
        exit(-1);
    }

    while (keepRunning) {   //Búcle infinito
        len = sizeof(cliAdrr);
        cliSock = accept(servSock, (struct sockaddr *)&cliAdrr, &len);
        
        if (cliSock < 0 && numEnv == 0) {
            printf("\nERR: No se pudo aceptar\n");
            exit(-1);
        }else if(cliSock < 0) {
            break;
        }
        gettimeofday(&ini, 0);   //Iniciamos el "cronómetro"
        
        if(inet_ntop(AF_INET, &cliAdrr.sin_addr.s_addr, cliName, sizeof(cliName)) != NULL) {
            printf("-Cliente: %s/%d   ", cliName, ntohs(cliAdrr.sin_port));
        }
        else{
            printf("\nERR: No se pudo obtener la direccion del cliente\n");
            exit(-1);
        }
        
        bytesRecv = recv(cliSock, dataRecv, 1024, 0);

        if(bytesRecv < 0) {
            printf("\nERR: No se pudo recibir datos del cliente\n");
            exit(-1);
        }
        else{
            printf("bytes= %d   ", bytesRecv);
        }

        while (bytesRecv > 0) {
            ssize_t bytesSent = send(cliSock, dataRecv, bytesRecv, 0);
            if (bytesSent < 0) {
                printf("\nERR: No se pudo enviar");
                exit(-1);
            }
            else if(bytesSent != bytesRecv) {
                printf("\nERR: Nº de bytes inesperados");
                exit(-1);
            }

            bytesRecv = recv(cliSock, dataRecv, 1024, 0);
            if (bytesRecv < 0) {
                printf("\nERR: No se pudo recibir datos del cliente\n");
                exit(-1);
            }

        }

        gettimeofday(&fin, 0); //Apagamos el "cronómetro"
        time = (fin.tv_sec - ini.tv_sec) * 1000.0f + (fin.tv_usec - ini.tv_usec) / 1000.0f;
        timeEstad(&time, &tmin, &tmax);

        printf("time= %.3f\n", time);
        numEnv++;
    }

    printf("\n------Estadísticas------\n %i paquetes transmitidos\n tmax= %.3f tmin= %.3f tmed= %.3f\n", numEnv, tmax, tmin, ((tmax+tmin)/2));

    close(servSock);

    return 0;
}