//#include "Stdbib.h"
//#include "CDEbib.h"
#include "logs.h"

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdarg.h>
#include <ncurses.h>

#define BACKLOG 5
#define STDIN 0

typedef struct listaEsperaCDE
{
	int iSocket;
	int iMinutos;
	char * nombre;
	struct listaEsperaCDE* siguiente;
}listaEsperaCDE;

struct mjeTiempo 
{
	char cabecera [3];
	int tiempo;
}mjeTiempo;

int fiCDEbibTomarConfiguracion (char* , char* , int *);
int fvCDEbibSacarDeVectorOLista (struct listaEsperaCDE * , int , int * ,struct listaEsperaCDE** , int );
void fvCDEbibSacarDeVector (struct listaEsperaCDE * , int , int );
int fiCDEbibTomarConfiguracion (char* , char* , int * );
int fvCDEbibAsignarTurnoEquipo(int ,int , struct listaEsperaCDE * );
int fvCDEbibAgregarAListaDeEspera(int , int ,struct listaEsperaCDE** );
void fvCDEbibSacarNodoListaEspera(int , struct listaEsperaCDE** );
char* fcpStdbibMayusculas(char[]);
char* itoa(int , char* );
int fiCdeRecibirTiempo(int );
void fvCDEbibAsignarTurnoEquipo2(int ,int , struct listaEsperaCDE * , char *);

void init_cde(void);
void cerrar_curses(void);
void init_curses(void);
void clrscr(void);
void wclrscr(WINDOW * );
void printTitle(int , int , char *);
void imprimirEntrenando(struct listaEsperaCDE [], int );
void imprimirCola(struct listaEsperaCDE *);

