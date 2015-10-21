#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "routerBib.h"

int fiRouRecibirCabeceraCiudad(int iSock)
{
	char cpBuf[2];
	int iTam;

	if((iTam = recvAll (iSock, cpBuf, 2, 0))==-1)
	{
		perror("recvAll");
		exit(1);
	}

	if (iTam <= 0)
		return (MSDesconectadoC);

	if (!(strncmp (cpBuf, "TP", 2))) /*LEV INTERNA - LI|CANT|RES|LARGO|NOMBRE|...*/
		return (MSTDP);

	if (!(strncmp (cpBuf, "DT", 2))) 
		return (MSDT);

	if (!(strncmp (cpBuf, "PB", 2)))
		return (MSPedido);

	if (!(strncmp (cpBuf, "EE", 2)))
		return (MSEncontrado);

	if (!(strncmp (cpBuf, "NE", 2)))
		return (MSNoEncontrado);

	return (-1);
}
/*---------------------------------------------------------------------------------------------------*/
int fiRouRecibirCabeceraRouter(int iSock, char * identificador, char * ttl, char * hops, int * largoMensaje)
{
	char cpBufCab[7];
	int iTam;

	if((iTam = recvAll (iSock, cpBufCab, 7, 0)) == -1)
	{
			perror("recvAll");
			exit(1);
	}
	
	if (iTam <= 0)
		return (MSDesconectadoR);
	else
	{
		identificador[0] = cpBufCab[0];
		identificador[1] = cpBufCab[1];
		(*ttl) = cpBufCab[3];
		(*hops) = cpBufCab[4];
		(*largoMensaje) = ((unsigned char)cpBufCab[5]) + (256 * (unsigned char)cpBufCab[6]);
	}
	return (cpBufCab[2]);
}
/*----------------------------------------------------------------------------------------------------------------*/
void  fiRouRecibirDatosCiudad(int iSockCiudad, char ** cpNombre)
{
	char cTamanio;

	if(recvAll(iSockCiudad,&cTamanio,1,0)<=0)
	{
		perror("recvAll");
		exit(1);
	}

	*cpNombre=(char *)malloc(cTamanio);

	if(recvAll(iSockCiudad,*cpNombre,cTamanio,0)<=0)
	{
		perror("recvAll");
		exit(1);
	}
}
/*---------------------------------------------------------------------------------------------------------------*/
int fiRouRecibirTDP(int iSocket, struct sTDP ** lista)
{
	char cpBufInicial[3];
	char cpBufNombre[257];
	char cpBufCiudad[256];
	char cpBufComienzo[5];
	struct sTDP * nuevo;
	int i;

	recvAll(iSocket, cpBufInicial, 3, 0);

	for(i=0; i<cpBufInicial[2]; i++)
	{
		recvAll(iSocket, cpBufComienzo, 5, 0);
		recvAll(iSocket, cpBufNombre, cpBufComienzo[4]+1, 0);
		recvAll(iSocket, cpBufCiudad, cpBufNombre[(int)cpBufComienzo[4]], 0);
		nuevo	= (sTDP*)malloc ( sizeof(sTDP) );
		if ( nuevo==NULL )
		{
			exit (1);
		}
		nuevo->nombre	= (char*)malloc (strlen(cpBufNombre) + 1);
		if ( nuevo->nombre==NULL )
		{
			exit (1);
		}
		nuevo->ciudad	= (char*)malloc (strlen(cpBufCiudad) + 1);
		if ( nuevo->ciudad==NULL )
		{
			exit (1);
		}
		nuevo->puntos = cpBufComienzo[0];
		nuevo->pg = cpBufComienzo[1];
		nuevo->pe = cpBufComienzo[2];
		nuevo->pp = cpBufComienzo[3];
		strcpy(nuevo->ciudad, cpBufCiudad);
		strcpy(nuevo->nombre, cpBufNombre);
		nuevo->siguiente = (*lista);
		(*lista) = nuevo;
	}
	return 1;
}
/*---------------------------------------------------------------------------------------------------------------*/
int fiRouRecibirDiscoverC(int iSocket,char * cpMensajeFD)
{
	char cLargo;
	char * cpNombre;
	char * cpMensaje;

	if(recvAll(iSocket,&cLargo,1,0)<0)
	{
		perror("recvAll");
		exit(1);
	}
	cpNombre = (char *)malloc(cLargo);

	if(recvAll(iSocket,cpNombre,(unsigned char)cLargo,0)<0)
	{
		perror("recvAll");
		exit(1);
	}

	cpMensaje=(char *)malloc(9+cLargo);
	strcpy(cpMensaje,"FO");
	cpMensaje[2]=cLargo;
	memcpy(&cpMensaje[3],cpNombre,cLargo);
	memcpy(&cpMensaje[3+cLargo],cpMensajeFD,6);

	if(send(iSocket,cpMensaje,9+cLargo,0)==-1)
	{
		perror("send");
		exit(1);
	}

	free(cpNombre);
	free(cpMensaje);
}

struct sDiscover * fiRouBuscarDiscoverID(struct sDiscover *lista, char * cpId)
{
	while (lista)
	{
		if (compararVector(cpId, lista->iIdentificador, 2))
			return lista;
		lista = lista->siguiente;
	}
	return lista;
}

int fiRouAgregarRouter(struct sRouter ** sListaRout, int socket)
{			
	struct sRouter * nuevo;
	nuevo = malloc(sizeof(struct sRouter));
	memset(nuevo->cpIpPuerto, 0, 6);
	nuevo->iSocket = socket;
	nuevo->iTengoDatos = 0;
	nuevo->siguiente = (*sListaRout);
	(*sListaRout) = nuevo;
}

struct sRouter * fiRouBuscarRouterSocket(sRouter * lista, int socket)
{
	while(lista && (socket != lista->iSocket))
		lista = lista->siguiente;
	return lista;
}

