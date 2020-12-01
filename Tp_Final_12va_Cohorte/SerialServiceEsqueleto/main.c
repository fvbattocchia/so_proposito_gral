#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "SerialManager.h"
#include "tcpServer.h"

#define SERIAL_PORT 1 //(0 -> dev/ttyusb0, 1-> portusb1)
#define BAUDRATE 115200

int newfd;
int status_thread;
void bloquearSign(void)
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    if(pthread_sigmask(SIG_BLOCK, &set, NULL)== -1) {
		perror("sigmask");
		exit(1);
	}
}

void desbloquearSign(void)
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    if(pthread_sigmask(SIG_UNBLOCK, &set, NULL)== -1) {
		perror("sigmask");
		exit(1);
	}

}

void recibiSignal(int sig)
{
		status_thread=STOP;
}

int main(void)
{
	pthread_t tcpServer;
	int ret;
	char buffer[TAMANO];
	newfd=ERROR;

	struct sigaction sa;
	//configuro handler signals
	sa.sa_handler = recibiSignal;
	sa.sa_flags = 0; //SA_RESTART;
	sigemptyset(&sa.sa_mask);

	if(sigaction(SIGINT,&sa,NULL)== -1) {
		perror("sigaction");
		exit(1);
	}

	if(sigaction(SIGTERM,&sa,NULL)== -1) {
		perror("sigaction");
		exit(1);
	}

	//enmascaro señales
	bloquearSign();
	//variable de control de los thread
	status_thread = START;

	//Se abre el puerto serie

	printf("Inicio Serial Service\r\n");
	ret=serial_open(SERIAL_PORT,BAUDRATE);
	if(ret!=0){
		perror("Error al abrir el puerto serie");
		exit(1);
	}

	//se crea el threar del servidor
  pthread_create (&tcpServer, NULL, server_thread,NULL);
	//------
	//desenmascaro señales
	desbloquearSign();

	while(status_thread){
		ret=serial_receive(buffer,TAMANO);//buffer to send socket
    if(ret != EOF_){
			 buffer[ret]=0x00;
		   printf("Recibi de la EDU-CIAA %d bytes.:%s\n",ret,buffer);
		   //enviar x socket al cliente
			 if(newfd>ERROR)
			 {
				 printf("Enviando mjs al cliente\n");
				 if (write (newfd,buffer,TAMANO) == ERROR)
				 {
						 perror("Error escribiendo mensaje en socket");
						 exit (1);
				 }
		   }
			 else{
			 	printf("No hay conexion con el cliente\r\n");
		   }
		}
		sleep(1);
	 }
	 printf  ("Finalizando programa\n");
	 pthread_cancel(tcpServer);
	//------
	//se espera a que finalice el thread del server
  pthread_join (tcpServer, NULL);
	if(newfd!=ERROR){
		 close(newfd);
	}
	return 0;
}