int main (int argc, char *argv[])
{
	int iSockAccept, iSockfd;
	struct sockaddr_in miDireccion;
	struct sockaddr_in direccionExterna;

	int iYes;
	fd_set readfds;
	fd_set maestrofds;
	socklen_t iAddrlen;
	int iBuf;
	char* cpBufDes;
	int iCantEquiUsanCDE,iCantEquiUsandoCDE;
	struct listaEsperaCDE * listaEsperaCDE;
	struct listaEsperaCDE * ipVectorUsando;
	int iFdmax;
	char * cpIp;
	int  ipPuerto;
	int i;
	char * cpTeclado;
	char * cpNombreAux;
	char iTamanioAux;

	iYes = 1;

	listaEsperaCDE = NULL;

	cpIp = (char *)malloc(16);
	cpTeclado = (char *)malloc(30);

	fvStdIniciarLogs("CDE", "logs.log");
	iCantEquiUsanCDE = fiCDEbibTomarConfiguracion ("./CONFIG", cpIp, &ipPuerto);
	
	init_cde();
	
	miDireccion.sin_family = AF_INET;
	miDireccion.sin_port = htons (ipPuerto);
	inet_aton (cpIp, &(miDireccion.sin_addr));
	bzero (&(miDireccion.sin_zero), 8);

	iSockAccept = socket(AF_INET, SOCK_STREAM, 0);

	iFdmax = iSockAccept;

	if (setsockopt(iSockAccept, SOL_SOCKET, SO_REUSEADDR, &iYes, sizeof (int)) == -1)
	{   
		fvStdLog("cde.c", 94, TLError, "Error del setsockpt");
		exit (0);
	}

	if (bind (iSockAccept, (struct sockaddr *) &miDireccion, sizeof (struct sockaddr)) == -1)
	{
		fvStdLog("cde.c", 100, TLError, "Error del bind");
		perror ("bind");
		exit (1);
	}

	if (listen (iSockAccept, BACKLOG) == -1)
	{
		fvStdLog("cde.c", 107, TLError, "Error del listen");      
		perror ("listen");
		exit (1);
	}

	iCantEquiUsandoCDE = 0;
	ipVectorUsando = (struct listaEsperaCDE*) malloc (iCantEquiUsanCDE * sizeof(struct listaEsperaCDE ));

	FD_ZERO(&maestrofds);
	FD_ZERO(&readfds);
	FD_SET(iSockAccept, &maestrofds);
	FD_SET(STDIN, &maestrofds);

	while (1)
	{
		readfds = maestrofds;
		imprimirCola(listaEsperaCDE);
		imprimirEntrenando(ipVectorUsando, iCantEquiUsandoCDE);

		if(select (iFdmax + 1, &readfds, NULL, NULL, NULL) == -1)
		{
			fvStdLog("cde.c", 126, TLError, "Error del select");      
			exit(1);
		}
		for(i=0;i<=iFdmax;i++)
		{	
			if(FD_ISSET(i, &readfds))
			{
				if (i==iSockAccept) /* salio del select porque algun equipo se me quiere conectar */
				{
					iAddrlen= sizeof(struct sockaddr);
					if((iSockfd = accept (iSockAccept, (struct sockaddr *) &direccionExterna,&iAddrlen)) == -1) 
					{
						fvStdLog("cde.c", 138, TLError, "Error del accept"); 
						exit(1);
					}
					if(iSockfd>iFdmax)
					{
						iFdmax=iSockfd;
					}

					FD_SET(iSockfd, &maestrofds);

					fvStdLog("cde.c", 148, TLInformacion, "conexion aceptada en %d", iSockfd);      

				} /*Cierre del if(si algun equipo se me queria conectar)*/
				else /*o me manda el tiempo o se Desconecta*/
				{
					if ((iBuf=fiCdeRecibirTiempo(i)) == -1) 
					{
						fvStdLog("cde.c", 165, TLError, "Error al recibir PC de SOCKET:%d",i); 
						FD_CLR(i,&maestrofds);
						close(i);
						fvCDEbibSacarDeVectorOLista(ipVectorUsando, i,&iCantEquiUsandoCDE,&listaEsperaCDE, iCantEquiUsanCDE);
					}
					else if (iBuf == 0)
					{
						fvStdLog("cde.c", 169, TLInformacion, "Se Desconecto equipo Socket: %d", i); 
						FD_CLR(i,&maestrofds);
						close(i);
						fvCDEbibSacarDeVectorOLista(ipVectorUsando, i,&iCantEquiUsandoCDE,&listaEsperaCDE, iCantEquiUsanCDE);
					}
					else if(iCantEquiUsandoCDE  < iCantEquiUsanCDE)
					{
						if(fvCDEbibAsignarTurnoEquipo(i,iCantEquiUsandoCDE, ipVectorUsando)==-1)
						{
							FD_CLR(i,&maestrofds);
							close(i);
						}
						else
							iCantEquiUsandoCDE = iCantEquiUsandoCDE + 1;
					}
					else
					{
						if(fvCDEbibAgregarAListaDeEspera(i, iBuf ,&listaEsperaCDE)==-1)
						{
							FD_CLR(i,&maestrofds);
							close(i);
						}
						else
							fvStdLog("cde.c", 182, TLInformacion, "Agregado a lista de Espera %d", iSockfd); 
					}
				}
			}
		}
	}
}