void fvRouEnviarTokenInicial(struct sRouter * sListaR,char * cpNombreCiudad, char * cpMiIpPuerto, struct sCiudad ** sListaC, int iSockCiudad, char * cpID)
{
	char cOrden;
	struct sRouter * sAux;
	struct sCiudad * sCiuNueva;
	time_t tiempo;

	cOrden = 1;
	srandom(time(&tiempo));
	
	cpID[0] = random() % 256;
	cpID[1] = random() % 256;

	sCiuNueva = (struct sCiudad *) malloc(sizeof(struct sCiudad));
	sCiuNueva->orden = cOrden++;
	sCiuNueva->nombre = (char *) malloc(strlen(cpNombreCiudad)+1);
	strcpy(sCiuNueva->nombre, cpNombreCiudad);
	memcpy(sCiuNueva->ipPuerto, cpMiIpPuerto, 6);
	sCiuNueva->siguiente = NULL;
	(*sListaC)=sCiuNueva;
	
	sAux = sListaR;
	while(sAux)
	{
		if(sAux->iTengoDatos)
		{
			sCiuNueva = (struct sCiudad *) malloc(sizeof(struct sCiudad));
			sCiuNueva->orden = cOrden++;
			sCiuNueva->nombre = (char *) malloc(1);
			strcpy(sCiuNueva->nombre, "");
			memcpy(sCiuNueva->ipPuerto, sAux->cpIpPuerto, 6);
			sCiuNueva->siguiente = (*sListaC);
			(*sListaC)=sCiuNueva;
		}
		sAux = sAux->siguiente;
	}

	if(sendall(iSockCiudad, "TP\0\0\0", 5) == -1)
	{
		perror("sendall");
		exit(1);
	}
}

/*--------------------------------------------------------------------------------------------------------------------------*/
int fiRouEnviarTokenProximoRouter(struct sRouter ** sListaR,char * cpID,struct sTDP ** sListaT,struct sCiudad ** sListaC, char * cpMiIpPuerto)
{
	int iSocket;
	int iTamanio;
	int iPosicion;
	int iCantidadC;
	int iCantidadT;
	char * cpMensaje;
	struct sCiudad * auxCiu;
	struct sTDP * auxTDP;
	struct sCiudad * auxAuxCiu;
	struct sTDP * auxAuxTDP;


	if ((iSocket = fiRouConectarProximoRouter(sListaC, cpMiIpPuerto, sListaT)) == 0)
	{
		return 0;
	}

	auxCiu = * sListaC;
	auxTDP = * sListaT;

	iTamanio = 3;
	iCantidadC = 0;
	iCantidadT = 0;

	while(auxCiu)
	{
		iCantidadC++;
		iTamanio++;
		iTamanio += 2 + strlen(auxCiu->nombre);
		iTamanio += 6;
		auxCiu = auxCiu->siguiente;
	}
	
	iTamanio += 3;

	while(auxTDP)
	{
		iCantidadT++;
		iTamanio += 2 + strlen(auxTDP->ciudad);
		iTamanio += 2 + strlen(auxTDP->nombre);
		iTamanio += 4;
		auxTDP=auxTDP->siguiente;
	}

	auxCiu = * sListaC;
	auxTDP = * sListaT;

	cpMensaje = (char *) malloc(iTamanio);

	memcpy(cpMensaje, cpID, 2);
	cpMensaje[2] = iCantidadC;
	iPosicion = 3;

	while(auxCiu)
	{	
		cpMensaje[iPosicion++] = auxCiu->orden;		
		cpMensaje[iPosicion++] = strlen(auxCiu->nombre) +1;
		memcpy(&(cpMensaje[iPosicion]), auxCiu->nombre, strlen(auxCiu->nombre)+1);
		iPosicion +=  strlen(auxCiu->nombre)+1;
		memcpy(&(cpMensaje[iPosicion]), auxCiu->ipPuerto, 6);
		iPosicion += 6;
		auxAuxCiu = auxCiu;
		auxCiu = auxCiu->siguiente;
		free(auxAuxCiu->nombre);
		free(auxAuxCiu);
	}
	iPosicion+=2;
	cpMensaje[iPosicion++] = iCantidadT;
	while(auxTDP)
	{	
		cpMensaje[iPosicion++] = auxTDP->puntos;
		cpMensaje[iPosicion++] = auxTDP->pg;
		cpMensaje[iPosicion++] = auxTDP->pe;
		cpMensaje[iPosicion++] = auxTDP->pp;

		cpMensaje[iPosicion++] = strlen(auxTDP->nombre) + 1;
		memcpy(&(cpMensaje[iPosicion]), auxTDP->nombre, strlen(auxTDP->nombre)+1);
		iPosicion +=  strlen(auxTDP->nombre)+1;

		cpMensaje[iPosicion++] = strlen(auxTDP->ciudad) + 1;
		memcpy(&(cpMensaje[iPosicion]), auxTDP->ciudad, strlen(auxTDP->ciudad)+1);
		iPosicion +=  strlen(auxTDP->ciudad)+1;
		auxAuxTDP = auxTDP;
		auxTDP=auxTDP->siguiente;
		free(auxAuxTDP->nombre);
		free(auxAuxTDP->ciudad);
		free(auxAuxTDP);
	}
	if(sendall(iSocket, cpMensaje, iTamanio) == -1)
	{
		perror("send");
		exit(1);
	}

	(* sListaC) = NULL;
	(* sListaT) = NULL;
	free(cpMensaje);
	close(iSocket);
	return 1;
}

int fiRouConectarProximoRouter(struct sCiudad ** sListaC, char * cpMiIpPuerto,struct sTDP ** sListaT)
{
	struct sCiudad * sCiuAux;
	int i;
	int iMiOrden;
	int iSocket;

	sCiuAux = fiRouBuscarCiudadIPPuerto(*sListaC, cpMiIpPuerto);
	iMiOrden = sCiuAux->orden;
	i = iMiOrden + 1;

	while(i != iMiOrden)
	{	
		sCiuAux = fiRouBuscarCiudadOrden(*sListaC, i);
		if(i == iMiOrden)
		{
			return 0;	
		}
		else
		{
			if((iSocket = fiRouConectar((char*)sCiuAux->ipPuerto))!=-1)
			{
					return iSocket;
			}
			else
			{
				fiRouSacarListas(sListaT,sListaC,sCiuAux->nombre);
				sCiuAux = fiRouBuscarCiudadIPPuerto(*sListaC, cpMiIpPuerto);
				iMiOrden=sCiuAux->orden;
				i = (sCiuAux->orden) + 1;
			}
		}
	}
	return 0;
}

