/*
 * =====================================================================================
 *        Filename:  ciudadBib.c
 * =====================================================================================
 */

#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<signal.h>

#include "ciudadBib.h"
#include "logs.h"
#include "configuracion.h"

int fiCiuAgregarEquipoInicial (struct sEquipo **lsEquipos, int socket)
{
	struct sEquipo *sNuevoEquipo;

	if ((sNuevoEquipo = (struct sEquipo *) malloc (sizeof (struct sEquipo))) == NULL)
	{
		fvStdLog("ciudadBib.c",27,TLError,"Error de Malloc");
		exit (0);
	}
	sNuevoEquipo->nombre = NULL;
	sNuevoEquipo->iTieneLev = 0;
	sNuevoEquipo->cola = NULL;
	sNuevoEquipo->estado = ESTADOvacio;
	sNuevoEquipo->socket = socket;
	sNuevoEquipo->siguiente = *lsEquipos;
	*lsEquipos = sNuevoEquipo;
	fvStdLog("ciudadBib.c",37,TLMensaje,"Agregado equipo vacio a la lista");
	return 1;
}

int fiCiuAgregarDatosEquipo (struct sEquipo **lsEquipos, char *cpNombre, int *ipIpPuerto, int iSocket,fd_set * maestro)
{
	struct sEquipo *sEquipo;
	int i;
	if ((sEquipo = fiCiuBuscarEquipoNombre(*lsEquipos, cpNombre)) != NULL)
	{
		fiCiuSacarEquipoLista(iSocket, lsEquipos);
		sEquipo->socket = iSocket;
		sEquipo->iTieneLev=1;
//		if(sEquipo->estado == ESTADOdesconectado)
//		{
//				fiCiuSacarEquipoLista(iSocket, lsEquipos);
//				fvStdLog("ciudadBib.c",52,TLError,"Borrando Equipo que no Volvio  =(");
//				close(iSocket);
//				FD_CLR(iSocket,maestro);
//		}
	}
	else
	{
		if ((sEquipo = fiCiuBuscarEquipoSocket(*lsEquipos, iSocket)) == NULL)
		{
			return (0);
		}
		if ((sEquipo->nombre = (char *) malloc (strlen (cpNombre)+1)) == NULL)
		{
			fvStdLog("ciudadBib.c",65,TLError,"Error de Malloc" );
			exit (0);
		}
		for (i = 0; i < 6; i++)
			sEquipo->ipPuerto[i] = ipIpPuerto[i];
		strcpy (sEquipo->nombre, cpNombre);
		sEquipo->estado = ESTADOlibre;
	}
	fvStdLog("ciudadBib.c",73,TLMensaje,"Agregado Datos Equipo %s", cpNombre);
	return (1);
}

struct sEquipo * fiCiuBuscarEquipoNombre (struct sEquipo *lista, char *cpNombre)
{
	while (lista)
	{
		if((lista->estado != ESTADOvacio))
		{
			if (!strcmp (lista->nombre, cpNombre))
				return lista;
		}
		lista = lista->siguiente;

	}
	return lista;
}

struct sEquipo * fiCiuBuscarEquipoSocket (struct sEquipo *lista, int iSocket)
{
	while (lista)
	{
		if (iSocket == lista->socket)
			return lista;
		lista = lista->siguiente;
	}
	return lista;
}

int fiCiuRecibirCabeceraEquipo (int iSocket)
{
	char cpBuf[3];
	int iTam;

	if((iTam = recvAll (iSocket, cpBuf, 2, 0))<0)
	{
		fvStdLog("ciudadBib.c",124,TLError,"error al recibir cabecera del equipo");
		return MSDesconectado;
	}


	if (iTam == 0)
	{
		fvStdLog("ciudadBib.c",131,TLError,"REcibida cabecera vacia");
		return (MSDesconectado);
	}
	cpBuf[2] = '\0';
	fvStdLog("ciudadBib.c",135,TLMensaje,"Recibida cabecera %s de un equipo", cpBuf);

	if (!(strncmp (cpBuf, "LI", 2))) /*LEV INTERNA - LI|CANT|RES|LARGO|NOMBRE|...*/
		return (MSLEVI);

	if (!(strncmp (cpBuf, "DE", 2))) /*DATOS EQUIPO - DE|IP|PUERTO|LARGO|NOMBRE*/
		return (MSDatosEquipo);

	if (!(strncmp (cpBuf, "PJ", 2))) /*PEDIDO PARTIDO - PJ|LARGO|NOMBRE*/
		return (MSPedidoPartido);

	if (!(strncmp (cpBuf, "LB", 2))) /*LIBRE LB*/
		return (MSLibre);

	if (!(strncmp (cpBuf, "PE", 2))) /*PEDIDO ENTRENAMIENTO PE*/
		return (MSPedidoEntrenamiento);

	if (!(strncmp (cpBuf, "OC", 2))) /*OCUPADO*/
		return (MSOcupado);

	if (!(strncmp (cpBuf, "MG", 2))) /*DATOS MIGRACION*/
		return (MSMigracion);
	
	if (!(strncmp (cpBuf, "LE", 2))) /*LEV EXTERNA*/
		return (MSLEVE);

	if (!(strncmp (cpBuf, "PM", 2))) /*PEDIDO DE DATOS MIGRACION*/
		return (MSPedidoDtMigracion);
	
	return (-1);
}

int fiCiuCompleta(struct sEquipo * lista)
{
	while(lista)
	{
		if(lista->iTieneLev)
		{
			return 0;
		}
		lista = lista->siguiente;
	}
	return 1;
}

