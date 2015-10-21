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
#include "equipobib.h"
#include "logs.h"
#include "configuracion.h"

#define STDIN 0

int main(int argc, char *argv[])
{
	char* cpMiIp;
	char* cpIpCiudad;
	int iMiPuerto, iPuertoCiudad;
	int iSocketCiudad;
	int iSocketEscucha;
	int iSocketEquipo;
	int iSocketCde;
	int iMjeEqu;
	int iSalir;
	struct timeval tv;
	fd_set maefds;
	fd_set readfds;
	struct lLevInt * lLevInt=NULL;
	struct lLevInt * lLevExt=NULL;
	int  iPuertoCde;
	char * cpIpCde;
	int fdmax;
	int iJugando;
	int tMaxTiempo;
	int tDurPartido;
	unsigned int tTiempoDeUso;
	int iCabecera;
	int tTiempoMaxCansado;
	struct sockaddr_in miDir_addr;
	char * cpMiNombre;
	int iEstadoCde;
	int iCansancio;
	int iMaxCansancio;
	int iPendiente;
	int iMjeCde;
	char * buf;
	char * cpTeclado;
	char * cpMiPuerto;
	char * cpPuertoCiudad;
	char * cpMaxTiempo;
	char * cpDurPartido;
	char * cpTiempoMaxCansado;
	char * cpMaxCansancio;
	char * cpNombreLogs;
	int iEsperando=0;
	struct DatosMigrar sDatosMigrar;
	struct lLevInt * lLevAux;
	int iCant;
	int iCantEsp;

	FD_ZERO(&maefds);
	FD_ZERO(&readfds);

	/*MALLOCS*/
	/*-----------------------------------------------------------------------------------------------------------------*/
	cpIpCde = (char *) malloc (16);
	cpIpCiudad = (char *)malloc(16);
	cpMiIp =(char *)malloc(16);
	/*----------------------------------------------------------------------------------------------------------------*/

	if (argc > 1)
	{
		sleep(1);

		fvStdIniciarLogs("EquipoMigrado","migrado.log");
		strcpy(cpIpCiudad, argv[1]);
		iPuertoCiudad = atoi(argv[2]);
		iSocketCiudad=fiEquConectar(cpIpCiudad,iPuertoCiudad);
		fvStdLog("equipo.c",91,TLMensaje,"Coneccion OK con Ciudad IP:%s Puerto:%d Volviendo de Migracion", cpIpCiudad,iPuertoCiudad);
		sDatosMigrar=fiEquManejarMigracion(iSocketCiudad, cpIpCiudad, iPuertoCiudad,&cpMiNombre,&lLevExt,&iCant);

		strcpy(cpMiIp,sDatosMigrar.cpMiIp);
		iMiPuerto = sDatosMigrar.iMiPuerto;
		tMaxTiempo = sDatosMigrar.iMaxTiempo;
		tDurPartido = sDatosMigrar.iDurPartido;
		tTiempoMaxCansado = sDatosMigrar.iTiempoMaxCansado;
		iMaxCansancio = sDatosMigrar.iMaxCansancio;
		iCansancio=sDatosMigrar.iCansancio;

		cpNombreLogs=(char *)malloc(strlen(cpMiNombre)+5);
		strcpy(cpNombreLogs,cpMiNombre);
		strcat(cpNombreLogs,".log");

		fvStdIniciarLogs(cpMiNombre,cpNombreLogs);

		if(fiEquEnviarDatos(iSocketCiudad,cpMiIp,iMiPuerto,cpMiNombre)==-1)
		{
			fvStdLog("equipo.c",111,TLError,"Error al Intentar Enviar Datos a la Ciudad");
			exit(1);
		}
		if(fiEquEnviarResultados(iSocketCiudad,&lLevExt,iCant,"LE")==-1)
		{
			fvStdLog("equipo.c",116,TLError,"Error al Enviar Resultados a la Ciudad");
			return -1;
		}
		//*lLevExt = NULL;
	}	
	else
	{	

		fvStdTomarConfiguracion("./CONFIGE","IP", &cpMiIp);
		fvStdTomarConfiguracion("./CONFIGE","PUERTO", &cpMiPuerto);
		iMiPuerto = atoi(cpMiPuerto);
		fvStdTomarConfiguracion("./CONFIGE","IPCIUDAD", &cpIpCiudad);
		fvStdTomarConfiguracion("./CONFIGE","PUERTOCIUDAD", &cpPuertoCiudad);
		iPuertoCiudad = atoi(cpPuertoCiudad);
		fvStdTomarConfiguracion("./CONFIGE","MAXTIEMPO", &cpMaxTiempo);
		tMaxTiempo = atoi(cpMaxTiempo);
		fvStdTomarConfiguracion("./CONFIGE","NOMBRE", &cpMiNombre);
		fvStdTomarConfiguracion("./CONFIGE","DURACIONP", &cpDurPartido);
		tDurPartido = atoi(cpDurPartido);
		fvStdTomarConfiguracion("./CONFIGE","MAXTCANSADO", &cpTiempoMaxCansado);
		tTiempoMaxCansado = atoi(cpTiempoMaxCansado);
		fvStdTomarConfiguracion("./CONFIGE","MAXCANSANCIO", &cpMaxCansancio);
		iMaxCansancio = atoi(cpMaxCansancio);
		iCansancio = 0;

		cpNombreLogs=(char *)malloc(strlen(cpMiNombre)+5);
		strcpy(cpNombreLogs,cpMiNombre);
		strcat(cpNombreLogs,".log");

		fvStdIniciarLogs(cpMiNombre,cpNombreLogs);

		if((iSocketCiudad=fiEquConectar(cpIpCiudad,iPuertoCiudad)) == -1)/*Nos Conecta a la Ciudad y devuelve el SOcket*/	
		{
			fvStdLog("equipo.c",149,TLError,"Error al Intentar conectarse a la Ciudad con IP:%s Puerto:%d", cpIpCiudad, iPuertoCiudad);
			exit(1);
		}	   
		fvStdLog("equipo.c",152,TLMensaje,"Coneccion OK con Ciudad IP:%s Puerto:%d", cpIpCiudad,iPuertoCiudad);

		if(fiEquEnviarDatos(iSocketCiudad,cpMiIp,iMiPuerto,cpMiNombre)==-1)
		{
			fvStdLog("equipo.c",156,TLError,"Error al Intentar Enviar Datos a la Ciudad");
			exit(1);
		}
	}

	strncpy(argv[0],cpMiNombre, strlen(cpMiNombre)+1); /*Cambio el nombre al proceso con el nombre del Equipo*/

	fdmax = iSocketCiudad;

	miDir_addr.sin_family=AF_INET;
	miDir_addr.sin_port= htons(iMiPuerto);
	inet_aton (cpMiIp, &(miDir_addr.sin_addr));
	bzero (&(miDir_addr.sin_zero), 8);

	if((iSocketEscucha=fiEquEscucharAEquipos((struct sockaddr *)&miDir_addr)) == -1)/*Nos quedamos escuchando en un socket a Equipos*/
	{
		fvStdLog("equipo.c",172,TLError,"Error al escuchar desde IP:%s Puerto:%d", cpMiIp, iMiPuerto);
		exit(1);
	}	   
	fvStdLog("equipo.c",175,TLMensaje,"Escuchando desde IP:%s Puerto:%d",cpMiIp,iMiPuerto);
	fdmax = iSocketEscucha;

	FD_SET(iSocketCiudad, &maefds);
	FD_SET(iSocketEscucha, &maefds);

	iJugando = 0; /*Sin Jugar*/
	iEstadoCde = 2; /*Fuera del CDE*/
	iSalir = 0;
	tv.tv_sec = tMaxTiempo; 
	tv.tv_usec = 0 ;
	iPendiente = 0;
	iSocketCde = 1;

	while(iSalir == 0)       
	{
		readfds = maefds;

		if((iEstadoCde == 1)&&((tv.tv_sec == 0)&&(tv.tv_usec == 0)))/*Estoy Entrenando en el CDE*/
		{
			tv.tv_sec = tTiempoDeUso ; /*Es el tiempo que entrenare*/
			tv.tv_usec = 0 ;
		}
		else if((iEstadoCde!=1)&&((tv.tv_sec == 0)&&(tv.tv_usec == 0)))
		{
			tv.tv_sec = tMaxTiempo ; /*Es el Max tiempo que esperare sin hacer nada*/
			tv.tv_usec = 0 ;
		}

		if(select(fdmax+1, &readfds, NULL, NULL, &tv)==-1)
		{
			fvStdLog("equipo.c",206,TLError,"Error en Select");
			exit(1);
		}
		if(FD_ISSET(iSocketCiudad,&readfds)) /*La ciudad nos quiere enviar un mensaje*/
		{
			if((iCabecera=fiEquReciboCabecera(iSocketCiudad))==-1)
			{
				fvStdLog("equipo.c",213,TLError,"Error al recibir Cabecera de Mensaje Enviado por la Ciudad");
				exit(1);
			}
			switch(iCabecera)
			{
				case 0:
					/*SALIR*/
					exit(0);
					break;
				case 1:
					/*LEV INTERNA*/
					lLevInt  = NULL;
					if(fiEquRecibirLevI(iSocketCiudad,&lLevInt)==-1)	

					{
						fvStdLog("equipo.c",228,TLError,"Error al Recibir LEV INTERNA");	
						exit(1);
					}

					if(iEstadoCde == 2)/*No estoy en el CDE*/
					{
						if(iEsperando==0)
						{
							if(fiEquJugarLevI(lLevInt,tDurPartido,&iCansancio,iMaxCansancio,iSocketCiudad,tTiempoMaxCansado, cpMiNombre)==-1)	
								exit(1);
							iPendiente = 0;
						}
						else if(iEsperando == 1)
							iPendiente=1;
					}
					else if(iEstadoCde != 2)
					{
						if(fiEquPuedoJugar(iEstadoCde,&iCansancio,iMaxCansancio,tv,tTiempoDeUso,iEsperando)==0)
							iPendiente = 1;
						else
						{
							close(iSocketCde);
							FD_CLR(iSocketCde,&maefds);
							fvStdLog("equipo.c",251,TLMensaje,"Sali del CDE para Jugar LEV INTERNA");	
							iEstadoCde = 2;
							if(fiEquJugarLevI(lLevInt,tDurPartido,&iCansancio,iMaxCansancio,iSocketCiudad,tTiempoMaxCansado, cpMiNombre)==-1)
								exit(1);
							iPendiente = 0;
						}
					}
					break;
				case 2:
					/*LEV EXTERNA*/
					if(fiEquRecibirLevE(iSocketCiudad,&lLevExt)==-1)	
					{
						fvStdLog("equipo.c",263,TLError,"Error al Recibir LEV EXTERNA");	
						exit(1);
					}
					if(fiEquJugarLevE(iSocketCiudad,&lLevExt,tDurPartido,&iCansancio,iMaxCansancio,tTiempoMaxCansado,tMaxTiempo,cpMiIp,iMiPuerto,cpIpCiudad,iPuertoCiudad,cpMiNombre,cpIpCiudad,iPuertoCiudad)==-1)
						exit(1);	
					else
					{
							lLevAux = lLevExt;
							iCantEsp = 0;
							while(lLevAux)
							{
								iCantEsp++;	
								lLevAux=lLevAux->siguiente;
							}
							if(fiEquEnviarResultados(iSocketCiudad,&lLevExt,iCantEsp,"LE")==-1)
							{
									fvStdLog("equipo.c",279,TLError,"Error al Enviar Resultados a la Ciudad");
									return -1;
							}
					}


					break;

				case 3:
					/*Permiso Partido*/
					if(iJugando==1 || iEsperando ==1)
					{
						if(fiEquRtaCiudadOcupado(iSocketCiudad)==-1)	
						{
							fvStdLog("equipo.c",293,TLError,"Error al Responder a la Ciudad q estoy Ocupado");	
							exit(1);
						}
					}
					else
					{
						if(iEstadoCde == 2)/*No estoy en el CDE*/
						{
							if(fiEquRtaCiudadLibre(iSocketCiudad,cpMiIp,iMiPuerto)==-1)	
							{
								fvStdLog("equipo.c",303,TLError,"Error al Responder a la Ciudad q estoy libre");
								exit(1);
							}
							iJugando = 1;
						}
						else	 
						{
							if(fiEquPuedoJugar(iEstadoCde,&iCansancio,iMaxCansancio,tv,tTiempoDeUso,iEsperando)==0)
							{
								if(fiEquRtaCiudadOcupado(iSocketCiudad)==-1)	
								{
									fvStdLog("equipo.c",314,TLError,"Error al Responder a la Ciudad q estoy Ocupado");
									exit(1);
								}
							}
							else
							{
								close(iSocketCde);
								FD_CLR(iSocketCde,&maefds);
								fvStdLog("equipo.c",322,TLMensaje,"Sali del CDE para Jugar Partido de Local");
								iEstadoCde = 2;
								if(fiEquRtaCiudadLibre(iSocketCiudad,cpMiIp,iMiPuerto)==-1)	
								{
									fvStdLog("equipo.c",326,TLError,"Error al Responder a la Ciudad q estoy Libre");	
									exit(1);
								}
								iJugando = 1;
							}
						}
					}
					break;
				case 4:
					if(fiEquRtaEntrenamiento(iSocketCiudad,cpIpCde,&iPuertoCde)<=0)
					{
						fvStdLog("equipo.c",337,TLError,"Error al Recibir Mensaje de la Ciudad");
						return -1;
					}
					iEsperando = 0;

					tTiempoDeUso = fiEquAsignarTiempo(iCansancio,iMaxCansancio,tTiempoMaxCansado);
					if((iSocketCde=fiEquConectar(cpIpCde,iPuertoCde))==-1)/*Me conecta al CDE y me devuelve el socket*/
					{
						fvStdLog("equipo.c",345,TLError,"Error al Conectar a CDE con IP:%s PUERTO:%d",cpIpCde,iPuertoCde);
						iCansancio =0;
						if(fiEquRtaCiudadLibre(iSocketCiudad,cpMiIp,iMiPuerto)==-1)	
						{
							fvStdLog("equipo.c",349,TLError,"Error al Responder a la Ciudad q estoy Libre");	
							exit(1);
						}	
						//free(cpIpCde);
					}
					else if(iSocketCde != 0)
					{
						fvStdLog("equipo.c",356,TLMensaje,"Coneccion Establecida con el CDE");
						//free(cpIpCde);
						if((iEstadoCde=fiEquAvisarTiempoCde(iSocketCde,tTiempoDeUso, cpMiNombre))==-1)/*0 en COla, 1 Entrenando*/
						{
							fvStdLog("equipo.c",360,TLError,"Error al Pedir Campo al CDE");
							iCansancio=0;
						}
						else if(iEstadoCde==3)
						{
							iCansancio = 0;
							fvStdLog("equipo.c",366,TLError,"SE fue!!! SE fue el CDE =(");
							close(iSocketCde);
						}
						else
						{
							FD_SET(iSocketCde,&maefds);
							if(iSocketCde > fdmax)
								fdmax = iSocketCde;
							tv.tv_sec = tTiempoDeUso;
							tv.tv_usec = 0;
						}
					}
					if(iPendiente)
					{
						if(fiEquPuedoJugar(iEstadoCde,&iCansancio,iMaxCansancio,tv,tTiempoDeUso,iEsperando))
						{
							if(iSocketCde >= 0)
							{
								close(iSocketCde);
								FD_CLR(iSocketCde,&maefds);
								fvStdLog("equipo.c",251,TLMensaje,"Sali del CDE para Jugar LEV INTERNA");	
								iEstadoCde = 2;
							}
							if(fiEquJugarLevI(lLevInt,tDurPartido,&iCansancio,iMaxCansancio,iSocketCiudad,tTiempoMaxCansado, cpMiNombre)==-1)
								exit(1);
							iPendiente = 0;
						}
					}

					break;
				case 5:
					/*Basura*/
					fvStdLog("equipo.c",383,TLError,"La ciudad me bando Basura");	
					break;
			}
		}
		else if(FD_ISSET(iSocketEscucha,&readfds))/*Un Equipo se nos quiere conectar*/
		{
			if((iSocketEquipo = fiEquAceptoEquipo(iSocketEscucha))==-1)/*Acepta el equipo y devuelve el socket*/
			{
				fvStdLog("equipo.c",391,TLError,"Error al Aceptar Equipo");
				exit(1);
			}
			if((fiEquHandShake(iSocketEquipo))==-1)
			{
				fvStdLog("equipo.c",396,TLError,"Error el Socket:%d no respondio correctamente el Handshake",iSocketEquipo);
				close(iSocketEquipo);
			}
			else
			{ 
				iJugando = 1;
				FD_SET(iSocketEquipo, &maefds);
				if(iSocketEquipo>fdmax)
					fdmax=iSocketEquipo;
				fvStdLog("equipo.c",405,TLMensaje,"Comienza Partido de Local");
			}

		}
		else if((iEstadoCde!=2)&&(FD_ISSET(iSocketCde,&readfds)))/*El CDE nos quiero decir que tenemos lugar o se desconecto*/
		{
			if((iMjeCde=fiEquReciboMensajeCde(iSocketCde))==-1)/*Recibo Mensaje del CDE*/
			{
				fvStdLog("equipo.c",413,TLError,"Error al Recibir Mensaje del CDE");
				exit(1);
			}
			else if(iMjeCde == 0)
			{
				fvStdLog("equipo.c",418,TLError,"El CDE se desconecto");
				close(iSocketCde);
				FD_CLR(iSocketCde,&maefds);
				iCansancio=fiEquCalculoCansancio(iEstadoCde,iCansancio,tv,tTiempoDeUso);
				iEstadoCde = 2;

				if((fiEquRtaCiudadLibre(iSocketCiudad,cpMiIp,iMiPuerto))==-1)
				{
					fvStdLog("equipo.c",426,TLError,"Error al avisar a la Ciudad que comenzo un Partido");
					exit(1);
				}
			}
			else /*El CDE me avisa que ya hay lugar*/
			{
				iEstadoCde = 1;
				tv.tv_sec = tTiempoDeUso ; /*Es el tiempo que entrenare*/
				tv.tv_usec = 0 ;
			}

		}
		else if((iJugando)&&(FD_ISSET(iSocketEquipo,&readfds)))/*Es el Equipo con el que estoy jugando que quiere decirme que termino el partido*/
		{
			if((iMjeEqu= recv(iSocketEquipo,buf,1,0))==-1)
			{
				fvStdLog("equipo.c",442,TLError,"Error al recibir fin de Partido");
				exit(1);
			}
			else if (iMjeEqu == 0)
			{
				iJugando = 0;
				iCansancio = (iCansancio + fiEquCansancio());
				FD_CLR(iSocketEquipo, &maefds);
				close(iSocketEquipo);
				if((fiEquRtaCiudadLibre(iSocketCiudad,cpMiIp,iMiPuerto))==-1)
				{
					fvStdLog("equipo.c",453,TLError,"Error al avisar a la Ciudad que comenzo un Partido");
					exit(1);
				}
				fvStdLog("equipo.c",456,TLMensaje,"Finaliza Partido de local");
			}
			else
			{
				fvStdLog("equipo.c",460,TLMensaje,"EL Equipo me mando cualquier cosa loco!");

			}
		}
		/*FIN DE LOS FD_ISSET*/
		else if((iEsperando == 0) && 
				(((iJugando == 0)&&((tv.tv_sec == 0)&&(tv.tv_usec == 0))&&(iEstadoCde== 2)) ||
				 ((iCansancio > iMaxCansancio)&&(iEstadoCde == 2)&&(iJugando ==0))))/*Ninguno de los Sockets estuvo Seteado*/
		{ 
			if(fiEquPedidoDeCde(iSocketCiudad)==-1) /*Pido el CDE a la Ciudad que me devuelve Ip y Puerto*/
			{
				fvStdLog("equipo.c",471,TLError,"Error al Pedir el CDE a IP:%s Puerto:%d",cpIpCde,iPuertoCde);	
				exit(1);
			}
			iEsperando = 1;
		}
		else if((iEstadoCde == 1)&&(((tv.tv_sec == 0)&&(tv.tv_usec == 0))))/*Termine de Entrenar*/
		{
			iCansancio = 0;
			iEstadoCde = 2;
			close(iSocketCde);
			FD_CLR(iSocketCde,&maefds);
			fvStdLog("equipo.c",482,TLMensaje,"Fin de Entrenamiento");
			if(fiEquRtaCiudadLibre(iSocketCiudad,cpMiIp,iMiPuerto)==-1)	
			{
				fvStdLog("equipo.c",485,TLError,"Error al Responder a la Ciudad q estoy libre");
				exit(1);
			}
			if(iPendiente == 1)
			{
				if(fiEquJugarLevI(lLevInt,tDurPartido,&iCansancio,iMaxCansancio,iSocketCiudad,tTiempoMaxCansado, cpMiNombre)==-1)	
					exit(1);
				iPendiente = 0;
			}
			iEsperando = 0;
		}
	}
	close(iSocketCiudad);
	close(iSocketEscucha);
	free(cpIpCiudad);
	free(cpMiIp);
	return(0);
}