int fiRouSacarListas(struct sTDP ** sListaT,struct sCiudad ** sListaC,char * nombre)
{
	struct sCiudad * sAuxC;
	struct sCiudad * sAuxC2;
	struct sTDP * sAuxT;
	struct sTDP * sAuxT2;
	char * nombreAux;
	int iOrden;
  
	nombreAux = (char*) malloc(strlen(nombre)+1);
	strcpy(nombreAux, nombre);

	while (((*sListaT) != NULL) && (!strcmp(nombreAux, (*sListaT)->ciudad)))
	{
		sAuxT = (*sListaT);
		(*sListaT) = sAuxT->siguiente;
		free(sAuxT->ciudad);
		free(sAuxT->nombre);
		free(sAuxT);
	}
	if((*sListaT) != NULL)
	{
		sAuxT = (*sListaT);
		sAuxT2 = sAuxT->siguiente;
		while(sAuxT2!=NULL)
		{
			if(!strcmp(nombreAux, sAuxT2->ciudad))
			{
				sAuxT->siguiente = sAuxT2->siguiente;
				free(sAuxT2->ciudad);
				free(sAuxT2->nombre);
				free(sAuxT2);
				sAuxT2 = sAuxT->siguiente;
			}
			else
			{
				sAuxT=sAuxT2;
				sAuxT2=sAuxT2->siguiente;
			}
		}
	}

	if(((*sListaC) != NULL) && (!strcmp(nombreAux, (*sListaC)->nombre)))
	{
		sAuxC = (*sListaC);
		(*sListaC) = sAuxC->siguiente;
		iOrden = sAuxC->orden;
		free(sAuxC->nombre);
		free(sAuxC);
	}
	else if((*sListaC) != NULL)
	{
		sAuxC = (*sListaC);
		sAuxC2 = sAuxC->siguiente;
		while(sAuxC2!=NULL)
		{
			if(!strcmp(nombreAux, sAuxC2->nombre))
			{
				sAuxC->siguiente = sAuxC2->siguiente;
				iOrden = sAuxC2->orden;
				free(sAuxC2->nombre);
				free(sAuxC2);
				sAuxC2 = sAuxC->siguiente;
			}
			else
			{
				sAuxC=sAuxC2;
				sAuxC2=sAuxC2->siguiente;
			}
		}
	}

	sAuxC = *sListaC;

	while(sAuxC)
	{
		if (sAuxC->orden > iOrden)
			(sAuxC->orden)--;
		sAuxC = sAuxC->siguiente;
	}
	free(nombreAux);
	return 1;
}

struct sCiudad * fiRouBuscarCiudadIPPuerto(struct sCiudad * sListaC, char * cpIpPuerto)
{
	while(sListaC)
	{
		if(compararVector(cpIpPuerto, sListaC->ipPuerto, 6))
			return(sListaC);
		sListaC=sListaC->siguiente;
	}
}

int compararVector(char * vec1, char * vec2, int tam)
{
	int i;
	for(i=0; i<tam;i++)
		if(vec1[i] != vec2[i])
			return(0);
	return(1);
}

struct sCiudad * fiRouBuscarCiudadOrden(struct sCiudad * sListaC, int orden)
{
	struct sCiudad * sCiuAux;

	sCiuAux = sListaC;

	while(sCiuAux)
	{
		if(sCiuAux->orden == orden)
			return(sCiuAux);
		sCiuAux = sCiuAux->siguiente;
	}

	sCiuAux = sListaC;

	while(sCiuAux)
	{
		if(sCiuAux->orden == 1)
			return(sCiuAux);
		sCiuAux = sCiuAux->siguiente;
	}

}