fiCiuProcesarLEV (struct sTDP *tabla, int iSocket, char *cpEquipoLocal, char * cpCiudadLocal)
{
	char cCantidad, iLargoRecibido;
	unsigned char cRdo;
	unsigned char cLargo;
	char * cpNombreCiudad;
	char *cpEquipoVisitante;

	if ((iLargoRecibido = recvAll (iSocket, &cCantidad, 1, 0)) < 1)
	{
		return (iLargoRecibido);
	}
	while (cCantidad--)
	{
		if ((iLargoRecibido=recvAll(iSocket, &cRdo, 1, 0)) < 1)
		{
			return (iLargoRecibido);
		}
		if ((iLargoRecibido=recvAll(iSocket,&cLargo, 1, 0)) < 1)
		{
			return (iLargoRecibido);
		}
		if((cpNombreCiudad = (char *)malloc(cLargo)) == NULL)
		{
			fvStdLog("ciudadBib.c",204,TLError,"Error de Malloc");
			exit (0);
		}
		if((iLargoRecibido=recvAll(iSocket, cpNombreCiudad, cLargo, 0)) < 1)
		{
			return (iLargoRecibido);
		}

		if ((iLargoRecibido=recvAll(iSocket,&cLargo, 1, 0)) < 1)
		{
			return (iLargoRecibido);
		}
		if((cpEquipoVisitante = (char *)malloc(cLargo)) == NULL)
		{
			fvStdLog("ciudadBib.c",218,TLError,"Error de Malloc");
			exit (0);
		}
		if((iLargoRecibido=recvAll(iSocket, cpEquipoVisitante, cLargo, 0)) < 1)
		{
			return (iLargoRecibido);
		}
		fvCiuAnotarPartidos (tabla,cpCiudadLocal,cpEquipoLocal,cpNombreCiudad,cpEquipoVisitante, cRdo);
		free (cpEquipoVisitante);
		free(cpNombreCiudad);
	}
	return 1;
}
void fvCiuAnotarPartidos (struct sTDP * tabla,char * cpCiudadLocal, char *cpEquipoLocal, char * cpCiudadVisitante,char *cpEquipoVisitante, unsigned char cRdo)
{
	fvStdLog("ciudadBib.c",233,TLMensaje,"Resultado Partido: %s - %s : %c", cpEquipoLocal, cpEquipoVisitante, cRdo);
	while (tabla)
	{
		if ((!strcmp (tabla->nombre, cpEquipoLocal)) && (!strcmp (tabla->ciudad, cpCiudadLocal)))
		{
			switch (cRdo)
			{
				case 'G':
					(tabla->pg)++;
					(tabla->puntos) += 3;
					break;
				case 'P':
					(tabla->pp)++;
					break;
				case 'E':
					(tabla->pe)++;
					(tabla->puntos)++;
					break;
			}			/* -----  end switch  ----- */
		}
		if((!strcmp (tabla->nombre, cpEquipoVisitante))&&(!strcmp (tabla->ciudad, cpCiudadVisitante)))
		{
			switch (cRdo)
			{
				case 'P':
					(tabla->pg)++;
					(tabla->puntos) += 3;
					break;
				case 'G':
					(tabla->pp)++;
					break;
				case 'E':
					(tabla->pe)++;
					(tabla->puntos)++;
					break;
			}			/* -----  end switch  ----- */
		}
		tabla = tabla->siguiente;
	}
}

void fvCiuRecibirDatosEquipo (int iSocket, struct sEquipo **lista, fd_set * maestro)
{
	char cpBufIpPuerto[6];
	char *cpBufNombre;
	int iLargoRecibido;
	char cpBufTamanio;

	if ((iLargoRecibido = recvAll (iSocket, &cpBufTamanio, 1, 0)) < 1)
	{
		return /*(iLargoRecibido) */ ;
	}

	if ((cpBufNombre = (char *) malloc (cpBufTamanio)) == NULL)
	{
		fvStdLog("ciudadBib.c",288,TLError,"Error de Malloc");
		exit (0);
	}

	if ((iLargoRecibido = recvAll (iSocket, cpBufNombre, cpBufTamanio, 0)) < 1)
	{
		return /*(iLargoRecibido) */ ;
	}

	if ((iLargoRecibido = recvAll (iSocket, cpBufIpPuerto, 6, 0)) < 1)
	{
		return /*(iLargoRecibido) */ ;
	}
	
	fiCiuAgregarDatosEquipo (lista, cpBufNombre, (int *) cpBufIpPuerto, iSocket, maestro);
	free(cpBufNombre);
}


