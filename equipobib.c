#include <signal.h>
#include <ctype.h>
#include <unistd.h>
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
#include "equipobib.h"
#include "logs.h"

#define STDIN 0
/*----------------------------------------------------------------------------------------------------------*/
int fiEquConectar(char* cpIp, int iPuerto)
{
	int iSockfd;
	struct sockaddr_in dirCiudad;
	int iYes;

	iYes= 1;

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
	  close(iSockfd);
		return -1;
	}

	fvStdLog("equipobib.c",45,TLMensaje,"Realizada la conexion con  IP:%s Puerto:%d", cpIp, iPuerto);
	return iSockfd;
}
/*------------------------------------------------------------------------------------------------------------*/
char* fcEquMayusculas(char string[]) /*recibe un char* y lo devuelve convertido a mayusculas*/
{
	int i;

	for(i=0; string[i] != '\0'; i++)
		string[i] = toupper(string[i]);

	return (&string[0]);

}
/*--------------------------------------------------------------------------------------------------------------*/
int fiEquReciboCabecera(int iSocket)
{
	char* cpCabecera;
	int iRec;

	cpCabecera= (char*) malloc (3);
	if((iRec=recvAll(iSocket, cpCabecera, 2,0))<0)
	{
		fvStdLog("equipobib.c",68,TLError,"Error al Recibir la cabecera");
		exit(1);

	}
	else if(iRec == 0)
	{
		fvStdLog("equipobib.c",74,TLError,"La Ciudad se ha Desconectado");
		return(0);
	}
	cpCabecera[2]='\0';

	fcEquMayusculas(cpCabecera);
	if(!(strncmp(cpCabecera, "LI",2)))
	{
		free(cpCabecera);
		return 1;
	}
	if(!(strncmp(cpCabecera, "LE",2)))
	{
		free(cpCabecera);
		return 2;
	}
	if(!(strncmp(cpCabecera, "PJ",2)))
	{
		free(cpCabecera);
		return 3;
	}
	if(!(strncmp(cpCabecera, "RE",2)))
	{
		free(cpCabecera);
		return 4;
	}

	free(cpCabecera);
	return 5; /*en caso de recibir basura*/
}
/*-----------------------------------------------------------------------------------------------------------------*/
int fiEquRecibirLevI(int iSocketCiudad,struct lLevInt ** lLevInt)
{
	char cCant;
	char cLargo;
	int i;
	char cResultado;
	char* cpNombre;
	struct lLevInt * nodoEquipo;
	char * cpNombreCiudad;

	if(recvAll(iSocketCiudad,&cCant,1,0)<= 0)
		return -1;

	for(i=0;i<cCant;i++)
	{
		nodoEquipo =(struct lLevInt*)malloc(sizeof(struct lLevInt	));
		
		if(recvAll(iSocketCiudad,&cResultado,1,0)<= 0)
			return -1;

		nodoEquipo->iResultado = cResultado;
		
		if(recvAll(iSocketCiudad,&cLargo,1,0)<= 0)
			return -1;
		cpNombreCiudad = (char *)malloc(cLargo);
		nodoEquipo->cpNombreCiudad=(char*)malloc(cLargo);
		if(recvAll(iSocketCiudad,cpNombreCiudad,cLargo,0)<= 0)
			return -1;
		strcpy(nodoEquipo->cpNombreCiudad, cpNombreCiudad);

		if(recvAll(iSocketCiudad,&cLargo,1,0)<= 0)
			return -1;

		cpNombre = (char *)malloc(cLargo);
		nodoEquipo->cpNombreEquipo=(char*)malloc(cLargo);
		if(recvAll(iSocketCiudad,cpNombre,cLargo,0)<= 0)
			return -1;
		strcpy(nodoEquipo->cpNombreEquipo,cpNombre);
		
		fiEquAgregarALista(nodoEquipo,lLevInt);
		free(cpNombre);
		free(cpNombreCiudad);
	}
	return (0);
}
/*--------------------------------------------------------------------------------------------------------------------*/
int fiEquRecibirLevE(int iSocketCiudad,struct lLevInt ** lLevExt)
{
	char cCant;
	char cLargo;
	int i;
	char cResultado;
	struct lLevInt * nodoEquipo;


	if(recvAll(iSocketCiudad,&cCant,1,0)<= 0)
		return -1;

	for(i=0;i<cCant;i++)
	{
		nodoEquipo =(struct lLevInt*)malloc(sizeof(lLevInt));
		if(recvAll(iSocketCiudad,&cResultado,1,0)<= 0)
			return -1;
		nodoEquipo->iResultado = cResultado;

		if(recvAll(iSocketCiudad,&cLargo,1,0)<= 0)
			return -1;
		nodoEquipo->cpNombreCiudad=(char*)malloc(cLargo);
		if(recvAll(iSocketCiudad,nodoEquipo->cpNombreCiudad,cLargo,0)<= 0)
			return -1;

		if(recvAll(iSocketCiudad,&cLargo,1,0)<= 0)
			return -1;
		nodoEquipo->cpNombreEquipo=(char*)malloc(cLargo);
		if(recvAll(iSocketCiudad,nodoEquipo->cpNombreEquipo,cLargo,0)<= 0)
			return -1;

		fiEquAgregarALista(nodoEquipo,lLevExt);
	}
	return cCant;
}
/*--------------------------------------------------------------------------------------------------------------------*/
int fiEquAgregarALista(struct lLevInt * nodo , struct lLevInt ** lLevInt)
{
	struct lLevInt * lAct=NULL;
	struct lLevInt * lAnt=NULL;

	lAct = *lLevInt;

	if (lAct == NULL)
	{
		*lLevInt = nodo;
		nodo->siguiente = NULL;
		return(0);
	}

	while(lAct != NULL )
	{
		lAnt=lAct;
		lAct=lAct->siguiente;
	}

	lAnt->siguiente = nodo;
	nodo->siguiente = lAct;

	return(0);
}
/*------------------------------------------------------------------------------------------------------------------*/
int fiEquJugarLevI(struct lLevInt * lLevInt, int tDurPartido,int * iCansancio,int iCansancioMax, int iSocketCiudad,int tTiempoMaxCansado, char * cpMiNombre)
{
	struct lLevInt * lLevAux;
	int iPermiso ;
	char cpIpEquipo[16];
	char * buf;
	int iPuertoEquipo;
	int iRec;
	int iSalir;
	int iCant;
	int iSocketEquipo;
	fd_set repartloc;
	fd_set mapartloc;
	struct timeval tv;
	int iFin;
	int iMax;

	iSalir = 0;
	iCant = 0;
	lLevAux = lLevInt;

	while(lLevAux != NULL)
	{
		iCant ++;
		if((iPermiso=fiEquPermisoJugar(iSocketCiudad,lLevAux->cpNombreCiudad,lLevAux->cpNombreEquipo,cpIpEquipo,&iPuertoEquipo))==-1)/*Espero hasta q me lo de*/
		{
			fvStdLog("equipobib.c",239,TLError,"Error al Pedir Permiso para jugar con:%s",lLevAux->cpNombreEquipo);
			return -1;
		}
		else if(iPermiso==1)    /*puedo jugar*/
		{
			if((iSocketEquipo=fiEquConectar(cpIpEquipo,iPuertoEquipo))==-1)
			{
				fvStdLog("equipobib.c",246,TLError,"Error al Conectarme a Equipo para jugar Partido Visitante");
				iFin=1;
			//	return -1;
			}
			else
			{
				iMax=iSocketEquipo;
				fvStdLog("equipobib.c",253,TLMensaje,"Comenzo Partido de Visitante");
				FD_ZERO(&mapartloc);
				FD_ZERO(&repartloc);
				FD_SET(iSocketCiudad, &mapartloc);
				FD_SET(iSocketEquipo, &mapartloc);
				tv.tv_sec = tDurPartido;
				tv.tv_usec = 0;
				iFin = 0;
			}

			while(iFin == 0)
			{
				repartloc = mapartloc;
				if(select(iMax + 1,&repartloc,NULL,NULL,&tv)==-1)/*Jugando*/
				{
					fvStdLog("equipobib.c",268,TLError,"Error en Select");
					return -1;
				}
				if(FD_ISSET(iSocketCiudad,&repartloc))/*La Ciudad seguramente se desconecto*/
				{
					buf=(char*)malloc(2);
					if(recvAll(iSocketCiudad,buf,1,0)==0)
					{
						fvStdLog("equipobib.c",276,TLError,"Error la Ciudad se Desconecto");
						exit(0);
					}
					else
					{
						fvStdLog("equipobib.c",281,TLError,"Error la Ciudad me Mando un Mensaje Erroneo");
						return -1;
					}
					free(buf);
				}
				if(FD_ISSET(iSocketEquipo,&repartloc))/*Se desconecto el otro Equipo*/
				{
					buf=(char *)malloc(3);
					if((iRec=recvAll(iSocketEquipo,buf,2,0))==-1)
					{
						fvStdLog("equipobib.c",291,TLError,"Error al Recibir HANDSHAKE!!!");
						FD_CLR(iSocketEquipo,&mapartloc);
						close(iSocketEquipo);
						iFin = 1;
					}
					else if(iRec==0)
					{
						FD_CLR(iSocketEquipo,&mapartloc);
						close(iSocketEquipo);
						iFin = 1;
						lLevAux->iResultado = 'G';
						fvStdLog("equipobib.c",302,TLMensaje,"Finalizo partido de visitante");
					}
					else
					{
						buf[2]='\0';
						if(!strcmp(buf, "HS"))
						{
							if(sendall(iSocketEquipo, "HS", 2)==-1)
							{
								fvStdLog("equipobib.c",311,TLError,"Error al enviarle el Hand Shake");
								return -1;
							}

						}
						else
						{
							fvStdLog("equipobib.c",318,TLError,"No me mando HS, mando verdura!");
							return -1;
						}
					}
				}
				if((tv.tv_sec == 0)&&(tv.tv_usec == 0))
				{
					FD_CLR(iSocketEquipo,&mapartloc);
					iFin = 1;
					close(iSocketEquipo);
					fvStdLog("equipobib.c",328,TLMensaje,"Finalizo partido de visitante");
					(*iCansancio) = ((*iCansancio) + fiEquCansancio()); /*Genera de 1 a 5 unidades de cansancio al azar*/
					if((*iCansancio)>iCansancioMax)
					{
						if(fiEquDescansarTotalmente(iSocketCiudad,iCansancio,iCansancioMax,tTiempoMaxCansado, cpMiNombre)==-1)
						{
							fvStdLog("equipobib.c", 334,TLError, "Error al descansar totalmente");
							return -1;
						}
					}
					lLevAux->iResultado = fiEquCalcularResultado();
				}
			}
		}
		lLevAux = lLevAux->siguiente;
	}
	if(fiEquEnviarResultados(iSocketCiudad,&lLevInt,iCant,"LI")==-1)
	{
		fvStdLog("equipobib.c",346,TLError,"Error al Enviar Resultados a la Ciudad");
		return -1;
	}
	return 0;
}
/*-----------------------------------------------------------------------------------------------------------------*/
int fiEquJugarLevE(int iSocketCiudad,struct lLevInt ** lLevExt,int tDurPartido,int * iCansancio,int iCansancioMax,int tTiempoMaxCansado,int iMaxTiempo,char * cpMiIp,int iMiPuerto,char *cpIpMiCiudad,int iMiPuertoCiudad,char * cpMiNombre,char * cpIpCiuActual,int iPuertoCiuActual)
{
	int iCant = 0;
	char * cpIp;
	int iPuerto;
	struct lLevInt * lAux;
	struct DatosMigrar sDatos;
	int iSockMigrar;
	int iSocketEquipo;
	fd_set mapartloc;
	fd_set repartloc;
	int iFin=0;
	int iMax;
	struct timeval tv;
	char * buf;
	int iRec = 0;
	int iPermiso;

	cpIp=(char*)malloc(16);
	lAux = *lLevExt;

	while(lAux && lAux->iResultado)	
	{
		lAux = lAux->siguiente;
	}
	if(lAux)
		iPermiso=fiEquPermisoJugar(iSocketCiudad,lAux->cpNombreCiudad,lAux->cpNombreEquipo,cpIp,&iPuerto);
	
	while((lAux != NULL)&&(iPermiso==1 || iPermiso == 0))/*PUEDO JUGAR*/
	{
		if(iPermiso==1)
		{
			if(!(lAux->iResultado))
			{
				if((iSocketEquipo=fiEquConectar(cpIp,iPuerto))==-1)
				{
					fvStdLog("equipobib.c",388,TLError,"Error al Conectarme a Equipo para jugar Partido Visitante");
					iFin=1;
				}
				else
				{
					iMax=iSocketEquipo;
					fvStdLog("equipobib.c",394,TLMensaje,"Comenzo Partido de Visitante Migrado");
					FD_ZERO(&mapartloc);
					FD_ZERO(&repartloc);
					FD_SET(iSocketCiudad, &mapartloc);
					FD_SET(iSocketEquipo, &mapartloc);
					tv.tv_sec = tDurPartido;
					tv.tv_usec = 0;
					iFin = 0;
				}

				while(iFin == 0)
				{
					repartloc = mapartloc;
					if(select(iMax + 1,&repartloc,NULL,NULL,&tv)==-1)/*Jugando*/
					{
						fvStdLog("equipobib.c",409,TLError,"Error en Select");
						return -1;
					}
					if(FD_ISSET(iSocketCiudad,&repartloc))/*La Ciudad seguramente se desconecto*/
					{
						buf=(char*)malloc(1);
						if(recvAll(iSocketCiudad,buf,1,0)==0)
						{
							fvStdLog("equipobib.c",417,TLError,"Error la Ciudad se Desconecto");
							exit(0);
						}
						else
						{
							fvStdLog("equipobib.c",422,TLError,"Error la Ciudad me Mando un Mensaje Erroneo");
							return -1;
						}
						free(buf);
					}
					if(FD_ISSET(iSocketEquipo,&repartloc))/*Se desconecto el otro Equipo*/
					{
						buf=(char *)malloc(3);
						if((iRec=recvAll(iSocketEquipo,buf,2,0))==-1)
						{
							fvStdLog("equipobib.c",432,TLError,"Error al Recibir HANDSHAKE!!!");
							FD_CLR(iSocketEquipo,&mapartloc);
							close(iSocketEquipo);
							iFin = 1;
						}
						else if(iRec==0)
						{
							FD_CLR(iSocketEquipo,&mapartloc);
							close(iSocketEquipo);
							iFin = 1;
							lAux->iResultado = 'G';
							fvStdLog("equipobib.c",443,TLMensaje,"Finalizo partido de visitante");
						}
						else
						{
							buf[2]='\0';
							if(!strcmp(buf, "HS"))
							{
								if(sendall(iSocketEquipo, "HS", 2)==-1)
								{
									fvStdLog("equipobib.c",452,TLError,"Error al enviarle el Hand Shake");
									return -1;
								}

							}
							else
							{
								fvStdLog("equipobib.c",459,TLError,"No me mando HS, mando verdura!");
								return -1;
							}
						}
					}
					if((tv.tv_sec == 0)&&(tv.tv_usec == 0))
					{
						FD_CLR(iSocketEquipo,&mapartloc);
						iFin = 1;
						close(iSocketEquipo);
						fvStdLog("equipobib.c",469,TLMensaje,"Finalizo partido de visitante");
						(*iCansancio) = ((*iCansancio) + fiEquCansancio()); /*Genera de 1 a 5 unidades de cansancio al azar*/
						if((*iCansancio)>iCansancioMax)
						{
							if(fiEquDescansarTotalmente(iSocketCiudad,iCansancio,iCansancioMax,tTiempoMaxCansado, cpMiNombre)==-1)
							{
								fvStdLog("equipobib.c", 475,TLError, "Error al descansar totalmente");
								return -1;
							}
						}
						lAux->iResultado = fiEquCalcularResultado();
					}
				}
			}
		}
		else 
		{
			lAux->iResultado = 'N'; 
			//fiEquSacarEquipoLev(lLevExt,lAux->cpNombreCiudad,lAux->cpNombreEquipo);
		}
		lAux = lAux->siguiente;
		while(lAux && lAux->iResultado)	
		{
			lAux = lAux->siguiente;
		}
		if(lAux)
			iPermiso=fiEquPermisoJugar(iSocketCiudad,lAux->cpNombreCiudad,lAux->cpNombreEquipo,cpIp,&iPuerto);
	}
	if(!lAux)
	{
		if((!strcmp(cpIpMiCiudad,cpIpCiuActual))&&(iMiPuertoCiudad==iPuertoCiuActual))
		{
			return 1;
		}

		if((iSockMigrar=fiEquConectar(cpIpMiCiudad,iMiPuertoCiudad))==-1)
		{

			fvStdLog("equipobib.c",508,TLError,"No puedo Volver =( a Mi Ciudad IP:%s PUERTO:%d",cpIp,iPuerto);
			exit(0);

		}
		strcpy(sDatos.cpMiIp,cpMiIp);
		sDatos.iMiPuerto=iMiPuerto;
		strcpy(sDatos.cpIpCiudad,cpIpMiCiudad);
		sDatos.iPuertoCiudad=iMiPuertoCiudad;
		sDatos.iMaxTiempo=iMaxTiempo;
		sDatos.iDurPartido=tDurPartido;
		sDatos.iTiempoMaxCansado=tTiempoMaxCansado;
		sDatos.iCansancio=*iCansancio;
		sDatos.iMaxCansancio=iCansancioMax;

		if(fiEquEnviarMigracion(iSockMigrar,*lLevExt,sDatos,cpMiNombre)==-1)
		{
			fvStdLog("equipobib.c",524,TLError,"Error al Enviar Datos Migracion a la Ciudad");
			return -1;
		}
		else
		{
			fvStdLog("equipobib.c",529,TLMensaje,"Equipo Volvio a su Ciudad a IP:%s PUERTO:%D",cpIp,iPuerto);
			exit(0);
		}

	}
	else if(iPermiso==2) 
	{

		if((iSockMigrar=fiEquConectar(cpIp,iPuerto))==-1)
		{

			fvStdLog("equipobib.c",540,TLError,"Error al Conectar con Ciudad IP:%s PUERTO:%d",cpIp,iPuerto);

		}
		strcpy(sDatos.cpMiIp,cpMiIp);
		sDatos.iMiPuerto=iMiPuerto;
		strcpy(sDatos.cpIpCiudad,cpIpMiCiudad);
		sDatos.iPuertoCiudad=iMiPuertoCiudad;
		sDatos.iMaxTiempo=iMaxTiempo;
		sDatos.iDurPartido=tDurPartido;
		sDatos.iTiempoMaxCansado=tTiempoMaxCansado;
		sDatos.iCansancio=*iCansancio;
		sDatos.iMaxCansancio=iCansancioMax;

		if(fiEquEnviarMigracion(iSockMigrar,*lLevExt,sDatos,cpMiNombre)==-1)
		{
			fvStdLog("equipobib.c",555,TLError,"Error al Enviar Datos Migracion a la Ciudad");
			return -1;
		}
		else
		{

			fvStdLog("equipobib.c",561,TLMensaje,"Equipo Migrado a IP:%s PUERTO:%D",cpIp,iPuerto);
			exit(0);
		}
	}
	else if(iPermiso==-1)
	{
		fvStdLog("equipobib.c",567,TLError,"Error al Pedir Permiso para jugar con:%s",lAux->cpNombreEquipo);
		return -1;
	}
}
/*-----------------------------------------------------------------------------------------------------------------*/
int fiEquEnviarResultados(int iSocketCiudad,struct lLevInt ** lLevInt,int iCant,char * cpTipo)
{
	char * cpResultados;
	struct lLevInt * lAux;

	cpResultados = (char *)malloc(3);
	sprintf(cpResultados,"%s",cpTipo);
	cpResultados[2]=iCant;
	if(sendall(iSocketCiudad,cpResultados,3)==-1)
	{
		fvStdLog("equipobib.c",582,TLError,"Error al Enviar Resultados a la Ciudad");
		return -1;
	}
	free(cpResultados);

	while((*lLevInt)!=NULL)
	{
		cpResultados = (char *)malloc(strlen((*lLevInt)->cpNombreCiudad)+strlen((*lLevInt)->cpNombreEquipo)+5);
		cpResultados[0]=(*lLevInt)->iResultado;
		cpResultados[1]=strlen((*lLevInt)->cpNombreCiudad) + 1;
		memcpy(&(cpResultados[2]), (*lLevInt)->cpNombreCiudad, strlen((*lLevInt)->cpNombreCiudad) + 1);	
		cpResultados[3+strlen((*lLevInt)->cpNombreCiudad)]=strlen((*lLevInt)->cpNombreEquipo) + 1;
		memcpy(&(cpResultados[4+strlen((*lLevInt)->cpNombreCiudad)]), (*lLevInt)->cpNombreEquipo, strlen((*lLevInt)->cpNombreEquipo) + 1);	
		if(sendall(iSocketCiudad,cpResultados,strlen((*lLevInt)->cpNombreCiudad)+strlen((*lLevInt)->cpNombreEquipo)+5)==-1)
		{
			fvStdLog("equipobib.c",597,TLError,"Error al Enviar Permiso a la Ciudad");
			return -1;
		}
		free(cpResultados);
		lAux = *lLevInt;
		*lLevInt = (*lLevInt)->siguiente;
		free(lAux);
	}
	fvStdLog("equipobib.c",605,TLMensaje,"Se enviaron los Resultados a la CIudad");
	return 0;
}
/*-------------------------------------------------------------------------------------------------------------------------*/
int fiEquEnviarMigracion(int iSocketCiudad,struct lLevInt * lLevExt,struct DatosMigrar sDatos,char * cpMiNombre)
{
	char * cpMensaje;
	struct lLevInt * lLevAux;
	int iTamanio;
	int iPosicion;
	int iCantidad;
	char cpConfirm[2];

	iTamanio = 5;
	iCantidad = 0;
	lLevAux = lLevExt;

	while(lLevAux != NULL)
	{
		iTamanio += 5+strlen(lLevAux->cpNombreEquipo)+strlen(lLevAux->cpNombreCiudad);
		iCantidad++;
		lLevAux = lLevAux->siguiente;
	}

	if(iCantidad == 0) 
		return(0);

	iTamanio += strlen(cpMiNombre)+2;

	cpMensaje= (char*)malloc(iTamanio+sizeof(struct DatosMigrar));
	if(cpMensaje==NULL)
	{
		fvStdLog("equipobib.c",637,TLError,"Error de Malloc");
		exit (EXIT_FAILURE);
	}

	lLevAux = lLevExt;
	strcpy(cpMensaje, "MG");
	cpMensaje[2]= (unsigned char)((iTamanio-4 + sizeof(struct DatosMigrar))%256);
	cpMensaje[3]= (unsigned char)((iTamanio-4 + sizeof(struct DatosMigrar))/256);
	cpMensaje[4]= iCantidad;
	iPosicion = 5;

	while(lLevAux!= NULL)
	{
		cpMensaje[iPosicion] = (unsigned char)lLevAux->iResultado;
		cpMensaje[iPosicion+1] =(unsigned char) strlen(lLevAux->cpNombreCiudad) + 1;
		memcpy(&(cpMensaje[iPosicion+2]), lLevAux->cpNombreCiudad, strlen(lLevAux->cpNombreCiudad)+1);
		cpMensaje[strlen(lLevAux->cpNombreCiudad)+3+iPosicion] =(unsigned char)strlen(lLevAux->cpNombreEquipo)+1;
		memcpy(&(cpMensaje[strlen(lLevAux->cpNombreCiudad)+4+iPosicion]), lLevAux->cpNombreEquipo, strlen(lLevAux->cpNombreEquipo) + 1);
		iPosicion += 5 + strlen(lLevAux->cpNombreEquipo) + strlen(lLevAux->cpNombreCiudad);
		lLevAux = lLevAux->siguiente;
	}

	cpMensaje[iPosicion++] = strlen(cpMiNombre) + 1;
	memcpy(&(cpMensaje[iPosicion]), cpMiNombre, strlen(cpMiNombre) + 1);

	if (sendall(iSocketCiudad, cpMensaje, iTamanio ) == -1)
	{
		fvStdLog("equipobib.c",665,TLError,"Error al enviar LEV EXTERNA a la Ciudad");
		return(-1);
	}
	if (send(iSocketCiudad, &sDatos, sizeof(struct DatosMigrar), 0)< (sizeof(struct DatosMigrar)))
	{
		fvStdLog("equipobib.c",665,TLError,"Error al enviar LEV EXTERNA a la Ciudad");
		return(-1);
	}
	if(recvAll(iSocketCiudad,cpConfirm,2,0)==-1)
	{
		fvStdLog("equipobib.c",670,TLError,"Error al Recibir Confirmacion de la Ciudad");
		return(-1);
	}
	else
	{
		if(strncmp(cpConfirm,"OK",2))
			return -1;
	}	

	fvStdLog ("equipobib.c",679,TLMensaje,"enviados Datos Migracion a Ciudad:%d",iTamanio);
	free (cpMensaje);
	return(1);
}
/*-------------------------------------------------------------------------------------------------------------------------*/
int fiEquPermisoJugar(int iSocketCiudad,char * nombreCiudad,char * nombreEquipo,char * cpIpEquipo,int * iPuertoEquipo)
{
	char * buf;
	fd_set maefds;
	fd_set readfds;
	int iSalir;
	int iMsje;
	int iLargo;

	buf = (char *)malloc(strlen(nombreCiudad)+strlen (nombreEquipo) + 6);

	strcpy(buf,"PJ");
	buf[2] = strlen(nombreCiudad)+1;
	memcpy(&(buf[3]),nombreCiudad,strlen(nombreCiudad)+1);
	buf[4+strlen(nombreCiudad)] = strlen(nombreEquipo)+1;
	memcpy(&(buf[5+strlen(nombreCiudad)]),nombreEquipo,strlen(nombreEquipo)+1);

	iLargo= strlen(nombreCiudad) + strlen(nombreEquipo) + 6;

	if((sendall(iSocketCiudad,buf,iLargo))==-1)
	{
		fvStdLog("equipobib.c",705,TLError,"Error al Enviar Permiso a la Ciudad");
		return -1;
	}
	free(buf);

	FD_ZERO(&maefds);
	FD_ZERO(&readfds);
	FD_SET(iSocketCiudad,&maefds);
	iSalir = 0;

	while(iSalir==0)
	{
		readfds = maefds;
		if(select(iSocketCiudad+1,&readfds,NULL,NULL,NULL)==-1)
		{
			fvStdLog("equipobib.c",720,TLError,"Error en Select");
			return -1;
		}
		if(FD_ISSET(iSocketCiudad,&readfds))
		{
			if((iMsje=fiEquReciboRespuesta(iSocketCiudad,cpIpEquipo,iPuertoEquipo))==-1)
			{
				fvStdLog("equipobib.c",727,TLError,"Error al Recibir Mensaje de la CIudad");
				return -1;
			}
			return iMsje;
		}
	}
	return -1;
}
/*-------------------------------------------------------------------------------------------------------------------*/
int fiEquReciboRespuesta(int iSocketCiudad,char *cpIpEquipo, int * iPuertoEquipo)
{
	int iMsje;
	char * cpCabecera;
	char cpMensaje[6];

	cpCabecera=(char *)malloc(2);
	while(1)
	{

		if((iMsje=recvAll(iSocketCiudad,cpCabecera,2,0))==-1)
		{
			fvStdLog("equipobib.c",746,TLError,"Error al Recibir Cabecera de Permiso");
			return -1;
		}
		else if(iMsje == 0)
		{
			fvStdLog("equipobib.c",751,TLError,"Error la Ciudad se Desconecto  =( ");
			return  -1;
		}

		if(!strncmp(cpCabecera,"RJ",2))
		{
			if((recvAll(iSocketCiudad,cpMensaje,6,0))==-1)
			{
				fvStdLog("equipobib.c",759,TLError,"Error al Recibir IP Puerto del EQUIPO (RJ)");
				return -1;
			}
			fiRouPrintfIP(cpMensaje, cpIpEquipo, iPuertoEquipo);
			return 1;
		}
		else if(!strncmp(cpCabecera,"NJ",2))
		{
			fvStdLog("equipobib.c",768,TLMensaje,"El Equipo No se Encuentra");
			return 0;
		}
		else if(!strncmp(cpCabecera,"MJ",2))
		{
			if((recvAll(iSocketCiudad,cpMensaje,6,0))==-1)
			{
				fvStdLog("equipobib.c",775,TLError,"Error al Recibir IP Puerto del EQUIPO (RJ)");
				return -1;
			}
			fiRouPrintfIP(cpMensaje, cpIpEquipo, iPuertoEquipo);
			return 2;
		}
		else if(!strncmp(cpCabecera,"RE",2))
		{
			if((recvAll(iSocketCiudad,cpMensaje,6,0))==-1)
			{
				fvStdLog("equipobib.c",775,TLError,"Error al Recibir IP Puerto de la Ciudad (RE)");
			}
		}
		else if(!strncmp(cpCabecera,"T2",2))
		{
			fvStdLog("equipobib.c",775,TLError,"me fui al carajo");
			exit(1);	
		}
		else
		{
			fvStdLog("equipobib.c",785,TLError,"Error la CIudad Me mando Verdura: %s",cpCabecera);
			for(iMsje = 0;iMsje<10;iMsje++)
			{
				printf("\a\a\a\a\a\a\a\a\a\a\a");
				sleep(1);
			}
			getchar();
			return -1;
		}
	}
}
/*--------------------------------------------------------------------------------------------------------------------*/
int fiEquCalcularResultado()
{
	int iResul;

	srand (time(NULL));
	iResul = (rand()%3)+1;

	if (iResul == 1)
		iResul = 'E';

	else if (iResul == 2)
		iResul = 'P';

	else
		iResul = 'G';

	return iResul;
}
/*--------------------------------------------------------------------------------------------------------------------*/
int fiEquCansancio(void)
{
	srand (time(NULL));
	return ((rand()%5)+1);
}
/*--------------------------------------------------------------------------------------------------------------------*/
int fiEquDescansarTotalmente(int iSocketCiudad,int * iCansancio, int iCansancioMax,int tTiempoMaxCansado, char * cpMiNombre)
{
	char * cpIpCde;
	char * cpTeclado;
	int iPuertoCde;
	int tTiempoDeUso;
	int iSalir;
	int iSocketCde;
	int iMjeCde;
	int iEstadoCde;
	fd_set maefds;
	fd_set readfds;

	struct timeval tv;

	cpIpCde = (char *)malloc(16);
	cpTeclado= (char*) malloc (22);

	if(fiEquPedidoDeCdeLI(iSocketCiudad,cpIpCde,&iPuertoCde)==-1) /*Pido el CDE a la Ciudad que me devuelve Ip y Puerto*/
	{
		fvStdLog("equipobib.c",835,TLError,"Error al Pedir el CDE a IP:%s Puerto:%d",cpIpCde,iPuertoCde);
		return -1;
	}
	else
	{
		tTiempoDeUso = fiEquAsignarTiempo(*iCansancio,iCansancioMax,tTiempoMaxCansado);
		if((iSocketCde=fiEquConectar(cpIpCde,iPuertoCde))==-1)/*Me conecta al CDE y me devuelve el socket*/
		{
			fvStdLog("equipobib.c",843,TLError,"El CDE con IP:%s PUERTO:%d no se encuentra",cpIpCde,iPuertoCde);
			free(cpIpCde);
			*iCansancio = 0;
		}
		else
		{
			fvStdLog("equipobib.c",849,TLMensaje,"Coneccion Establecida con el CDE");
			free(cpIpCde);
			if((iEstadoCde=fiEquAvisarTiempoCde(iSocketCde,tTiempoDeUso, cpMiNombre))==-1)
				fvStdLog("equipobib.c",852,TLError,"Error al Pedir Campo al CDE");
			else if(iEstadoCde == 3) /*Se Desconecto el CDE*/
			{
				*iCansancio = 0;
				fvStdLog("equipobib.c",856,TLError,"SE fue!!! SE fue el CDE =(");
				return 0;
			}
			FD_ZERO(&maefds);
			FD_ZERO(&readfds);
			FD_SET(iSocketCde,&maefds);
			tv.tv_sec = tTiempoDeUso;
			tv.tv_usec = 0;
			iSalir = 0;

			while(iSalir == 0)
			{
				readfds = maefds;

				if (( tv.tv_sec == 0)&&(tv.tv_usec == 0))/*Estoy en la COLA*/
				{
					tv.tv_sec = tTiempoDeUso;
					tv.tv_usec = 0;
				}
				if(select(iSocketCde+1, &readfds, NULL, NULL, &tv)==-1)
				{
					fvStdLog("equipobib.c",877,TLError,"Error en Select");
					exit(1);
				}
				if(FD_ISSET(iSocketCde,&readfds))
				{
					if((iMjeCde=fiEquReciboMensajeCde(iSocketCde))==-1)/*Recibo Mensaje del CDE*/
					{
						fvStdLog("equipobib.c",884,TLError,"Error al Recibir Mensaje del CDE");
						exit(1);
					}
					else if(iMjeCde == 0)
					{
						fvStdLog("equipobib.c",889,TLError,"El CDE se desconecto");
						close(iSocketCde);
						FD_CLR(iSocketCde,&maefds);
						*iCansancio = fiEquCalculoCansancio(iEstadoCde,*iCansancio,tv,tTiempoMaxCansado);
						iSalir = 1;
					}
					else /*El CDE me avisa que ya hay lugar*/
					{
						iEstadoCde = 1;
						tv.tv_sec = tTiempoDeUso;
						tv.tv_usec = 0;
					}
				}
				else /*Sali pork se cumplio el tiempo*/
				{
					if(iEstadoCde==1) /*Si estaba entrenando*/
					{
						*iCansancio = 0;
						close(iSocketCde);
						fvStdLog("equipobib.c",908,TLMensaje,"Fin de Entrenamiento");
						iSalir = 1;
					}/*Si no empiezo de nuevo*/
				}
			}
		}
	}
	return 0;
}
/*--------------------------------------------------------------------------------------------------------------------*/
int contar(char * string, char caracter, int cantidad)
{
	int i;
	int n;

	for (i=0, n=0; string[i]!='\0' && n<cantidad; i++)
	{
		if(string[i] == caracter)
			n++;
	}
	return(i);
}
/*-------------------------------------------------------------------------------------------------------------------*/
void reemplazar(char* cadena, char orig, char cambiado)
{
	int i;
	for (i=0; cadena[i]!='\0'; i++)
	{
		if(cadena[i] == orig)
			cadena[i] = cambiado;
	}
}
/*-------------------------------------------------------------------------------------------------------------------*/
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
/*---------------------------------------------------------------------------------------------------------------------*/
int fiEquEscucharAEquipos(struct sockaddr * miDir)
{
	int iSockfd;
	int yes;

	yes=1;

	if((iSockfd= socket(AF_INET,SOCK_STREAM,0))<0)
	{
		return -1;
	}

	if(setsockopt(iSockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		perror("setsockopt");
		exit(1);
	}

	if(bind(iSockfd, (struct sockaddr*)miDir, sizeof(struct sockaddr))==-1)
	{
		return -1;
	}

	if((listen(iSockfd, 5))<0)
	{
		return -1;
	}

	return iSockfd;
}
/*---------------------------------------------------------------------------------------------------------------------*/
int fiEquRtaCiudadOcupado(int iSocket)
{
	if(sendall(iSocket, "OC", 2)<0)
	{
		return -1;
	}
	return 1;
}
/*---------------------------------------------------------------------------------------------------------------------*/
int fiEquRtaCiudadLibre(int iSocket, char * cpIp, int iPuerto)
{
	char cpIpPuerto[8];

	memcpy(cpIpPuerto, "LB", 2);

	fiRouScanfIP(&(cpIpPuerto[2]), cpIp, iPuerto);
//	sscanf(cpIp, "%u.%u.%u.%u", (unsigned char *)&(cpIpPuerto[2]), (unsigned char *)&(cpIpPuerto[3]), (unsigned char *)&(cpIpPuerto[4]), (unsigned char *)&(cpIpPuerto[5]));
//	cpIpPuerto[6] = (unsigned char)(iPuerto % 256);
//	cpIpPuerto[7] = (unsigned char)(iPuerto / 256);

	if(send(iSocket,&(cpIpPuerto[0]), 8,0)==-1)
	{
		return -1;
	}
	return 1;
}
/*---------------------------------------------------------------------------------------------------------------------*/
int fiEquAceptoEquipo(int iSocket)
{
	int iSockfd;
	socklen_t  iLargo;
	struct sockaddr_in eqDir;

	iLargo= sizeof(struct sockaddr);
	if((iSockfd=accept(iSocket,(struct sockaddr *)&eqDir, &iLargo))==-1)
		return -1;
	return iSockfd;
}
/*----------------------------------------------------------------------------------------------------------------------*/
int fiEquReciboMensajeCde(int iSocket)
{
	int iEstado;
	char * cpMensaje;

	cpMensaje = (char*)malloc(3);

	if((iEstado=recvAll(iSocket, cpMensaje, 2, 0))<0)   /*ERROR*/
		return -1;

	else if (iEstado == 0)    /*se desconecto*/
		return 0;

	else if(!strncmp(cpMensaje, "RC",2))  /*empiezo a entrenar*/
	{
		free(cpMensaje);
		return 1;
	}
	//	else if(!strcmp(cpMensaje, "NC"))  /*me Encola*/
	//	{
	//		free(cpMensaje);
	//		return 2;
	//	}
	cpMensaje[2]='\0';
	fvStdLog("equipobib.c",1054,TLError,"Error el CDE me envio Basura: %s",cpMensaje);
	free(cpMensaje);
	return 0;

}
/*-------------------------------------------------------------------------------------------------------------------*/
int fiEquReciboRespuestaCde(int iSocketCde)
{
	int iEstado;
	char * cpMensaje;

	cpMensaje = (char*)malloc(3);

	if((iEstado=recvAll(iSocketCde, cpMensaje, 2, 0))<0)   /*ERROR*/
		return -1;
	else if(!strncmp(cpMensaje, "RC",2))  /*empiezo a entrenar*/
	{
		free(cpMensaje);
		return 1;
	}
	else if(!strncmp(cpMensaje, "NC",2))   /*me encolo*/
	{
		free(cpMensaje);
		return 0;
	}
	else if(!iEstado)
	{
		free(cpMensaje);
		return 3;
	}
	cpMensaje[2]='\0';
	fvStdLog("equipobib.c",1085,TLError,"Error el CDE me envio Basura: %s",cpMensaje);
	free(cpMensaje);
	return 3;
}
/*--------------------------------------------------------------------------------------------------------------------*/
int fiEquHandShake(int iSocket)
{
	int iRec;
	fd_set maefds;

	if(sendall(iSocket, "HS", 2)==-1)
	{
		fvStdLog("equipobib.c",1097,TLError,"Error al enviarle el Hand Shake");
		return -1;
	}

	FD_ZERO(&maefds);
	FD_SET(iSocket, &maefds);

	if(select(iSocket + 1, &maefds, NULL, NULL, NULL)<0)
	{
		fvStdLog("equipobib.c",1106,TLError,"Error en Select");
		return -1;

	}
	if(FD_ISSET(iSocket,&maefds))
	{
		if((iRec=fiEquReciboRtaHS(iSocket))==-1)
		{
			fvStdLog("equipobib.c",1114,TLError,"Error al recibir rta Hand Shake");
			return -1;

		}
		else if (iRec==0)
		{
			fvStdLog("equipobib.c",1120,TLError,"Se desconecto equipo!!!");
			return -1;
		}
		else if(iRec==1)
		{
			fvStdLog("equipobib.c",1125,TLMensaje,"Hand Shake OK!!!");
			return 1;
		}
		else
		{
			fvStdLog("equipobib.c",1130,TLError,"me conecte a cualquier cosa!!!");
			return -1;
		}
	}
	return -1;
}
/*---------------------------------------------------------------------------------------------------------------------*/
int fiEquReciboRtaHS(int iSocket)
{
	int iMje;
	char * cpMensaje;

	cpMensaje= (char*) malloc (3);

	if((iMje=recvAll(iSocket, cpMensaje, 2, 0))==-1)
	{
		return -1;
	}
	else if(iMje==0)
	{
		return 0;
	}
	cpMensaje[2]= '\0';
	if(!strcmp(cpMensaje, "HS"))
		return 1;
	else
		return 2;
}
/*---------------------------------------------------------------------------------------------------------------------*/
int fiEquPuedoJugar(int iEstadoCde, int * iCansancio,int iMaxCansancio, struct timeval tv, int tvMax,int iEsperando)
{
	if(iEsperando == 1)
		return 0;

	*iCansancio= fiEquCalculoCansancio(iEstadoCde, *iCansancio,tv, tvMax);

	if(*iCansancio<iMaxCansancio)   /*puedo jugar*/
		return 1;
	else
		return 0;               /*todavia estoy cansado*/
}
/*----------------------------------------------------------------------------------------------------------------------*/
int fiEquCalculoCansancio(int iEstadoCde, int iCansancio, struct timeval tv,int tvMax)
{
	int iDescansoParcial;

	if(iEstadoCde == 1)
		iDescansoParcial= (tv.tv_sec * iCansancio)/ tvMax;
	else
		iDescansoParcial=iCansancio;

	return iDescansoParcial;

}
/*---------------------------------------------------------------------------------------------------------------------*/

int fiEquAsignarTiempo(int iCansancio, int iMaxCansancio, int iTiempo)
{
	int iTiempoDescanso;

	if(iCansancio >= iMaxCansancio)
		iTiempoDescanso= (iCansancio * iTiempo) / iMaxCansancio;
	else
		iTiempoDescanso= iTiempo*2;

	return iTiempoDescanso;
}
/*---------------------------------------------------------------------------------------------------------------------*/
int fiEquEnviarDatos(int iSocketCiudad,char *cpMiIp,int iMiPuerto,char * cpMiNombre)
{
	char * buf;
	unsigned char cpIpPuerto[6];

	buf = (char *)malloc(strlen(cpMiNombre)+4);
	strcpy(buf,"DE");
	buf[2]=strlen(cpMiNombre)+1;
	fvEquConcatenar(buf, cpMiNombre,3,strlen(cpMiNombre)+1);
	if(sendall(iSocketCiudad, buf ,strlen(cpMiNombre) + 4)==-1)
	{
		fvStdLog("equipobib.c",1209,TLError,"Error al enviar Mis Datos a la Ciudad");
		return -1;
	}

	//strcpy(sMensaje->ip, cpMiIp);

	fiRouScanfIP(&(cpIpPuerto[0]), cpMiIp, iMiPuerto);
//	sscanf(cpMiIp, "%u.%u.%u.%u", (unsigned int *)&(cpIpPuerto[0]), (unsigned int *)&(cpIpPuerto[1]), (unsigned int *)&(cpIpPuerto[2]), (unsigned int *)&(cpIpPuerto[3]));
//	cpIpPuerto[4] = iMiPuerto % 256;
//	cpIpPuerto[5] = iMiPuerto / 256;

	if(send(iSocketCiudad,&(cpIpPuerto[0]),6,0)==-1)
	{
		fvStdLog("equipobib.c",1221,TLError,"Error al enviar Mis Datos a la Ciudad");
		return -1;
	}

	return 1;
}
/*---------------------------------------------------------------------------------------------------------------------*/ 
void fvEquConcatenar(char* cpReceptor, char* cpEmisor, int iDesde, int iLargo)
{
	int i;
	for(i=0;i<iLargo;i++)
		cpReceptor[iDesde + i] = cpEmisor[i];
}
/*---------------------------------------------------------------------------------------------------------------------*/
int fiEquAvisarTiempoCde( int iSocket, int tTiempo, char *cpMiNombre)
{

	struct mjeTiempo  mje;
	fd_set maefds;
	fd_set readfds;
	char tamanio;
	int iMsje;
	
	tamanio = strlen(cpMiNombre)+1;
	strcpy(mje.cabecera, "PC");
	mje.tiempo= tTiempo;

	if(send(iSocket, &mje,sizeof(struct mjeTiempo),0)<0)
	{
		fvStdLog("equipobib.c",1248,TLError,"Error al mandar el tiempo de descanso..La puta madre!!! jaja");
		return -1;
	}

	if(send(iSocket, &tamanio,1,0)<0)
	{
		fvStdLog("equipobib.c",1248,TLError,"Error al mandar el tiempo de descanso..La puta madre!!! jaja");
		return -1;
	}
	if(send(iSocket, cpMiNombre,tamanio,0)<0)
	{
		fvStdLog("equipobib.c",1248,TLError,"Error al mandar el tiempo de descanso..La puta madre!!! jaja");
		return -1;
	}
	FD_ZERO(&maefds);
	FD_ZERO(&readfds);
	FD_SET(iSocket,&maefds);

	while(1)
	{
		readfds = maefds;
		if(select(iSocket+1,&readfds,NULL,NULL,NULL)==-1)
		{
			fvStdLog("equipobib.c",1261,TLError,"Error en Select");
			return -1;
		}
		if(FD_ISSET(iSocket,&readfds))
		{
			if((iMsje=fiEquReciboRespuestaCde(iSocket))==-1)
			{
				fvStdLog("equipobib.c",1268,TLError,"Error al Recibir Mensaje del Cde");
				return -1;
			}
			return iMsje;                
		}
	}
}
/*------------------------------------------------------------------------------------------------------------------------*/
int fiEquPedidoDeCde(int iSocket)
{
	if(sendall(iSocket, "PE", 2)<0)
	{
		fvStdLog("equipobib.c",1280,TLError,"Error al pedir permiso de entrenamiento a la ciudad");
		return -1;
	}
	return 0;
}
/*-------------------------------------------------------------------------------------------------------------*/
int fiEquPedidoDeCdeLI(int iSocket,char * cpIpCde, int * iPuertoCde)
{
	int iMsje;
	int iSalir;
	fd_set maefds;
	fd_set readfds;

	FD_ZERO(&maefds);
	FD_ZERO(&readfds);

	if(sendall(iSocket, "PE", 2)<0)
	{
		fvStdLog("equipobib.c",1298,TLError,"Error al pedir permiso de entrenamiento a la ciudad");
		return -1;
	}

	FD_SET(iSocket,&maefds);
	iSalir = 0;

	while(iSalir==0)
	{
		readfds = maefds;
		if(select(iSocket+1,&readfds,NULL,NULL,NULL)==-1)
		{
			fvStdLog("equipobib.c",1310,TLError,"Error en Select");
			return -1;
		}
		if(FD_ISSET(iSocket,&readfds))
		{
			if((iMsje=fiEquRtaEntrenamientoLI(iSocket,cpIpCde,iPuertoCde))<=0)
			{
				fvStdLog("equipobib.c",1317,TLError,"Error al Recibir Mensaje de la Ciudad");
				return -1;
			}
			iSalir = 1;
		}
	}
	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------*/
int fiEquRtaEntrenamiento(int iSocket, char * cpIp, int *iPuerto)
{
	int iRec;
	unsigned char cpIpPuerto[6];

	if((iRec=recvAll(iSocket, cpIpPuerto, 6, 0))==-1)
	{
		fvStdLog("equipobib.c",1334,TLError,"Error al Recibir datos de la Ciudad");
		return -1;
	}

	else if(iRec==0)
	{
		fvStdLog("equipobib.c",1340,TLError,"La ciudad se Desconecto");
		return 0;
	}

	fiRouPrintfIP(cpIpPuerto, cpIp, iPuerto);
	//sprintf(cpIp, "%u.%u.%u.%u", (unsigned char)(cpIpPuerto[0]), (unsigned char)(cpIpPuerto[1]), (unsigned char)(cpIpPuerto[2]), (unsigned char)(cpIpPuerto[3]));
	//*iPuerto = (unsigned char)cpIpPuerto[4] + ((unsigned char)cpIpPuerto[5]) * 256;

	return 1;
}
/*-------------------------------------------------------------------------------------------------------------------------*/
int fiEquRtaEntrenamientoLI(int iSocket, char * cpIp, int *iPuerto)
{
	int iRec;
	unsigned char cpIpPuerto[8];

	if((iRec=recvAll(iSocket, cpIpPuerto, 8, 0))==-1)
	{
		fvStdLog("equipobib.c",1357,TLError,"Error al Recibir datos de la Ciudad");
		return -1;
	}
	else if(iRec==0)
	{
		fvStdLog("equipobib.c",1362,TLError,"La ciudad se Desconecto");
		return 0;
	}

		fiRouPrintfIP(&(cpIpPuerto[2]), cpIp, iPuerto);
	//sprintf(cpIp, "%u.%u.%u.%u", (unsigned char)(cpIpPuerto[2]), (unsigned char)(cpIpPuerto[3]), (unsigned char)(cpIpPuerto[4]), (unsigned char)(cpIpPuerto[5]));
	//*iPuerto = (unsigned char)cpIpPuerto[6] + (unsigned char)cpIpPuerto[7] * 256;

	return 1;
}
/*-------------------------------------------------------------------------------------------------------------------------*/
struct DatosMigrar fiEquManejarMigracion(int iSocket, char *cpIp, int iPuerto, char ** cpMiNombre,struct lLevInt ** lLevExt,int * iCant)
{
	char cpCabecera[2];
	char cLargo;
	struct DatosMigrar sDatos;

	if(sendall(iSocket, "PM", 2)<0)
	{
		fvStdLog("equipobib.c",1380,TLError,"Error al pedir datos de migracion a la ciudad");
		exit(0);
	}
	if(recvAll(iSocket,cpCabecera, 2, 0)==-1)
	{
		fvStdLog("equipobib.c",1385,TLError,"Error al Recibir Cabecera de la Ciudad");
		exit(0);
	}
	if(strncmp(cpCabecera,"DM",2))
	{
		fvStdLog("equipobib.c",1390,TLError,"Error la Ciudad me Envio Cabecera Erronea");
		exit(0);
	}
	if((*iCant=fiEquRecibirLevE(iSocket,lLevExt))==-1)	
	{
		fvStdLog("equipobib.c",1395,TLError,"Error al Recibir LEV EXTERNA");	
		exit(1);
	}
	if(recvAll(iSocket, &cLargo, 1, 0)==-1)
	{
		fvStdLog("equipobib.c",1400,TLError,"Error al Recibir Largo de la Ciudad");
		exit(0);
	}
	*cpMiNombre = (char *)malloc(cLargo);
	if(recvAll(iSocket, *cpMiNombre, cLargo, 0)==-1)
	{
		fvStdLog("equipobib.c",1406,TLError,"Error al Recibir Mi Nombre de la Ciudad");
		exit(0);
	}
	if(recv(iSocket, &sDatos, sizeof(struct DatosMigrar),0)==-1)
	{
		fvStdLog("equipobib.c",1411,TLError,"Error al Recibir datos de la Ciudad");
		exit(0);
	}
	if((!strcmp(sDatos.cpIpCiudad,cpIp))&&(sDatos.iPuertoCiudad==iPuerto))
	{
		return sDatos;
	}

	if(fiEquJugarLevE(iSocket,lLevExt,sDatos.iDurPartido,&sDatos.iCansancio,sDatos.iMaxCansancio,sDatos.iTiempoMaxCansado,sDatos.iMaxTiempo,sDatos.cpMiIp,sDatos.iMiPuerto,sDatos.cpIpCiudad,sDatos.iPuertoCiudad,*cpMiNombre,cpIp,iPuerto)==-1)
		exit(1);	

}
/*---------------------------------------------------------------------------------------------------------------------------*/
void fiEquSacarEquipoLev(struct lLevInt ** lista,char * cpNombreCiudad, char * cpNombreEquipo)
{
	struct lLevInt * sAux;
	struct lLevInt * sAux2;

	if (((*lista) != NULL) && (!strcmp((*lista)->cpNombreCiudad,cpNombreCiudad)) && (!strcmp((*lista)->cpNombreEquipo,cpNombreEquipo)))
	{
		sAux = (*lista);
		(*lista) = sAux->siguiente;
		free(sAux->cpNombreCiudad);
		free(sAux->cpNombreEquipo);
		free(sAux);
	}
	else if((*lista) != NULL)
	{
		sAux = (*lista);
		sAux2 = sAux->siguiente;
		while(sAux2!=NULL)
		{
			if((!strcmp(sAux2->cpNombreCiudad,cpNombreCiudad)) && (!strcmp(sAux2->cpNombreEquipo,cpNombreEquipo)))
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
/*-----------------------------------------------------------------------------------------------------------------------------*/

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
	strcpy(ip, itoa((unsigned char)ipPuerto[0], str));
	strcat(ip, ".");
	strcat(ip, itoa((unsigned char)ipPuerto[1], str));
	strcat(ip, ".");
	strcat(ip, itoa((unsigned char)ipPuerto[2], str));
	strcat(ip, ".");
	strcat(ip, itoa((unsigned char)ipPuerto[3], str));
	
	(*puerto)= ((unsigned char) ipPuerto[4]) + (((unsigned char) ipPuerto[5]) * 256);

}

char * itoa(int i, char * str)
{
	sprintf(str, "%d", i);
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