int fiRouConectar(char* cpIpPuerto)
{
	int iSockfd;
	struct sockaddr_in dirCiudad;
	int iYes;
	char cpIp[16];
	int iPuerto;

	iYes= 1;

	fiRouPrintfIP(cpIpPuerto, cpIp, &iPuerto);
	//sprintf(cpIp, "%u.%u.%u.%u", (unsigned char)(cpIpPuerto[0]), (unsigned char)(cpIpPuerto[1]),(unsigned char) (cpIpPuerto[2]), (unsigned char)(cpIpPuerto[3]));
	//iPuerto = (unsigned char)cpIpPuerto[4] + (unsigned char)cpIpPuerto[5] * 256;

	if((iSockfd= socket(AF_INET,SOCK_STREAM,0))<0)
	{
		return -1;
	}

	dirCiudad.sin_family= AF_INET;
	dirCiudad.sin_port= htons(iPuerto);
	inet_aton (cpIp, &(dirCiudad.sin_addr));
	bzero (&(dirCiudad.sin_zero), 8);

	if((connect(iSockfd,(struct sockaddr*)&dirCiudad, sizeof(struct sockaddr)))==-1)
	{
		return -1;
	}

	return iSockfd;
}
/*--------------------------------------------------------------------------------------------------------------------------------*/
int fiRouEnviarTDP(int iSock,struct sTDP** sTabla)
{
	char * cpMensaje;
	struct sTDP * sTablaRecorrer;
	struct sTDP * sTablaAux;
	int iTamanio;
	int iPosicion;
	int iCantidad;

	iTamanio = 5;	
	iCantidad = 0;
	sTablaRecorrer = (*sTabla);

	while(sTablaRecorrer != NULL)
	{
		iTamanio += 6 + strlen(sTablaRecorrer->nombre) + 1 + strlen(sTablaRecorrer->ciudad) + 1;
		sTablaRecorrer = sTablaRecorrer->siguiente;
		iCantidad ++;
	}

	cpMensaje	= (char*)malloc (iTamanio);
	if ( cpMensaje==NULL )
	{
		exit (EXIT_FAILURE);
	}

	sTablaRecorrer = (*sTabla);
	strcpy(cpMensaje, "TP");
	cpMensaje[2]=(unsigned char)(iTamanio-4)%256;
	cpMensaje[3]=(unsigned char)(iTamanio-4)/256;
	cpMensaje[4]= iCantidad;
	iPosicion = 5;

	while(sTablaRecorrer != NULL)
	{
		cpMensaje[iPosicion++] = sTablaRecorrer->puntos;
		cpMensaje[iPosicion++] = sTablaRecorrer->pg;
		cpMensaje[iPosicion++] = sTablaRecorrer->pe;
		cpMensaje[iPosicion++] = sTablaRecorrer->pp;

		cpMensaje[iPosicion++] = strlen(sTablaRecorrer->nombre) + 1;
		memcpy(&(cpMensaje[iPosicion]), sTablaRecorrer->nombre, strlen(sTablaRecorrer->nombre) + 1);
		iPosicion +=  strlen(sTablaRecorrer->nombre) + 1;

		cpMensaje[iPosicion++] = strlen(sTablaRecorrer->ciudad) + 1;
		memcpy(&(cpMensaje[iPosicion]), sTablaRecorrer->ciudad, strlen(sTablaRecorrer->ciudad) + 1);
		iPosicion +=  strlen(sTablaRecorrer->ciudad) + 1;
		sTablaAux = sTablaRecorrer;
		sTablaRecorrer = sTablaRecorrer->siguiente;
		free(sTablaAux->ciudad);
		free(sTablaAux->nombre);
		free(sTablaAux);
	}
	if (sendall(iSock, cpMensaje, iTamanio) == -1)
	{
		perror("Sendall");
		exit (0);
	}	
	free (cpMensaje);
	(*sTabla) = NULL;
	return 1;
}