int fiCiuManejarPedidoPermiso (int iSock, struct sEquipo  *lista, char cTipoLev, int iSockRouter, struct sDiscover ** listaDiscover, char * cpMiNombre)
{
	char cLargo;
	int iLargoRecibido;
	char *cpNombre;
	char * cpNombreCiudad;
	char *cpMensajeRouter;
	struct sDiscover * nodoNuevo;
	struct sEquipo * lAux1;

	if ((iLargoRecibido = recvAll (iSock, &cLargo, 1, 0)) < 1)
	{
		return (iLargoRecibido);
	}

	if ((cpNombreCiudad = (char *)malloc(cLargo)) == NULL)
	{
		fvStdLog("ciudadBib.c",322,TLError,"Error de Malloc");
		exit (0);
	}
	
	if ((iLargoRecibido = recvAll(iSock, cpNombreCiudad, cLargo, 0)) < 1)
	{
		return (iLargoRecibido);
	}
	
	if ((iLargoRecibido = recvAll (iSock, &cLargo, 1, 0)) < 1)
	{
		return (iLargoRecibido);
	}

	if ((cpNombre = (char *)malloc(cLargo)) == NULL)
	{
		fvStdLog("ciudadBib.c",338,TLError,"Error de Malloc");
		exit (0);
	}
	if ((iLargoRecibido = recvAll(iSock, cpNombre, cLargo, 0)) < 1)
	{
		return (iLargoRecibido);
	}

	fvStdLog("ciudadBib.c",346,TLMensaje,"Recibido pedido de permiso para jugar con: %s" , cpNombre);
	

	if (strcmp(cpMiNombre, cpNombreCiudad))
	{
		fvStdLog("ciudadBib.c",351,TLMensaje,"Pidiendo al Router que busque al equipo: %s", cpNombre);
		cpMensajeRouter = (char *)malloc(strlen(cpNombre) + strlen(cpNombreCiudad)+ 6);
		strcpy(cpMensajeRouter, "PB");
		cpMensajeRouter[2] = strlen(cpNombreCiudad) + 1;
		memcpy(&(cpMensajeRouter[3]), cpNombreCiudad, strlen(cpNombreCiudad) +1);
		cpMensajeRouter[4 + strlen(cpNombreCiudad)] = strlen(cpNombre) + 1;
		memcpy(&(cpMensajeRouter[5 + strlen(cpNombreCiudad)]), cpNombre, strlen(cpNombre) +1);
		if (sendall (iSockRouter, cpMensajeRouter, strlen(cpNombreCiudad) + strlen(cpNombre) + 6, 0) == -1)
		{
			fvStdLog("ciudadBib.c",360,TLError,"Error al enviar mensaje PB");
			exit (0);
		}
		nodoNuevo = (struct sDiscover *) malloc(sizeof(struct sDiscover));
		nodoNuevo->nombre = (char*) malloc(strlen(cpNombre)+1);
		nodoNuevo->nombreCiudad = (char*) malloc(strlen(cpNombreCiudad)+1);
		strcpy(nodoNuevo->nombre, cpNombre);
		strcpy(nodoNuevo->nombreCiudad, cpNombreCiudad);
		nodoNuevo->iSocket = iSock;
		nodoNuevo->siguiente = (*listaDiscover);
		(*listaDiscover) = nodoNuevo;
		free(cpMensajeRouter);
	}
	else
	{
		lAux1 = lista;
		lista = fiCiuBuscarEquipoNombre(lAux1, cpNombre);
		lAux1 = fiCiuBuscarEquipoSocket(lAux1, iSock);
		if(cTipoLev != 'N' && lAux1->estado == ESTADOvacio)
		{
			fvStdLog("ciudadBib.c",377,TLMensaje,"Enviado Terminator a equipo migrado");
			if (sendall (iSock, "T2", 2, 0) == -1)
			{
				fvStdLog("ciudadBib.c",380,TLError,"Error al enviar mensaje T2 a equipo migrado");
				exit (0);
			}
		}
		else if(lista!= NULL)
		{
			fvStdLog("ciudadBib.c",377,TLMensaje,"Enviado pedido de permiso a: %s", cpNombre);
			if (sendall (lista->socket, "PJ", 2, 0) == -1)
			{
				fvStdLog("ciudadBib.c",380,TLError,"Error al enviar mensaje PJ a %s", lista->nombre);
				exit (0);
			}
			fvCIUAgregarFIFO (&(lista->cola), iSock);
		}
		else
		{
			fvStdLog("ciudadBib.c",387,TLMensaje,"No se encontro al equipo: %s", cpNombre);
			if (sendall (iSock, "NJ", 2, 0) == -1)
			{
				fvStdLog("ciudadBib.c",390,TLError,"Error al enviar mensaje NJ");
				exit (0);
			}
		}

	}
	free(cpNombreCiudad);
	free(cpNombre);
	return 1;
}

void fvCIUAgregarFIFO (struct sEquipoCola **cola, int iSocket)
{
	struct sEquipoCola *equipoNuevo;
	struct sEquipoCola *recorrer;

	equipoNuevo = (struct sEquipoCola *) malloc(sizeof(struct sEquipoCola));

	equipoNuevo->socket = iSocket;
	equipoNuevo->siguiente = NULL;

	if (*cola == NULL)
	{
		*cola = equipoNuevo;
	}
	else
	{
		recorrer = *cola;
		while (recorrer->siguiente != NULL)
		{
			recorrer = recorrer->siguiente;
		}
		recorrer->siguiente = equipoNuevo;
	}
}

int fiCiuManejarPedidoEntrenamiento(int iSock, char * mensajePE)
{
	if (sendall (iSock, mensajePE, 8, 0) == -1)
	{
		fvStdLog("ciudadBib.c",428,TLError,"Error al enviar respuesta de pedido de entrenamiento");
		exit (1);
	}	
	fvStdLog("ciudadBib.c",431,TLMensaje,"Enviada respuesta al pedido de entrenamiento");
	return 1;
}

int fiCiuEnviarLEVInternaEquipo(int iSock, char * nombreEquipo, char * nombreCiudad, struct sTDP* sTabla, struct sEquipo * lista)
{
	char * cpMensaje;
	struct sTDP * sTablaRecorrer;
	struct sEquipo * sEquipoAux;
	int iTamanio;
	int iPosicion;
	int iCantidad;

	iTamanio = 3;
	iCantidad = 0;
	sTablaRecorrer = sTabla;

	sEquipoAux = fiCiuBuscarEquipoSocket(lista, iSock);
	if((sEquipoAux != NULL )&& (sEquipoAux->estado != ESTADOvacio ))
	{
		while(sTablaRecorrer != NULL)
		{
			if (!strcmp(sTablaRecorrer->ciudad, nombreCiudad) && strcmp(sTablaRecorrer->nombre, nombreEquipo))
			{
				iTamanio += 2 + strlen(sTablaRecorrer->nombre) + 1;
				iTamanio += 2 + strlen(nombreCiudad) ;
				iCantidad++;
			}
			sTablaRecorrer = sTablaRecorrer->siguiente;
		}

		if(iCantidad == 0) 
			return(0);

		cpMensaje	= (char*)malloc (iTamanio);
		if ( cpMensaje==NULL )
		{
			fvStdLog("ciudadBib.c",468,TLError,"Error de Malloc");
			exit (EXIT_FAILURE);
		}

		sTablaRecorrer = sTabla;
		strcpy(cpMensaje, "LI");
		cpMensaje[2]= iCantidad;
		iPosicion = 3;

		while(sTablaRecorrer != NULL)
		{
			if (!strcmp(sTablaRecorrer->ciudad, nombreCiudad) && strcmp(sTablaRecorrer->nombre, nombreEquipo))
			{
				cpMensaje[iPosicion++] = 0;
				cpMensaje[iPosicion++] = strlen(nombreCiudad) + 1;
				memcpy(&(cpMensaje[iPosicion]), sTablaRecorrer->ciudad, strlen(sTablaRecorrer->ciudad) + 1);
				iPosicion +=  strlen(nombreCiudad) + 1;
				cpMensaje[iPosicion++] = strlen(sTablaRecorrer->nombre) + 1;
				memcpy(&(cpMensaje[iPosicion]), sTablaRecorrer->nombre, strlen(sTablaRecorrer->nombre) + 1);
				iPosicion +=  strlen(sTablaRecorrer->nombre) + 1;
			}
			sTablaRecorrer = sTablaRecorrer->siguiente;
		}
		if (sendall (iSock, cpMensaje, iTamanio, 0) == -1)
		{
			fvStdLog("ciudadBib.c",493,TLError,"Error al enviar Lev interna al equipo %s",nombreEquipo);
			free (cpMensaje);
			return(0);
		}
		fvStdLog("ciudadBib.c",496,TLMensaje,"enviada LEV interna al equipo %s", nombreEquipo);
		free (cpMensaje);
		return(1);
	}
	return(0);
}