/*LAS FUNCIONES */
/*-------------------------------------------------------------------------------------------------------------------*/
int fvCDEbibSacarDeVectorOLista (struct listaEsperaCDE * ipVectorUsando, int iSockfd, int * iCantEquiUsandoCDE,struct listaEsperaCDE** listaEsperaCDE, int iCantEquiUsanCDE)
{
	int i;
	struct listaEsperaCDE* listaRecorrer;
	int iSock;


	for(i=0; i< *iCantEquiUsandoCDE; i++)
	{
		if (ipVectorUsando[i].iSocket == iSockfd)
		{
			fvCDEbibSacarDeVector(ipVectorUsando,i, *iCantEquiUsandoCDE);

			*iCantEquiUsandoCDE = *iCantEquiUsandoCDE - 1;

			if(*listaEsperaCDE != NULL)
			{
				fvCDEbibAsignarTurnoEquipo2((*listaEsperaCDE)->iSocket,*iCantEquiUsandoCDE,ipVectorUsando,(*listaEsperaCDE)->nombre);
				*iCantEquiUsandoCDE = *iCantEquiUsandoCDE + 1;
				//fvCDEbibSacarNodoListaEspera((*listaEsperaCDE)->iSocket, listaEsperaCDE);				
				listaRecorrer = *listaEsperaCDE;
				*listaEsperaCDE = (*listaEsperaCDE)->siguiente;
				free(listaRecorrer->nombre);
				free(listaRecorrer);
			}
			return 1;

		}
	}
	listaRecorrer = *listaEsperaCDE;
	while (listaRecorrer != NULL)
	{
		if ((listaRecorrer)->iSocket == iSockfd)
		{
			iSock = (listaRecorrer)->iSocket;
			(listaRecorrer) = (listaRecorrer)->siguiente;
			fvCDEbibSacarNodoListaEspera(iSock,  listaEsperaCDE);
			return 1;
		}
		else
		{
			(listaRecorrer) = (listaRecorrer)->siguiente;
		}
	}

	fvStdLog("cde.c",231,TLError,"No se encontro al Socket:%d",iSockfd);
	return -1;
}
/*-------------------------------------------------------------------------------------------------------*/
void fvCDEbibSacarDeVector (struct listaEsperaCDE * ipVectorUsando, int iPos, int iCantEquiUsandoCDE)
{
	free(ipVectorUsando[iPos].nombre);
	while(iPos < (iCantEquiUsandoCDE-1))
	{
		ipVectorUsando[iPos]= ipVectorUsando[iPos + 1];
		iPos++;
	}
}
/*--------------------------------------------------------------------------------------------------------*/
int fiCDEbibTomarConfiguracion (char* cpArch, char* cpIp, int * ipPuerto)
{
	FILE * archivo;
	char* cpLeido;
	char* cpVar;
	char *cpDatos;
	int iCantEquip;

	if ((archivo = fopen (cpArch, "r")) == NULL)
	{
		perror ("fopen");
		exit (1);
	}

	cpLeido = (char *) malloc (100);
	while (fgets (cpLeido, 100,archivo) != NULL)
	{
		cpVar = strtok (cpLeido, ":");
		cpDatos = strtok (NULL, "\n");

		if (!strcmp(fcpStdbibMayusculas(cpVar), "IP"))
		{
			strcpy((cpIp), cpDatos);

		}
		if (!strcmp( fcpStdbibMayusculas(cpVar), "PUERTO"))
		{
			*ipPuerto = atoi(cpDatos);

		}
		if (!strcmp ((char *) fcpStdbibMayusculas (cpVar), "CANTEQUIPOS"))
		{
			iCantEquip  = atoi (cpDatos);

		}

	}
	free(cpLeido);
	fclose (archivo);
	return (iCantEquip);
}
/*-----------------------------------------------------------------------------------------------------------*/
int fvCDEbibAsignarTurnoEquipo(int iSockfd,int iCantEquiUsandoCDE, struct listaEsperaCDE * ipVectorUsando)
{
	char cTam;
	char * cpNombre;	
	if(recv(iSockfd, &cTam, 1, 0)<=0)
	{
		return -1;

	}
	cpNombre = (char *) malloc(cTam);
	if(recv(iSockfd, cpNombre, cTam, 0)<=0)
	{
		free(cpNombre);
		return -1;
	}
	
	if(send(iSockfd, "RC", 2, 0)==-1)
		fvStdLog("cde.c",289,TLError,"Error al enviar NC al socket: %d", iSockfd);
	else
	{
		ipVectorUsando[iCantEquiUsandoCDE].nombre = (char *) malloc(cTam);
		strcpy(ipVectorUsando[iCantEquiUsandoCDE].nombre, cpNombre); 
		ipVectorUsando[iCantEquiUsandoCDE].iSocket = iSockfd;
		fvStdLog("cde.c", 293, TLInformacion, "Asignado Turno a %s", ipVectorUsando[iCantEquiUsandoCDE].nombre); 
	}
	free(cpNombre);
	return 0;
}
void fvCDEbibAsignarTurnoEquipo2(int iSockfd,int iCantEquiUsandoCDE, struct listaEsperaCDE * ipVectorUsando, char *cpNombre)
{
	if(send(iSockfd, "RC", 2, 0)==-1)
		fvStdLog("cde.c",289,TLError,"Error al enviar NC al socket: %d", iSockfd);
	else
	{
		ipVectorUsando[iCantEquiUsandoCDE].nombre = (char *) malloc(strlen(cpNombre)+1);
		strcpy(ipVectorUsando[iCantEquiUsandoCDE].nombre, cpNombre); 
		ipVectorUsando[iCantEquiUsandoCDE].iSocket = iSockfd;
		fvStdLog("cde.c", 293, TLInformacion, "Asignado Turno a %s", ipVectorUsando[iCantEquiUsandoCDE].nombre); 
	}
}