/*--------------------------------------------------------------------------------------------------------------*/
int fiRouRecibirPedido(int iSocket,struct sDiscover ** sListaD, struct sRouter * sListaR,char ttlIni) 
{
	unsigned char cTam;
	struct sDiscover * nuevo;
	time_t tiempo;

	nuevo = (struct sDiscover *)malloc(sizeof(struct sDiscover));

	if(recv(iSocket,&cTam,1,0)==-1)
	{
		perror("recvAll");
		exit(1);
	}
	nuevo->cpNombreCiudad=(char *)malloc(cTam);
	if(recvAll(iSocket,nuevo->cpNombreCiudad, cTam, 0)==-1)
	{
		perror("recvAll");
		exit(1);
	}
	if(recv(iSocket,&cTam,1,0)==-1)
	{
		perror("recvAll");
		exit(1);
	}
	nuevo->cpNombreEquipo=(char *)malloc(cTam);
	if(recvAll(iSocket,nuevo->cpNombreEquipo, cTam, 0)==-1)
	{
		perror("recvAll");
		exit(1);
	}


	srandom(time(&tiempo));

	nuevo->iIdentificador[0]=random() % 256;
	nuevo->iIdentificador[1]=random() % 256;
	nuevo->iSocket = 0;
	nuevo->tiempo = tiempo;
	nuevo->ttl = ttlIni;
	nuevo->hops = 0;

	nuevo->siguiente = (*sListaD);
	(*sListaD)=nuevo;

	if(!fiRouEnviarDiscovers(0,sListaR,ttlIni,0,nuevo->cpNombreCiudad,nuevo->cpNombreEquipo,nuevo->iIdentificador))
	{
		fiRouEnviarNotFoundCiudad(iSocket,nuevo->cpNombreCiudad,nuevo->cpNombreEquipo);
		(*sListaD) = (*sListaD)->siguiente;
		free(nuevo);
	}

	return 1;

}
/*--------------------------------------------------------------------------------------------------------------*/
int fiRouEnviarDiscovers( int iSockRecibido,struct sRouter * sListaR, char ttl,char hops ,char * cpNombreCiu, char * cpNombreEqui ,char * cpID)
{
	char * cpMensaje;
	int iLargo;
	int iEnviado = 0;

	iLargo =  strlen(cpNombreEqui) + 1 + strlen(cpNombreCiu) + 1;
	cpMensaje = (char *) malloc(2 + 1 + 1 + 1 + 2 + iLargo);
	
	memcpy(cpMensaje, cpID, 2);
	cpMensaje[2] = 0;
	cpMensaje[3] = ttl;
	cpMensaje[4] = hops;
	cpMensaje[5] =  iLargo % 256;
	cpMensaje[6] =  iLargo / 256;
	memcpy(&(cpMensaje[7]), cpNombreCiu, strlen(cpNombreCiu)+1);
	memcpy(&(cpMensaje[7 + strlen(cpNombreCiu)+1]), cpNombreEqui, strlen(cpNombreEqui)+1);
	
	while(sListaR)
	{
		if(sListaR->iSocket != iSockRecibido)
		{
			if(sendall(sListaR->iSocket, cpMensaje, 2 + 1 + 1 + 1 + 2 + iLargo) == -1)
			{
				perror("sendall");
				exit(1);
			}
			iEnviado = 1;
		}
		sListaR = sListaR->siguiente;
	}
	free(cpMensaje);
	return iEnviado;
}
/*----------------------------------------------------------------------------------------------------------*/
int fiRouRecibirToken(int iSocket, char * cpID, struct sTDP ** sListaTDP, struct sCiudad ** sListaC, struct sRouter * sListaR, char * cpIpPuertoTok, char *cpNombreCiu)
{
	char cCantidad;
	char cTamanio;
	char * cpMensaje;
	struct sCiudad * sCiuNueva;
	int i;
	
	if(recvAll(iSocket, cpID, 2, 0) == -1)
	{
		perror("recvAll");
		exit(1);
	}
	if(recvAll(iSocket, &cCantidad, 1, 0) == -1)
	{
		perror("recvAll");
		exit(1);
	}
	for(i=0;i<cCantidad;i++)
	{
		sCiuNueva = (struct sCiudad *) malloc(sizeof(struct sCiudad));
		if(recvAll(iSocket, &(sCiuNueva->orden), 1, 0) == -1)
		{
			perror("recvAll");
			exit(1);
		}
		if(recvAll(iSocket, &cTamanio, 1, 0) == -1)
		{
			perror("recvAll");
			exit(1);
		}
		sCiuNueva->nombre = (char *) malloc(cTamanio);
		if(recvAll(iSocket, sCiuNueva->nombre, cTamanio, 0) == -1)
		{
			perror("recvAll");
			exit(1);
		}
		if(recvAll(iSocket, sCiuNueva->ipPuerto, 6, 0) == -1)
		{
			perror("recvAll");
			exit(1);
		}
		if(compararVector(sCiuNueva->ipPuerto, cpIpPuertoTok, 6))
		{
			if(strcmp(cpNombreCiu, sCiuNueva->nombre))
			{
				free(sCiuNueva->nombre);
				sCiuNueva->nombre = (char*) malloc(strlen(cpNombreCiu)+1);
				strcpy(sCiuNueva->nombre, cpNombreCiu);
			}
		}
		sCiuNueva->siguiente = (*sListaC);
		(*sListaC) = sCiuNueva;
	}
	
	fiRouRecibirTDP(iSocket, sListaTDP);

	fiRouCompletarToken(sListaC, sListaR);
}
/*------------------------------------------------------------------------------------------------------------*/
int fiRouManejarDiscover(int iSocket,struct sDiscover** sListaD,struct sRouter * sListaR, char * cpNombreCiu, int iSockCiudad, char * cpID, char ttl, char hops, int iLargoMensaje)
{
	char * cpMensaje;
	char * cpMensajeEnviar;
	char * nombreEquipo;
	struct sDiscover * nuevo;
	time_t tiempo;

	cpMensaje = (char*) malloc(iLargoMensaje);

	if(recvAll(iSocket, cpMensaje, iLargoMensaje, 0) == -1)
	{
		perror("recvAll");
		exit(1);
	}
	
	if(fiRouBuscarDiscoverID(*sListaD, cpID))
	{
		return 0;
	}

	srandom(time(&tiempo));

	nombreEquipo = &(cpMensaje[strlen(cpMensaje)+1]);
	nuevo=(struct sDiscover*)malloc(sizeof(struct sDiscover));
	nuevo->cpNombreCiudad=(char *)malloc(strlen(cpMensaje)+1);
	nuevo->cpNombreEquipo=(char *)malloc(strlen(nombreEquipo)+1);
	strcpy(nuevo->cpNombreCiudad,cpMensaje);
	strcpy(nuevo->cpNombreEquipo,nombreEquipo);
	memcpy(nuevo->iIdentificador,cpID,2);
	nuevo->iSocket = iSocket;
	nuevo->tiempo = tiempo;
	nuevo->ttl = ttl;
	nuevo->hops = hops;

	nuevo->siguiente = (*sListaD);
	(*sListaD)=nuevo;


	if(!strcmp(cpMensaje, cpNombreCiu))
	{
		cpMensajeEnviar = (char*) malloc(4 + strlen(nombreEquipo));
		strcpy(cpMensajeEnviar, "E?");
		cpMensajeEnviar[2] = strlen(nombreEquipo)+1;
		memcpy(&(cpMensajeEnviar[3]), nombreEquipo,strlen(nombreEquipo)+1);
		if(sendall(iSockCiudad, cpMensajeEnviar, 4 + strlen(nombreEquipo))==-1)
		{
			perror("send");
			exit(1);
		}
		free(cpMensajeEnviar);
	}
	else if(ttl>0)
	{
		fiRouEnviarDiscovers(iSocket,sListaR,ttl-1,hops +1,nuevo->cpNombreCiudad,nuevo->cpNombreEquipo,nuevo->iIdentificador);
	}
	free(cpMensaje);
	return 1;
}
/*------------------------------------------------------------------------------------------------------------*/
int fiRouManejarEncontrado(int iSocket,struct sDiscover ** sListaD,char * cpNombreCiudad)
{
	char cpIpPuerto[6];
	char * cpNombreEquipo;
	char cTam;
	struct sDiscover * lAux;

	if(recvAll(iSocket,cpIpPuerto,6,0)==-1)
	{
		perror("recvAll");
		exit(1);
	}

	if(recvAll(iSocket,&cTam,1,0)==-1)
	{
		perror("recvAll");
		exit(1);
	}

	cpNombreEquipo = (char *)malloc(cTam);

	if(recvAll(iSocket,cpNombreEquipo,cTam,0)==-1)
	{
		perror("recvAll");
		exit(1);
	}
	
	lAux = *sListaD;
	while(lAux)
	{
		if(!strcmp(lAux->cpNombreEquipo,cpNombreEquipo) && !strcmp(lAux->cpNombreCiudad,cpNombreCiudad))
			fiEnviarFound(lAux->iSocket,lAux->iIdentificador,lAux->hops/*TTL*/,0/*HOPS*/,cpNombreCiudad,cpNombreEquipo,cpIpPuerto);
		lAux = lAux->siguiente;
	}
	fiRouLimpiarLista(sListaD,cpNombreCiudad,cpNombreEquipo);
	free(cpNombreEquipo);
}
/*-----------------------------------------------------------------------------------------------------------*/
int fiRouManejarNoEncontrado(int iSocket,struct sDiscover ** sListaD, char * cpNombreCiudad)
{
	char * cpNombreEquipo;
	char cTam;
	struct sDiscover * lAux;

	if(recvAll(iSocket,&cTam,1,0)==-1)
	{
		perror("recvAll");
		exit(1);
	}
	cpNombreEquipo = (char *)malloc(cTam);

	if(recvAll(iSocket,cpNombreEquipo,cTam,0)==-1)
	{
		perror("recvAll");
		exit(1);
	}
	
	lAux = *sListaD;
	while(lAux)
	{
		if(!strcmp(lAux->cpNombreEquipo,cpNombreEquipo) && !strcmp(lAux->cpNombreCiudad,cpNombreCiudad))
			fiEnviarNotFound(lAux->iSocket,lAux->iIdentificador,lAux->hops, 0,cpNombreCiudad,cpNombreEquipo);
		lAux = lAux->siguiente;
	}
	fiRouLimpiarLista(sListaD,cpNombreCiudad,cpNombreEquipo);
	free(cpNombreEquipo);
}

