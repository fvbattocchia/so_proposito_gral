#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "tcpServer.h"
#include "SerialManager.h"

#define PORT 10000
#define ADDRESS "127.0.0.1"

extern int newfd;
extern int conexionTcp;
pthread_mutex_t mutexData = PTHREAD_MUTEX_INITIALIZER;

void* server_thread (void* message)
{
  	socklen_t addr_len;
	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr;
	char buffer[TAMANO];
	int n;

	// Se crea socket
	int s = socket(AF_INET,SOCK_STREAM, 0);

  	//Se carga datos de IP:PORT del server
	bzero((char*) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	//serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if(inet_pton(AF_INET, ADDRESS, &(serveraddr.sin_addr))<=0)
	{
    	fprintf(stderr,"ERROR invalid server IP\r\n");
    	return NULL;
	}

	//Se abre el puerto con bind()
	if (bind(s, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == ERROR)
    {
		close(s);
		perror("listener: bind");
		return NULL;
	}

	// Se setea socket en modo Listening
	if (listen (s, 10) == ERROR) // backlog=10
  	{
  	perror("error en listen");
  	exit(1);
  	}
  	//permite reconexiones del cliente
	while(START)
	{
		// Se ejecuta accept() para recibir conexiones entrantes
    	printf("server:  esperando recibir conexion entrante\n");
		addr_len = sizeof(struct sockaddr_in);
		if((newfd = accept(s, (struct sockaddr *)&clientaddr,&addr_len)) == ERROR)
  		{
      		perror("error en accept");
     		exit(1);
  		}
		pthread_mutex_lock (&mutexData);
		conexionTcp=TRUE;
		pthread_mutex_unlock (&mutexData);

		char ipClient[32];
		inet_ntop(AF_INET, &(clientaddr.sin_addr), ipClient, sizeof(ipClient));
		printf  ("server:  conexion desde:  %s\n",ipClient);
		//_______________________________________________________

		// Se lee mensaje de cliente
    	do{
      		printf  ("server:  esperando leer mensaje del cliente\n");
      		n = read(newfd,buffer,TAMANO);
     		if(n>EOF_)
      		{
				buffer[n]=0x00;
				printf("Recibi del Cliente %d bytes.:%s\n",n,buffer);
				printf("Enviando mjs a la EDU-CIAA\n");
				serial_send(buffer,TAMANO);//Envia buffer por el puerto serie
      		}
    	}while(n!=EOF_ && n != ERROR);

		//Se cierra conexion con cliente
		pthread_mutex_lock (&mutexData);
		conexionTcp=FALSE;
		pthread_mutex_unlock (&mutexData);

		close(newfd);
		newfd=ERROR;
	}
	return NULL;
}

void close_thread(){
  pthread_mutex_lock (&mutexData);
  if(!conexionTcp){
    conexionTcp=FALSE;
    close(newfd);
  }
  pthread_mutex_unlock (&mutexData);
}