int fiCiuEnviarLEVExternaEquipo(int iSock, char * nombreEquipo, char * nombreCiudad, struct sTDP* sTabla, struct sEquipo * lista)
{
	char * cpMensaje;
	struct sTDP * sTablaRecorrer;
	struct sEquipo * sEquipoAux;
	int iTamanio;
	int iPosicion;
	int iCantidad;

	iTamanio = 3;
	iCantidad = 0;
	sTablaRecorrer = sTabla;

	sEquipoAux = fiCiuBuscarEquipoSocket(lista, iSock);
	if((sEquipoAux != NULL) && (sEquipoAux->estado != ESTADOvacio))
	{
		while(sTablaRecorrer != NULL)
		{
			if (strcmp(sTablaRecorrer->ciudad, nombreCiudad) && strcmp(sTablaRecorrer->nombre, nombreEquipo))
			{
				iTamanio += 2 + strlen(sTablaRecorrer->nombre) + 1;
				iTamanio += 2 + strlen(sTablaRecorrer->ciudad);
				iCantidad++;
			}
			sTablaRecorrer = sTablaRecorrer->siguiente;
		}

		if(iCantidad == 0) 
			return(0);

		cpMensaje	= (char*)malloc (iTamanio);
		if ( cpMensaje==NULL )
		{
			fvStdLog("ciudadBib.c",536,TLError,"Error de Malloc");
			exit (EXIT_FAILURE);
		}

		sTablaRecorrer = sTabla;
		strcpy(cpMensaje, "LE");
		cpMensaje[2]= iCantidad;
		iPosicion = 3;

		while(sTablaRecorrer != NULL)
		{
			if (strcmp(sTablaRecorrer->ciudad, nombreCiudad) && strcmp(sTablaRecorrer->nombre, nombreEquipo))
			{
				cpMensaje[iPosicion++] = 0;
				cpMensaje[iPosicion++] = strlen(sTablaRecorrer->ciudad) + 1;
				memcpy(&(cpMensaje[iPosicion]), sTablaRecorrer->ciudad, strlen(sTablaRecorrer->ciudad) + 1);
				iPosicion +=  strlen(sTablaRecorrer->ciudad) + 1;
				cpMensaje[iPosicion++] = strlen(sTablaRecorrer->nombre) + 1;
				memcpy(&(cpMensaje[iPosicion]), sTablaRecorrer->nombre, strlen(sTablaRecorrer->nombre) + 1);
				iPosicion +=  strlen(sTablaRecorrer->nombre) + 1;
			}
			sTablaRecorrer = sTablaRecorrer->siguiente;
		}
		if (sendall (iSock, cpMensaje, iTamanio, 0) == -1)
		{
			fvStdLog("ciudadBib.c",561,TLError,"Error al enviar Lev externa al equipo %s",nombreEquipo);
			free (cpMensaje);
			return(0);
		}
		fvStdLog("ciudadBib.c",564,TLMensaje,"enviada LEV externa al equipo %s", nombreEquipo);
		free (cpMensaje);
		return(1);
	}
	return(0);
}