/*------------------------------------------------------------------------------------------------------------------*/
int fiEnviarFound(int iSocket,char * iIdentificador,char ttl,char hops,char * cpNombreCiudad,char * cpNombreEquipo,char *cpIpPuerto)
{
	char * cpMensaje;
	int iLargo;

	cpMensaje = (char *)malloc(9 + strlen(cpNombreEquipo)+ strlen(cpNombreCiudad)+6);

	memcpy(&cpMensaje[0],iIdentificador,2);
	cpMensaje[2] = 1; /*FOUND*/
	cpMensaje[3] = ttl; 
	cpMensaje[4] = hops;
	iLargo = strlen(cpNombreEquipo) + strlen(cpNombreCiudad) + 8;
	cpMensaje[5] = iLargo%256;
	cpMensaje[6] = iLargo/256;
	memcpy(&cpMensaje[7],cpNombreCiudad,strlen(cpNombreCiudad)+1);
	memcpy(&cpMensaje[8+strlen(cpNombreCiudad)],cpNombreEquipo,strlen(cpNombreEquipo)+1);
	memcpy(&cpMensaje[9+strlen(cpNombreCiudad)+strlen(cpNombreEquipo)],cpIpPuerto,6);
	
	if(sendall(iSocket, cpMensaje,9+strlen(cpNombreCiudad)+strlen(cpNombreEquipo)+6)==-1)
	{
		perror("send");
	}
	free(cpMensaje);
}
/*------------------------------------------------------------------------------------------------------------------*/
int fiEnviarNotFound(int iSocket,char * iIdentificador,char ttl,char hops,char * cpNombreCiudad,char * cpNombreEquipo)
{
	char * cpMensaje;
	int iLargo;

	cpMensaje = (char *)malloc(9 + strlen(cpNombreEquipo)+ strlen(cpNombreCiudad));

	memcpy(&cpMensaje[0],iIdentificador,2);
	cpMensaje[2] = 2; /*NOT FOUND*/
	cpMensaje[3] = ttl;
	cpMensaje[4] = hops;
	iLargo = strlen(cpNombreEquipo) + strlen(cpNombreCiudad) + 2;
	cpMensaje[5] = iLargo%256;
	cpMensaje[6] = iLargo/256;
	memcpy(&cpMensaje[7],cpNombreCiudad,strlen(cpNombreCiudad)+1);
	memcpy(&cpMensaje[8+strlen(cpNombreCiudad)],cpNombreEquipo,strlen(cpNombreEquipo)+1);
	
	if(sendall(iSocket, cpMensaje,9+strlen(cpNombreCiudad)+strlen(cpNombreEquipo))==-1)
	{
		perror("send");
	}
	free(cpMensaje);
}
/*----------------------------------------------------------------------------------------------------------------*/
void fvRouManejarFound(int iSocket,struct sDiscover ** sListaD, char * cpNombreCiu,int iSockCiudad,char * iIdMensaje,char ttl, char hops, int iLargoMensaje)
{
	char * cpNombreEquipo;
	char * cpNombreCiudad;
	char * cpMensaje;
	char cpIpPuerto[6];
	struct sDiscover * lAux;

	cpMensaje = (char *)malloc(iLargoMensaje);

	if(recvAll(iSocket,cpMensaje,iLargoMensaje,0)==-1)
	{
		perror("recvAll");
		exit(1);
	}
	cpNombreCiudad = cpMensaje;
	cpNombreEquipo = &(cpMensaje[strlen(cpMensaje)+1]);
	memcpy(&cpIpPuerto[0], &(cpNombreEquipo[strlen(cpNombreEquipo)+1]),6);

	lAux = * sListaD;

	while(lAux)
	{
		if(!strcmp(lAux->cpNombreEquipo,cpNombreEquipo) && !strcmp(lAux->cpNombreCiudad,cpNombreCiudad))
		{
			if(!lAux->iSocket /* || !ttl */)
				fiRouEnviarFoundCiudad(iSockCiudad,cpNombreCiudad,cpNombreEquipo,cpIpPuerto);
		        else if (ttl)
				fiEnviarFound(lAux->iSocket,lAux->iIdentificador,lAux->hops/*TTL*/,lAux->ttl/*HOPS*/,cpNombreCiudad,cpNombreEquipo,cpIpPuerto);
		}
		lAux = lAux->siguiente;
	}
	fiRouLimpiarLista(sListaD,cpNombreCiudad,cpNombreEquipo);
	free(cpMensaje);
}
/*-------------------------------------------------------------------------------------------------------------*/
void fvRouManejarNotFound(int iSocket,struct sDiscover ** sListaD, char * cpNombreCiu,int iSockCiudad,char * iIdMensaje,char ttl, char hops, int iLargoMensaje)
{
	char * cpNombreEquipo;
	char * cpNombreCiudad;
	char * cpMensaje;
	struct sDiscover * lAux;

	cpMensaje = (char *)malloc(iLargoMensaje);

	if(recvAll(iSocket,cpMensaje,iLargoMensaje,0)==-1)
	{
		perror("recvAll");
		exit(1);
	}
	cpNombreCiudad = (char *)malloc(strlen(cpMensaje)+1);
	cpNombreEquipo = (char *)malloc(iLargoMensaje-(strlen(cpMensaje)+1));
	strcpy(cpNombreCiudad,cpMensaje);
	strcpy(cpNombreEquipo,&(cpMensaje[(strlen(cpMensaje)+1)]));

	lAux = * sListaD;

	while(lAux)
	{
		if(!strcmp(lAux->cpNombreEquipo,cpNombreEquipo) && !strcmp(lAux->cpNombreCiudad,cpNombreCiudad))
		{
			if(!(lAux->iSocket))
				fiRouEnviarNotFoundCiudad(iSockCiudad,cpNombreCiudad,cpNombreEquipo);
		        else if (ttl)
				fiEnviarNotFound(lAux->iSocket,lAux->iIdentificador,lAux->hops/*TTL*/,lAux->ttl/*HOPS*/,cpNombreCiudad,cpNombreEquipo);
		}
		lAux = lAux->siguiente;
	}
	fiRouLimpiarLista(sListaD,cpNombreCiudad,cpNombreEquipo);

	free(cpMensaje);
	free(cpNombreEquipo);
	free(cpNombreCiudad);
}
/*---------------------------------------------------------------------------------------------------------------*/
int fiRouEnviarFoundCiudad(int iSockCiudad,char *cpNombreCiudad,char *cpNombreEquipo,char *cpIpPuerto)
{
	char * cpMensaje;

	cpMensaje = (char *)malloc(3+strlen(cpNombreCiudad)+2+strlen(cpNombreEquipo)+7);
	strcpy(cpMensaje,"FO");
	cpMensaje[2]=strlen(cpNombreCiudad)+1;
	memcpy(&cpMensaje[3],cpNombreCiudad,strlen(cpNombreCiudad)+1);
	cpMensaje[4+strlen(cpNombreCiudad)]=strlen(cpNombreEquipo)+1;
	memcpy(&cpMensaje[5+strlen(cpNombreCiudad)],cpNombreEquipo,strlen(cpNombreEquipo)+1);
	memcpy(&cpMensaje[6+strlen(cpNombreCiudad)+strlen(cpNombreEquipo)],cpIpPuerto,6);

	if(sendall(iSockCiudad,cpMensaje,12+strlen(cpNombreEquipo)+strlen(cpNombreCiudad))==-1)
	{		
		perror("send");
		exit(1);
	}
	free(cpMensaje);
			
}
/*---------------------------------------------------------------------------------------------------------------*/
int fiRouEnviarNotFoundCiudad(int iSockCiudad,char *cpNombreCiudad,char *cpNombreEquipo)
{
	char * cpMensaje;

	cpMensaje = (char *)malloc(3+strlen(cpNombreCiudad)+2+strlen(cpNombreEquipo)+1);
	strcpy(cpMensaje,"NF");
	cpMensaje[2]=strlen(cpNombreCiudad)+1;
	memcpy(&cpMensaje[3],cpNombreCiudad,strlen(cpNombreCiudad)+1);
	cpMensaje[4+strlen(cpNombreCiudad)]=strlen(cpNombreEquipo)+1;
	memcpy(&cpMensaje[5+strlen(cpNombreCiudad)],cpNombreEquipo,strlen(cpNombreEquipo)+1);

	if(sendall(iSockCiudad,cpMensaje,6+strlen(cpNombreEquipo)+strlen(cpNombreCiudad))==-1)
	{		
		perror("send");
		exit(1);

	}
	free(cpMensaje);
}
/*------------------------------------------------------------------------------------------------------------*/

