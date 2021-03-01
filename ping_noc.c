#include <stdio.h>
#include <stdlib.h>
#include <string.h>         //Func con cad de caracteres
#include <locale.h>         //Formato español

#include <signal.h>         //CRTL-C
#include <stdbool.h>        //Variables booleanas

#include <unistd.h>         
#include <netdb.h>
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

void hostNameToIP(char* hostName, char* IP) {
 	struct hostent *h;
	struct in_addr **add;
	int i;
		
    h = gethostbyname(hostName);
	add = (struct in_addr **) h->h_addr_list;
	
	for(i = 0; add[i] != NULL; i++) 
	{
		strcpy(IP , inet_ntoa(*add[i]) );
	}
}

//CLIENTE UDP
int main(int argc, char const *argv[]){    

    if(argc != 3){
        printf("\nERR: nº de argumentos no válido\n");
        exit(-1);
    }

    //Variables sockets
    int cliSock;
    int servPort = atoi(argv[2]);  
    char *host = argv[1];
    char servIP[50];

    hostNameToIP(host, servIP);

    char dataRecv[1024];

    char * echo = "abcd";
    int echoLen = strlen(echo);

    struct sockaddr_in server;
    int serverLen = sizeof(server);

    //Variables time
    struct timeval ini;
    struct timeval fin;
    float time;
    float tmax = 0.0;
    float tmin = 0.0;
    float tmed = 0.0;
    int numEnv = 0;

    //CTRL-C STOP
    struct sigaction signCTRL;
    signCTRL.sa_handler = intHandler;
    sigaction(SIGINT, &signCTRL, NULL);

    if(servPort < 1023){
        printf("\nERR: El nº de puerto debe ser mayor que 1023\n");
        exit(-1);
    }


    while (keepRunning) {
        unsigned int totalBytesRecv = 0;
        cliSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(cliSock < 0) {
            printf("\nERR: No se pudo crear el socket\n");
            exit(-1);
        }

        server.sin_family = AF_INET;
        server.sin_port = htons(servPort);
        server.sin_addr.s_addr = inet_addr(servIP);

        gettimeofday(&ini, 0);   //Iniciamos el "cronómetro"

        ssize_t bytesSent = sendto(cliSock, echo, echoLen, 0,(struct sockaddr*)&server, serverLen);
        if(bytesSent < 0) {
            printf("\nERR: No se pudo enviar\n");
            exit(-1);
        }else if (bytesSent != echoLen){
            printf("\nERR: No se pudo enviar\n");
            exit(-1);
        }
        
        ssize_t bufferRecv = recvfrom(cliSock, dataRecv, sizeof(dataRecv), 0,(struct sockaddr*)&server, &serverLen);
        gettimeofday(&fin, 0); //Apagamos el "cronómetro"

        if (bufferRecv < 0 && numEnv == 0) {
            printf("\nERR: No se pudo recibir datos del servidor\n");
            exit(-1);
        }
        else if(bufferRecv < 0 && numEnv != 0) {
            break;
        }

        totalBytesRecv += bufferRecv;
        time = (fin.tv_sec - ini.tv_sec) * 1000.0f + (fin.tv_usec - ini.tv_usec) / 1000.0f;
        timeEstad(&time, &tmin, &tmax);

        printf("-Servidor: %s/%d   bytes= %d   time= %.3f\n", servIP, servPort, strlen(dataRecv), time);
        numEnv++;
        close(cliSock);
        sleep(1);
    }

    printf("\n------Estadísticas------\n %i paquetes transmitidos\n tmax= %.3f tmin= %.3f tmed= %.3f\n", numEnv, tmax, tmin, ((tmax+tmin)/2));

    return 0;
}