/*----------------------------------------------------------------------------------------------------------*/
int fvCDEbibAgregarAListaDeEspera(int iSockfd, int iCantMin,struct listaEsperaCDE** listaEsperaCDE)
{
	struct listaEsperaCDE* listaRecorrer;
	struct listaEsperaCDE *nuevo;
	char cTam;
	char * cpNombre;
	
	if(recv(iSockfd, &cTam, 1, 0)<=0)
		return -1;
	cpNombre = (char*) malloc(cTam);
	if(recv(iSockfd, cpNombre, cTam, 0)<=0)
	{
		free(cpNombre);
		return -1;
	}
	
	if(send(iSockfd,"NC",2,0)==-1)
	{
		fvStdLog("cde.c",304,TLError,"Error al enviar NC al socket: %d", iSockfd);
	}
	else
	{

		nuevo=(struct listaEsperaCDE*)malloc(sizeof(struct listaEsperaCDE));
		nuevo->nombre = (char *) malloc(strlen(cpNombre)+1);
		strcpy(nuevo->nombre, cpNombre);
		
		nuevo->iSocket = iSockfd;
		nuevo->iMinutos = iCantMin;
		
		if(*listaEsperaCDE==NULL || (*listaEsperaCDE)->iMinutos > iCantMin)
		{
			nuevo->siguiente = *listaEsperaCDE;
			*listaEsperaCDE=nuevo;

		}
		else 
		{
			listaRecorrer = (*listaEsperaCDE);
			while(listaRecorrer->siguiente != NULL && listaRecorrer->siguiente->iMinutos <= iCantMin)
				listaRecorrer = listaRecorrer->siguiente;
			nuevo->siguiente = listaRecorrer->siguiente;
			listaRecorrer->siguiente = nuevo;
		}
	}
	free(cpNombre);
	return 0;
}

/*----------------------------------------------------------------------------------------------------------*/
void fvCDEbibSacarNodoListaEspera(int iSocket, struct listaEsperaCDE** listaEsperaCDE)
{
	struct listaEsperaCDE * listaAux, *listaAux2;

	if (((*listaEsperaCDE) != NULL) && ((*listaEsperaCDE)->iSocket == iSocket))
	{
		listaAux = (*listaEsperaCDE);
		(*listaEsperaCDE) = listaAux->siguiente;
		free(listaAux->nombre);
		free(listaAux);
	}

	else if((*listaEsperaCDE) != NULL)
	{
		listaAux = (*listaEsperaCDE);
		listaAux2 = listaAux->siguiente;

		while(listaAux2!=NULL)
		{
			if(listaAux2->iSocket == iSocket)
			{
				listaAux->siguiente = listaAux2->siguiente;
				free(listaAux2->nombre);
				free(listaAux2);
				listaAux2 = listaAux->siguiente;
			}
			else
			{
				listaAux=listaAux->siguiente;
				listaAux2=listaAux->siguiente;
			}
		}
	}
}
/*--------------------------------------------------------------------------------------------------*/
char* fcpStdbibMayusculas(char cpCadena[])
{
	int i;
	for(i=0; cpCadena[i] != '\0'; i++)
		cpCadena[i] = toupper(cpCadena[i]);
	return (&cpCadena[0]);
}
/*-------------------------------------------------------------------------------------------------*/
char* itoa(int n, char* string)
{
	sprintf (string, "%d", n);
	return string;
}
/*---------------------------------------------------------------------------------------------------*/
int fiCdeRecibirTiempo(int iSockfd)
{
	int iRec;
	int iTiempo;
	struct mjeTiempo  mje;

	if((iRec=recv(iSockfd, &mje, sizeof(struct mjeTiempo), 0))== -1)
	{
		fvStdLog("cde.c",389,TLError,"Error al Recibir tiempo de un equipo socket:%d", iSockfd);
		//perror("recv");
		return -1;
	}
	else if (iRec==0)
		return 0;
	else if(strncmp(mje.cabecera, "PC",2))
	{
		fvStdLog("cde.c",397,TLError,"Me mandaron cualquier cosa socket:%d", iSockfd);
		return -1;
	}

	iTiempo= mje.tiempo;

	return iTiempo;
}
/*-----------------------------------------------------------------------------------------------------------------*/