int fiRouLimpiarLista(struct sDiscover ** sListaD,char *cpNombreCiudad,char *cpNombreEquipo)
{
	struct sDiscover * sAux;
	struct sDiscover * sAux2;

	while (((*sListaD) != NULL) && (!strcmp(cpNombreCiudad, (*sListaD)->cpNombreCiudad)) && (!strcmp(cpNombreEquipo, (*sListaD)->cpNombreEquipo)))
	{
		sAux = (*sListaD);
		(*sListaD) = sAux->siguiente;
		free(sAux->cpNombreCiudad);
		free(sAux->cpNombreEquipo);
		free(sAux);
	}
	if((*sListaD) != NULL)
	{
		sAux = (*sListaD);
		sAux2 = sAux->siguiente;
		while(sAux2!=NULL)
		{
			if((!strcmp(cpNombreCiudad, sAux2->cpNombreCiudad)) && (!strcmp(cpNombreEquipo, sAux2->cpNombreEquipo)))
			{
				sAux->siguiente = sAux2->siguiente;
				free(sAux2->cpNombreCiudad);
				free(sAux2->cpNombreEquipo);
				free(sAux2);
				sAux2 = sAux->siguiente;
			}
			else
			{
				sAux=sAux2;
				sAux2=sAux2->siguiente;
			}
		}
	}
}

int fiRouLimpiarListaTiempo(int iSocket,struct sDiscover ** sListaD, int tiempoBorrar)
{
	struct sDiscover * sAux;
	struct sDiscover * sAux2;
	time_t tiempo;

	time(&tiempo);

	while (((*sListaD) != NULL) && ((tiempo - (*sListaD)->tiempo) >= tiempoBorrar))
	{
		if(!(*sListaD)->iSocket)
			fiRouEnviarNotFoundCiudad(iSocket,(*sListaD)->cpNombreCiudad,(*sListaD)->cpNombreEquipo);
		else
			fiEnviarNotFound((*sListaD)->iSocket,(*sListaD)->iIdentificador,(*sListaD)->hops, 0,(*sListaD)->cpNombreCiudad,(*sListaD)->cpNombreEquipo);

		sAux = (*sListaD);
		(*sListaD) = sAux->siguiente;
		free(sAux->cpNombreCiudad);
		free(sAux->cpNombreEquipo);
		free(sAux);
	}
	if((*sListaD) != NULL)
	{
		sAux = (*sListaD);
		sAux2 = sAux->siguiente;
		while(sAux2!=NULL)
		{
			if(((tiempo - sAux2->tiempo) >= tiempoBorrar))
			{
					if(!sAux2->iSocket)
							fiRouEnviarNotFoundCiudad(iSocket,sAux2->cpNombreCiudad,sAux2->cpNombreEquipo);
					else
							fiEnviarNotFound(sAux2->iSocket,sAux2->iIdentificador,sAux2->hops, 0,sAux2->cpNombreCiudad,sAux2->cpNombreEquipo);

					sAux->siguiente = sAux2->siguiente;
					free(sAux2->cpNombreCiudad);
					free(sAux2->cpNombreEquipo);
					free(sAux2);
					sAux2 = sAux->siguiente;
			}
			else
			{
				sAux=sAux2;
				sAux2=sAux2->siguiente;
			}
		}
	}
}