int fiCiuManejarMensajeFound(int iSockRouter, struct sDiscover ** sListaDiscovers, char cTipo)
{
	int iMensaje;
	char * cpNombre;
	char * cpNombreCiudad;
	char * cpMensaje;
	char ipPuertoCiudad[6];
	char cTamanio;
	struct sDiscover * sAux;
	struct sDiscover * sAux2;
	struct sDiscover * sAuxxx;
	struct sDiscover * sAnt;
	
	if(cTipo == 'F')
	{
		if(recvAll(iSockRouter, &cTamanio, 1, 0)==-1)
		{
			fvStdLog("ciudadBib.c",588,TLError,"Error al recibir tamanio desde el Router");
			exit(0);
		}
		cpNombreCiudad = (char*)malloc(cTamanio);
		if(recvAll(iSockRouter, cpNombreCiudad, cTamanio, 0)==-1)
		{
			fvStdLog("ciudadBib.c",594,TLError,"Error al recibir nombre desde el Router");
			exit(0);
		}
		if(recvAll(iSockRouter, &cTamanio, 1, 0)==-1)
		{
			fvStdLog("ciudadBib.c",599,TLError,"Error al recibir tamanio desde el Router");
			exit(0);
		}
		cpNombre = (char*)malloc(cTamanio);
		if(recvAll(iSockRouter, cpNombre, cTamanio, 0)==-1)
		{
			fvStdLog("ciudadBib.c",605,TLError,"Error al recibir nombre desde el Router");
			exit(0);
		}
		if(recvAll(iSockRouter, ipPuertoCiudad, 6, 0)==-1)
		{
			fvStdLog("ciudadBib.c",610,TLError,"Error al recibir IP-PUERTO");
			exit(0);
		}
		cpMensaje =(char*)malloc(8);
		strcpy(cpMensaje, "MJ");
		memcpy(&(cpMensaje[2]), ipPuertoCiudad, 6);
		iMensaje = 8;
	}
	else
	{	
		if(recvAll(iSockRouter, &cTamanio, 1, 0)==-1)
		{
			fvStdLog("ciudadBib.c",622,TLError,"Error al recibir tamanio desde el Router");
			exit(0);
		}
		cpNombreCiudad = (char*)malloc(cTamanio);
		if(recvAll(iSockRouter, cpNombreCiudad, cTamanio, 0)==-1)
		{
			fvStdLog("ciudadBib.c",628,TLError,"Error al recibir nombre desde el Router");
			exit(0);
		}
		if(recvAll(iSockRouter, &cTamanio, 1, 0)==-1)
		{
			fvStdLog("ciudadBib.c",633,TLError,"Error al recibir tamanio desde el Router");
			exit(0);
		}
		cpNombre = (char*)malloc(cTamanio);
		if(recvAll(iSockRouter, cpNombre, cTamanio, 0)==-1)
		{
			fvStdLog("ciudadBib.c",639,TLError,"Error al recibir nombre desde el Router");
			exit(0);
		}
		cpMensaje = (char*) malloc(2);
		strncpy(cpMensaje, "NJ", 2);
		iMensaje=2;
		
	}

	while ((*sListaDiscovers) && (!strcmp((*sListaDiscovers)->nombreCiudad, cpNombreCiudad)) && !strcmp((*sListaDiscovers)->nombre, cpNombre))
	{
		if(sendall((*sListaDiscovers)->iSocket,cpMensaje, iMensaje,0)==-1)
		{
			fvStdLog("ciudadBib.c",656,TLError,"Error al enviar IP-puerto migracion");
			exit(0);
		}
		sAux = (*sListaDiscovers);
		(*sListaDiscovers) = sAux->siguiente;
		free(sAux->nombre);
		free(sAux->nombreCiudad);
		free(sAux);
	}
	if((*sListaDiscovers) != NULL)
	{
		sAux = (*sListaDiscovers);
		sAux2 = sAux->siguiente;
		while(sAux2!=NULL)
		{
			if(!strcmp(sAux2->nombreCiudad, cpNombreCiudad) && !strcmp(sAux2->nombre, cpNombre))
			{
				if(sendall(sAux2->iSocket,cpMensaje, iMensaje,0)==-1)
				{
					fvStdLog("ciudadBib.c",656,TLError,"Error al enviar IP-puerto migracion");
					exit(0);
				}
				sAux->siguiente = sAux2->siguiente;
				free(sAux2->nombre);
				free(sAux2->nombreCiudad);
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

	/*while(sAux)
	  {
	  if(!strcmp(sAux->nombre,cpNombre) && !strcmp(sAux->nombreCiudad, cpNombreCiudad))
	  {
	  if(sendall(sAux->iSocket,cpMensaje, iMensaje,0)==-1)
	  {
	  fvStdLog("ciudadBib.c",656,TLError,"Error al enviar IP-puerto migracion");
	  exit(0);
	  }
	  free(sAux->nombre);
	  free(sAux->nombreCiudad);
	  if(sAnt)
	  {
	  sAux = sAux->siguiente;
	  free(sAnt->siguiente);
	  sAnt->siguiente = sAux;
	  }
	  else
	  {
	  sAnt = (*sListaDiscovers);
	  (*sListaDiscovers) = sAnt->siguiente;
	  free (sAnt);
	  sAnt = NULL;
	  sAux =(*sListaDiscovers);
	  }
	  }
	  else
	  {
	  sAnt = sAux;
	  sAux = sAux->siguiente;
	  }
	  }*/
	free(cpNombre);
	free(cpNombreCiudad);
	free(cpMensaje);
}

int fiCiuEnviarLevExternaTodos(char * nombreCiudad, struct sTDP* sTabla, struct sEquipo * lista)
{
	struct sEquipo * listaAux;
	int iRetorno;

	iRetorno = 0;
	listaAux = lista;
	while(lista)
	{
		if(fiCiuEnviarLEVExternaEquipo(lista->socket, lista->nombre, nombreCiudad, sTabla, listaAux))
		{
			lista->iTieneLev = 1;
			iRetorno = 1;
		}
		lista = lista->siguiente;
	}
	return iRetorno;
}

int fiCiuEnviarTDPRouter(int iSock,struct sTDP** sTabla, struct sEquipo * sListaE,char * cpMiNombre)
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
		if((strcmp(sTablaRecorrer->ciudad,cpMiNombre)) || fiCiuBuscarEquipoNombre(sListaE,sTablaRecorrer->nombre))
		{
			iTamanio += 6 + strlen(sTablaRecorrer->nombre) + 1 + strlen(sTablaRecorrer->ciudad) + 1;
			iCantidad ++;
		}
		sTablaRecorrer = sTablaRecorrer->siguiente;
	}

	cpMensaje= (char*)malloc (iTamanio);
	if ( cpMensaje==NULL )
	{
		fvStdLog("ciudadBib.c",729,TLError,"Error de Malloc");
		exit (EXIT_FAILURE);
	}

	sTablaRecorrer = (*sTabla);
	strcpy(cpMensaje, "TP");
	cpMensaje[2]=(unsigned char)((iTamanio-4)%256);
	cpMensaje[3]=(unsigned char)((iTamanio-4)/256);
	cpMensaje[4]= iCantidad;
	iPosicion = 5;

	fvStdLog("ciudadBib.c",740,TLMensaje,"enviando TDP. cantidad equipos: %d", iCantidad);

	while(sTablaRecorrer != NULL)
	{
		if((strcmp(sTablaRecorrer->ciudad,cpMiNombre)) || fiCiuBuscarEquipoNombre(sListaE,sTablaRecorrer->nombre))
		{
			fvStdLog("ciudadBib.c",743,TLMensaje,"equipo: %s - puntos: %d - G: %d - E: %d - P: %d", sTablaRecorrer->nombre, sTablaRecorrer->puntos, sTablaRecorrer->pg, sTablaRecorrer->pe, sTablaRecorrer->pp);
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
		}
		sTablaAux = sTablaRecorrer;
		sTablaRecorrer = sTablaRecorrer->siguiente;
		free(sTablaAux->ciudad);
		free(sTablaAux->nombre);
		free(sTablaAux);
	}
	if (sendall (iSock, cpMensaje, iTamanio, 0) == -1)
	{
		fvStdLog("ciudadBib.c",764,TLError,"Error al enviar TDP al Router");
		exit (0);
	}	
	free (cpMensaje);
	(*sTabla) = NULL;
	return 1;
}

int fiCIURecibirTDPRouter(int iSocket, struct sTDP ** lista, struct sEquipo * listaEquipos, char * nombreCiudad)
{
	char cpBufInicial[3];
	char cpBufNombre[257];
	char cpBufCiudad[256];
	char cpBufComienzo[5];
	struct sTDP * nuevo;
	int i;
	int iEncontro = 0;

	recvAll (iSocket, cpBufInicial, 3, 0);


	fvStdLog("ciudadBib.c",785,TLMensaje,"Recibiendo TDP Router...");
	for(i=0; i<cpBufInicial[2]; i++)
	{
		recvAll(iSocket, cpBufComienzo, 5, 0);
		recvAll(iSocket, cpBufNombre, cpBufComienzo[4]+1, 0);
		recvAll(iSocket, cpBufCiudad, cpBufNombre[(int)cpBufComienzo[4]], 0);
		if(!strcmp(cpBufCiudad,  nombreCiudad))
				iEncontro = 1;
		if ((!strcmp(cpBufCiudad, nombreCiudad) && (fiCiuBuscarEquipoNombre(listaEquipos, cpBufNombre) != NULL))||(strcmp(cpBufCiudad,nombreCiudad)))
		{
			nuevo	= (sTDP*)malloc ( sizeof(sTDP) );
			if ( nuevo==NULL )
			{
				fvStdLog("ciudadBib.c",798,TLError,"Error de Malloc");
				exit (1);
			}
			nuevo->nombre	= (char*)malloc (strlen(cpBufNombre) + 1);
			if ( nuevo->nombre==NULL )
			{
				fvStdLog("ciudadBib.c",804,TLError,"Error de Malloc");
				exit (1);
			}
			nuevo->ciudad	= (char*)malloc (strlen(cpBufCiudad) + 1);
			if ( nuevo->ciudad==NULL )
			{
				fvStdLog("ciudadBib.c",810,TLError,"Error de Malloc");
				exit (1);
			}
			nuevo->puntos = cpBufComienzo[0];
			nuevo->pg = cpBufComienzo[1];
			nuevo->pe = cpBufComienzo[2];
			nuevo->pp = cpBufComienzo[3];
			strcpy(nuevo->ciudad, cpBufCiudad);
			strcpy(nuevo->nombre, cpBufNombre);
			fvStdLog("ciudadBib.c",819,TLMensaje,"Equipo:%s - Ciudad:%s - Puntos:%d - G:%d - E:%d - P:%d", nuevo->nombre, nuevo->ciudad, nuevo->puntos, nuevo->pg, nuevo->pe, nuevo->pp);
			nuevo->siguiente = (*lista);
			(*lista) = nuevo;
		}
	}
	while(listaEquipos != NULL)
	{
		if((listaEquipos->estado != ESTADOvacio))
		{
			if(!fiCiuEstaEnTDP(*lista, listaEquipos->nombre))
			{
				nuevo	= (sTDP*)malloc ( sizeof(sTDP) );
				if ( nuevo==NULL )
				{
					fvStdLog("ciudadBib.c",833,TLError,"Error de Malloc");
					exit (1);
				}
				nuevo->nombre	= (char*)malloc (strlen(listaEquipos->nombre) + 1);
				if ( nuevo->nombre==NULL )
				{
					fvStdLog("ciudadBib.c",839,TLError,"Error de Malloc");
					exit (1);
				}
				nuevo->ciudad	= (char*)malloc (strlen(nombreCiudad) + 1);
				if ( nuevo->ciudad==NULL )
				{
					fvStdLog("ciudadBib.c",845,TLError,"Error de Malloc");
					exit (1);
				}
				nuevo->puntos = 0;
				nuevo->pg = 0;
				nuevo->pe = 0;
				nuevo->pp = 0;
				strcpy(nuevo->ciudad, nombreCiudad);
				strcpy(nuevo->nombre, listaEquipos->nombre);
				nuevo->siguiente = (*lista);
				(*lista) = nuevo;
			}
		}
		listaEquipos = listaEquipos->siguiente;
	}
	return iEncontro;
}

void fiCiuSacarEquipoLista(int iSock, struct sEquipo ** lista)
{
	struct sEquipo * sAux;
	struct sEquipo * sAux2;

	if (((*lista) != NULL) && ((*lista)->socket == iSock) /*&& ((*lista)->estado == ESTADOvacio)*/)
	{
		sAux = (*lista);
		(*lista) = sAux->siguiente;
		fvCiuBorrarEquipo(&sAux);
	}
	else if((*lista) != NULL)
	{
		sAux = (*lista);
		sAux2 = sAux->siguiente;
		while(sAux2!=NULL)
		{
			if(sAux2->socket==iSock /*&& sAux2->estado==ESTADOvacio*/)
			{
				sAux->siguiente = sAux2->siguiente;
				fvCiuBorrarEquipo(&sAux2);
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

void fiCiuSacarEquipoColas(int iSock, struct sEquipo * lista)
{
	struct sEquipoCola * sAux;
	struct sEquipoCola * sAux2;


	while(lista)
	{
		if ((lista->cola != NULL) && ((lista->cola)->socket == iSock))
		{
			sAux = (lista->cola);
			(lista->cola) = sAux->siguiente;
			free(sAux);
		}
		else if((lista->cola) != NULL)
		{
			sAux = (lista->cola);
			sAux2 = sAux->siguiente;
			while(sAux2!=NULL)
			{
				if(sAux2->socket==iSock)
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
		lista = lista->siguiente;
	}
}

void fvCiuBorrarEquipo(struct sEquipo ** equipo)
{
	struct sEquipoCola * colaAux;
	char * cpMensaje;

	cpMensaje= (char*)malloc (2);
	if ( cpMensaje==NULL )
	{
		fvStdLog("ciudadBib.c",940,TLError,"Error de Malloc");
		exit (1);
	}

	strcpy(cpMensaje, "NJ");

	while((*equipo)->cola != NULL)
	{
		if (sendall (((*equipo)->cola)->socket, cpMensaje, 2, 0) == -1)
		{
			fvStdLog("ciudadBib.c",950,TLError,"Error al enviar mensaje NJ");
			exit (0);
		}
		colaAux = (*equipo)->cola;
		(*equipo)->cola = ((*equipo)->cola)->siguiente;
		free(colaAux);
	}
	free((*equipo)->nombre);
	free(*equipo);
	free (cpMensaje);
}

int fiCiuManejarMensajeLibre(int iSock, struct sEquipo * lista)
{
	int puerto;
	struct sEquipo* equipo;
	struct sEquipoCola * colaAux;
	char *cpMensaje;
	char *cpBuf;

	cpMensaje = (char*) malloc(8);
	cpBuf = (char*) malloc(6);
	equipo = fiCiuBuscarEquipoSocket(lista, iSock);

	if(recvAll(iSock, cpBuf, 6, 0)==-1)
	{
		fvStdLog("ciudadBib.c",976,TLError,"Error al recibir Ip Puerto RJ");
		exit (1);
	}
	strcpy(cpMensaje,"RJ");
	memcpy(&(cpMensaje[2]), cpBuf, 6);

	if (equipo->cola != NULL)
	{
		if(sendall((equipo->cola)->socket,cpMensaje, 8, 0)==-1)
		{
			fvStdLog("ciudadBib.c",986,TLError,"error al enviar mensaje RJ");
			exit (1);
		}

		fvStdLog("ciudadBib.c",990,TLMensaje,"enviado mensaje 'libre' del equipo %s", equipo->nombre);

		colaAux = equipo->cola;
		equipo->cola = (equipo->cola)->siguiente;

		free(colaAux);
	}
	free(cpMensaje);
	free(cpBuf);
	return 1;
}

int fiCiuRecibirCabeceraRouter(int iSocket)
{
	char cpBuf[3];
	int iTam;

	iTam = recvAll (iSocket, cpBuf, 2, 0);

	cpBuf[2] = '\0';

	fvStdLog("ciudadBib.c",1009,TLMensaje,"recibida cabecera '%s' del router", cpBuf);
	if (iTam <= 0)
		return (iTam);

	if (!(strncmp (cpBuf, "TP", 2))) /*TABLA DE POSICIONES*/
		return (MSTDP);

	if (!(strncmp (cpBuf, "FO", 2))) /*FOUND*/
		return (MSFound);

	if (!(strncmp (cpBuf, "NF", 2))) /*NOT FOUND*/
		return (MSNotFound);

	if (!(strncmp (cpBuf, "E?", 2))) /*NOT FOUND*/
		return (MSEPregunta);

	return (-1);
}

int fiCiuEstaEnTDP (struct sTDP *lista, char *cpNombre)
{
	while (lista)
	{
		if (!strcmp (lista->nombre, cpNombre))
			return 1;
		lista = lista->siguiente;
	}
	return 0;
}

int fiCiuEnviarDT(int iSockRouter,char * nombreCiudad)
{
	char * cpBuf;

	cpBuf=(char *)malloc(strlen(nombreCiudad)+4);
	strcpy(cpBuf,"DT");
	fvStdLog("ciudadBib.c",1045,TLMensaje,"enviado mensaje DT al router con nombre: %s", nombreCiudad);
	cpBuf[2]=strlen(nombreCiudad)+1;
	memcpy(&(cpBuf[3]),nombreCiudad,strlen(nombreCiudad)+1);

	sendall(iSockRouter,cpBuf,strlen(nombreCiudad)+4,0);
	free(cpBuf);
	return 1;
}

int fiCiuManejarMigracion(int iSock, struct sMigracion **sMigracion, char * cpIp, char * cpPuerto, char * cpNombreExecEquipo)
{
	char * cpMigracion;
	char cpTamanio[2];
	int iTamanio;
	int i;

	if(recvAll(iSock, cpTamanio, 2, 0) == -1)
	{
		fvStdLog("ciudadBib.c",1063,TLError,"Error al recibir tamanio migracion");
		exit(0);
	}
	iTamanio = (unsigned char)cpTamanio[0] + (unsigned char)cpTamanio[1] * 256;

	cpMigracion = (char *) malloc(iTamanio);

	if(recvAll(iSock, cpMigracion, iTamanio, 0) == -1)
	{
		fvStdLog("ciudadBib.c",1072,TLError,"Error al recibir datos migracion");
		exit(0);
	}
	fvCIUAgregarFIFOMigracion (sMigracion, iSock, iTamanio,cpMigracion);
	
	if (!(i=fork()))
	{
		if(execlp(cpNombreExecEquipo,cpNombreExecEquipo,cpIp, cpPuerto, NULL) == -1)
		{
			fvStdLog("ciudadBib.c",1081,TLError,"Error al Levantar Hijo");
			exit(0);
		}
	}
}

void fvCIUAgregarFIFOMigracion (struct sMigracion **cola, int iSocket, int iTamanio, char * cpDatos)
{
	struct sMigracion *nuevo;
	struct sMigracion *recorrer;

	nuevo = (struct sMigracion *) malloc(sizeof(struct sMigracion));

	nuevo->iTamanioDatos = iTamanio;
	nuevo->iSocket = iSocket;
	nuevo->cpDatos = cpDatos;
	nuevo->siguiente = NULL;

	if (*cola == NULL)
	{
		*cola = nuevo;
	}
	else
	{
		recorrer = *cola;
		while (recorrer->siguiente != NULL)
		{
			recorrer = recorrer->siguiente;
		}
		recorrer->siguiente = nuevo;
	}
}

void handlerSigchild(int signum)
{
	int condicion;
	if(signum == SIGCHLD)
	{
		fvStdLog("ciudadBib.c",1119,TLMensaje,"se cerro el hijo XP");
		wait(&condicion);
	}
}

int fvCIUEnviarDatosMig(int iSocket, struct sMigracion ** cola)
{
	struct sMigracion * sAux;
	char cpCabecera[2];
	
	if(*cola)
	{
		strcpy(cpCabecera, "DM");

		if(sendall(iSocket, cpCabecera, 2, 0) == -1)
		{
			fvStdLog("ciudadBib.c",1135,TLError, "Error al enviar Cabecera de datos al equipo");
			return 8;
		}
		
		if(sendall(iSocket, (*cola)->cpDatos, (*cola)->iTamanioDatos, 0) == -1)
		{
			fvStdLog("ciudadBib.c",1141,TLError, "Error al enviar Cabecera de datos al equipo");
			return 8;
		}

		if(sendall((*cola)->iSocket, "OK", 2, 0) == -1)
		{
			fvStdLog("ciudadBib.c",1147,TLError, "Error al enviar OK al equipo");
			return 7;
		}
		
		sAux = (*cola);
		(*cola) = (*cola)->siguiente;
		free(sAux->cpDatos);
		free(sAux);
	}
}

int fiCiuManejarPregunta(int iSockRouter,struct sEquipo * sListaEquipos,char * cpIp, int iPuerto)
{
	char * cpNomEqui;
	char * cpMensaje;
	char cTam;
	int iIdMensaje;
	char iAux[2];
	struct sEquipo * sNodo;


	//if(recvAll(iSockRouter,iAux,2,0)==-1)
	//	perror("recvAll");

//	iIdMensaje = iAux[0]+iAux[1]*256;

	if(recvAll(iSockRouter,&cTam,1,0)==-1)
		perror("recvAll");

	cpNomEqui=(char*)malloc(cTam);

	if(recvAll(iSockRouter,cpNomEqui,cTam,0)==-1)
		perror("recvAll");

	sNodo = fiCiuBuscarEquipoNombre(sListaEquipos,cpNomEqui);

	if (sNodo != NULL)
	{
		cpMensaje = (char*) malloc(strlen(cpNomEqui)+2 + 8);
		strcpy(cpMensaje,"EE");
	//	memcpy(&(cpMensaje[2]),iAux,2);

	  fiRouScanfIP(&(cpMensaje[2]), cpIp, iPuerto);
		
//		sscanf(cpIp, "%u.%u.%u.%u", (unsigned int *)&(cpMensaje[2]), (unsigned int *)&(cpMensaje[3]), (unsigned int *)&(cpMensaje[4]), (unsigned int *)&(cpMensaje[5]));
//		cpMensaje[6] = iPuerto % 256;
//		cpMensaje[7] = iPuerto / 256;
		cpMensaje[8] = strlen(cpNomEqui) +1;
		memcpy(&(cpMensaje[9]), cpNomEqui, strlen(cpNomEqui)+1);
		

		if(sendall(iSockRouter,cpMensaje, 8 + strlen(cpNomEqui) + 2 )<0)
		{
			fvStdLog("ciudadBib.c",1197,TLError, "Error al Enviar E! al Router");
			return -1;
		}
		fvStdLog("ciudadBib.c",1197,TLMensaje, "enviado E! al router");
	}
	else
	{
		cpMensaje = (char*) malloc(strlen(cpNomEqui)+4);
		strcpy(cpMensaje,"NE");
		cpMensaje[2] = strlen(cpNomEqui)+1;
		memcpy(&(cpMensaje[3]),cpNomEqui,strlen(cpNomEqui)+1);
		if(sendall(iSockRouter,cpMensaje,4 + strlen(cpNomEqui))<0)
		{
			fvStdLog("ciudadBib.c",1209,TLError, "Error al Enviar NE al Router");
			return -1;
		}
		fvStdLog("ciudadBib.c",1197,TLMensaje, "enviado NE al router");
	}
	free(cpNomEqui);
	free(cpMensaje);
}

int sendall(int s, char *buf, int len,int iNiBola)
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

int fiCiuLimpiarListaEquipos(struct sEquipo ** sListaE)
{
	struct sEquipo * sAux;
	struct sEquipo * sAux2;

	while (((*sListaE) != NULL) && ((*sListaE)->socket==0))
	{
		sAux = (*sListaE);
		(*sListaE) = sAux->siguiente;
		free(sAux->nombre);
		free(sAux);
	}
	if((*sListaE) != NULL)
	{
		sAux = (*sListaE);
		sAux2 = sAux->siguiente;
		while(sAux2!=NULL)
		{
			if(!(sAux2->socket))
			{
					sAux->siguiente = sAux2->siguiente;
					free(sAux2->nombre);
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