void fvRouManejarDatos(int iSocket, struct sRouter * sListaR)
{
	struct sRouter * sBuscar;
	sBuscar = fiRouBuscarRouterSocket(sListaR, iSocket);
	if(recvAll(iSocket, sBuscar->cpIpPuerto, 6, 0)==-1)
	{
		perror("recvAll");
		exit(1);
	}	
	sBuscar->iTengoDatos = 1;
}

void fvEnviarDatos(int iSocket, char * cpIpPuerto)
{
		char cpMensaje[13];
		
		memset(&(cpMensaje[0]), 0, 2);
	cpMensaje[2] = MSDatos; 	
	cpMensaje[3] = 0;
	cpMensaje[4] = 0;
	cpMensaje[5] = 6;
	cpMensaje[6] = 0;
	memcpy(&(cpMensaje[7]), cpIpPuerto, 6);

	if(send(iSocket, cpMensaje, 13, 0)==-1)
	{
		perror("send");
		exit(1);
	}
}

int fiRouSacarRouterLista(struct sRouter ** sListaR,int iSocket)
{
	struct sRouter* sAux;
	struct sRouter* sAux2;

	if(((*sListaR) != NULL) && (iSocket == (*sListaR)->iSocket))
	{
		sAux = (*sListaR);
		(*sListaR) = sAux->siguiente;
		free(sAux);
	}
	else if((*sListaR) != NULL)
	{
		sAux = (*sListaR);
		sAux2 = sAux->siguiente;
		while(sAux2!=NULL)
		{
			if(iSocket == sAux2->iSocket)
			{
				sAux->siguiente = sAux2->siguiente;
				free(sAux2);
				sAux2 = sAux->siguiente;
			}
			else
			{
				sAux=sAux2;
				sAux2=sAux2->siguiente;
			}
		}
	}
}

int sendall(int s, char *buf, int len)
{
	int total = 0;        // cuántos bytes hemos enviado
	int bytesleft = len; // cuántos se han quedado pendientes
	int n;

	while(total < len)
	{
		n = send(s, buf+total, bytesleft, 0);
		if (n == -1)
		{
			break;
		}
		total += n;
		bytesleft -= n;
	}

	return (len - total); // devuelve -1 si hay fallo, 0 en otro caso
}


int fiRouCompletarToken(struct sCiudad ** sListaC, struct sRouter * sListaR)
{
		struct sCiudad * sAux;
		struct sCiudad * sCiuNueva;
		int iOrden = 0;
		int iEncontrado = 0;
		sAux = *sListaC;

		while(sAux)
		{
				iOrden = (iOrden < sAux->orden)?sAux->orden:iOrden;
				sAux = sAux->siguiente;
		}

		while(sListaR)
		{
				if(sListaR->iTengoDatos)
				{
						sAux = *sListaC;
						iEncontrado = 0;
						while(sAux && !iEncontrado)
						{
								if(compararVector(sAux->ipPuerto, sListaR->cpIpPuerto, 6))
										iEncontrado = 1;
								sAux = sAux->siguiente;
						}
						if(!iEncontrado)
						{
								sCiuNueva = (struct sCiudad *) malloc(sizeof(struct sCiudad));
								sCiuNueva->orden = ++iOrden;
								sCiuNueva->nombre = (char*) malloc(1);
								strcpy(sCiuNueva->nombre, "");
								memcpy(sCiuNueva->ipPuerto, sListaR->cpIpPuerto, 6);
								sCiuNueva->siguiente = (*sListaC);
								(*sListaC) = sCiuNueva;
						}
				}
				sListaR = sListaR->siguiente;
		}	
}

fiRouScanfIP(char * ipPuerto, char * ip, int puerto)
{
	int ip1;
	int ip2;
	int ip3;
	int ip4;
	
	sscanf(ip, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
	ipPuerto[0]=(unsigned char)ip1;
	ipPuerto[1]=(unsigned char)ip2;
	ipPuerto[2]=(unsigned char)ip3;
	ipPuerto[3]=(unsigned char)ip4;

	ipPuerto[4] = (unsigned char)(puerto % 256);
	ipPuerto[5] = (unsigned char)(puerto / 256);
	
}
int fiRouPrintfIP(char * ipPuerto, char * ip, int * puerto)
{
	char str[4];
	strcpy(ip, itoa((unsigned char)ipPuerto[0], &(str[0])));
	strcat(ip, ".");
	strcat(ip, itoa((unsigned char)ipPuerto[1], &(str[0])));
	strcat(ip, ".");
	strcat(ip, itoa((unsigned char)ipPuerto[2], &(str[0])));
	strcat(ip, ".");
	strcat(ip, itoa((unsigned char)ipPuerto[3], &(str[0])));
	
	(*puerto)= (int)((unsigned char) ipPuerto[4]) + (int)(((unsigned char) ipPuerto[5]) * 256);

}

char * itoa(unsigned char i, char * str)
{
	int j;
	j = i;
	sprintf(str, "%d", j);
	return str;
}

int recvAll(int sock, char * buf, int tam, int juanmanuel)
{
	int recibido = 0;
	int recAux;
	while(recibido<tam)
	{
		recAux = recv(sock, &(buf[recibido]), tam - recibido, 0);
		if(recAux == -1 || recAux == 0)
			return recAux;
		recibido = recibido + recAux;
	}
	return recibido;
}
