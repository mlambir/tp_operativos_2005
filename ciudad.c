/*
 * =====================================================================================
 *        Filename:  ciudad.c
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
#include	<getopt.h>
#include  <signal.h>
#include  <errno.h>
#include	<ncurses.h>
#include	"ciudadBib.h"
#include	"configuracion.h"
#include	"logs.h"
#include	"ciudadCurs.h"

int main ( int argc, char *argv[])
{
	fd_set maestro;
	fd_set readfds;
	struct sockaddr_in sMiDireccion;
	struct sockaddr_in sDireccionExterna;
	struct sockaddr_in sDireccionRouter;
	struct sTDP * sTablaDePosiciones;
	struct sEquipo * sListaEquipos;
	struct sEquipo * sEquipoAux;
	struct sMigracion * sListaMig;
	struct sDiscover * sListaDiscover;
	int iMaxFD;
	int iSockAccept;
	int iSockNuevo;
	int iSockRouter;
	int si = 1;
	int i;
	int iMensaje;
	int iEncontro;
	int iSalir;
	int iSegundosBorrar;
	int iAuxiliar;
	char * cpSegundosBorrar;
	char cpMensajePCDE[8];
	char cTipoLEV='N';
	char *nombreCiudad;
	char *cpMiIp;
	char *cpMiPuerto;
	int  iMiPuerto;
	char *cpIpCde;
	char *cpPuertoCde;
	char *cpIpRouter;
	char *cpPuertoRouter;
	char *cpNombreExecEquipo;
	socklen_t iAddrLen;
	struct sigaction accion;
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	sTablaDePosiciones=NULL;
	sListaEquipos=NULL;
	sListaMig=NULL;
	sListaDiscover=NULL;

	fvStdTomarConfiguracion("./CONFIGC","IP", &cpMiIp);
	fvStdTomarConfiguracion("./CONFIGC","PUERTO", &cpMiPuerto);
	fvStdTomarConfiguracion("./CONFIGC","IPCDE", &cpIpCde);
	fvStdTomarConfiguracion("./CONFIGC","PUERTOCDE", &cpPuertoCde);
	fvStdTomarConfiguracion("./CONFIGC","IPROUTER", &cpIpRouter);
	fvStdTomarConfiguracion("./CONFIGC","PUERTOROUTER", &cpPuertoRouter);
	fvStdTomarConfiguracion("./CONFIGC","NOMBRE", &nombreCiudad);
	fvStdTomarConfiguracion("./CONFIGC","EXECEQUIPO", &cpNombreExecEquipo);
	fvStdTomarConfiguracion("./CONFIGC","TIEMPOLIMPIEZA", &cpSegundosBorrar);
	
	init_ciudad();
	
	iSegundosBorrar = atoi(cpSegundosBorrar);
	iMiPuerto=atoi(cpMiPuerto);

	accion.sa_handler = handlerSigchild;
	sigemptyset(&accion.sa_mask);
	accion.sa_flags = SA_NOCLDSTOP;

	if((sigaction(SIGCHLD, &accion, NULL))<0)
	{
		perror("sigaction");
		exit(0);
	}

	strcpy(argv[0], nombreCiudad);

	fvStdIniciarLogs(nombreCiudad, "logs.log");

	FD_ZERO(&maestro);
	FD_ZERO(&readfds);
	/*
	 *		SOCKET ACCEPT
	 */
	if((iSockAccept = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}
	if(setsockopt(iSockAccept, SOL_SOCKET, SO_REUSEADDR, &si, sizeof(int)) == -1)
	{
		perror("setsockopt");
		exit(1);
	}

	strcpy(cpMensajePCDE, "RE");

	fiRouScanfIP(&(cpMensajePCDE[2]), cpIpCde, atoi(cpPuertoCde));

	//		sscanf(cpIpCde, "%u.%u.%u.%u", (unsigned int *)&(cpMensajePCDE[2]), (unsigned int *)&(cpMensajePCDE[3]), (unsigned int *)&(cpMensajePCDE[4]), (unsigned int *)&(cpMensajePCDE[5]));
	//		cpMensajePCDE[6] = atoi(cpPuertoCde) % 256;
	//		cpMensajePCDE[7] = atoi(cpPuertoCde) / 256;

	sMiDireccion.sin_family = AF_INET;
	sMiDireccion.sin_port = htons(atoi(cpMiPuerto));
	inet_aton(cpMiIp, &(sMiDireccion.sin_addr));
	memset(&(sMiDireccion.sin_zero), '\0', 8);
	if (bind(iSockAccept, (struct sockaddr *) &sMiDireccion, sizeof(sMiDireccion))== -1)
	{
		perror("bind");
		exit(1);
	}
	if(listen(iSockAccept, 10) == -1)
	{
		perror("listen");
		exit(1);
	}
	FD_SET(iSockAccept, &maestro);

	/*
	 *		SOCKET CONNECT 
	 */

	if((iSockRouter = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}
	sDireccionRouter.sin_family = AF_INET;
	sDireccionRouter.sin_port = htons(atoi(cpPuertoRouter));			
	inet_aton(cpIpRouter, &(sDireccionRouter.sin_addr));		
	memset(&(sDireccionRouter).sin_zero, '\0', 8);
	if (connect(iSockRouter, (struct sockaddr *) &sDireccionRouter, sizeof(struct sockaddr)))
	{
		perror("connect");
		exit(1);
	}

	fiCiuEnviarDT(iSockRouter,nombreCiudad);

	FD_SET(iSockRouter, &maestro);

	iMaxFD = (iSockAccept > iSockRouter)?iSockAccept:iSockRouter;

	while (1)
	{
		readfds = maestro;
		imprimirEquipos(sListaEquipos);
		imprimirTDP(sTablaDePosiciones);
		if(!(tv.tv_sec || tv.tv_usec))
			tv.tv_sec = iSegundosBorrar;
		if (select(iMaxFD + 1, &readfds, NULL, NULL, &tv) == -1)
		{
			if (errno != EINTR)
			{
				perror("select");
				exit(1);
			}
		}
		else
		{
			for ( i = 0; i <= iMaxFD; i++ )
			{
				if(FD_ISSET(i, &readfds))
				{
					if(i == iSockAccept)		//conexion entrante
					{
						iAddrLen =sizeof(sDireccionExterna);
						/*
						 *cambiar por lista
						 */
						if((iSockNuevo = accept(iSockAccept, (struct sockaddr *) &sDireccionExterna, &iAddrLen)) == -1)
						{
							perror("accept");
						}	
						else
						{

							fiCiuAgregarEquipoInicial(&sListaEquipos, iSockNuevo);

							agregarTexto("Aceptada Conexion");
							FD_SET(iSockNuevo, &maestro);
							if (iSockNuevo > iMaxFD)
							{
								iMaxFD = iSockNuevo;
							}
						}
					}
					else if (i == iSockRouter)	//mensaje nuevo ROUTER
					{
						iMensaje = fiCiuRecibirCabeceraRouter(i);
						switch(iMensaje)
						{
							case MSTDP:
								iEncontro = fiCIURecibirTDPRouter(i, &sTablaDePosiciones, sListaEquipos, nombreCiudad);
								agregarTexto("Recibida TDP");
								if(iEncontro)
								{
									sEquipoAux = sListaEquipos;
									iSalir = 0;
									while (!iSalir && sEquipoAux != NULL)
									{ 
										iSalir = fiCiuEnviarLEVInternaEquipo(sEquipoAux->socket, sEquipoAux->nombre, nombreCiudad, sTablaDePosiciones, sListaEquipos);
										if(!iSalir)
										{
											sEquipoAux = sEquipoAux->siguiente;
										}
										else
										{
											sEquipoAux->iTieneLev = 1;
											cTipoLEV = 'I';
											agregarTexto("Jugando LEV Interna");
										}
									}
									if(sEquipoAux == NULL)
									{
										if(!fiCiuEnviarLevExternaTodos(nombreCiudad, sTablaDePosiciones, sListaEquipos))
										{
											fiCiuEnviarTDPRouter(iSockRouter,&sTablaDePosiciones, sListaEquipos, nombreCiudad);
											agregarTexto("Enviando LEV al Router");
											cTipoLEV = 'N';
										}
										else
										{
											cTipoLEV = 'E';
											tv.tv_sec = iSegundosBorrar;
											agregarTexto("Jugando LEV Externa");
										}
									}
								}
								else
								{
									fiCiuEnviarTDPRouter(iSockRouter,&sTablaDePosiciones, sListaEquipos, nombreCiudad);
									cTipoLEV = 'N';
											agregarTexto("Enviando LEV al Router");
								}
								break;

							case MSFound:
								agregarTexto("Recibido FOUND");
								fiCiuManejarMensajeFound(i, &sListaDiscover,'F');
								break;

							case MSNotFound:
								agregarTexto("Recibido NOT FOUND");
								fiCiuManejarMensajeFound(i, &sListaDiscover,'N');
								break;
							case MSEPregunta:
								fiCiuManejarPregunta(iSockRouter,sListaEquipos,cpMiIp,iMiPuerto);
								break;
							case MSDesconectadoR:
								exit(0);
								break;
						}
					}
					else		//mensaje nuevo EQUIPO
					{
						iMensaje = fiCiuRecibirCabeceraEquipo(i);

						switch ( iMensaje )
						{
							case MSLEVI:	
								sEquipoAux =  fiCiuBuscarEquipoSocket(sListaEquipos , i);
								sEquipoAux->iTieneLev = 0;
								fiCiuProcesarLEV(sTablaDePosiciones, i, sEquipoAux->nombre,nombreCiudad);
								sEquipoAux = sEquipoAux->siguiente;
								iSalir = 0;
								while (!iSalir && sEquipoAux != NULL)
								{ 
									iSalir = fiCiuEnviarLEVInternaEquipo(sEquipoAux->socket, sEquipoAux->nombre, nombreCiudad, sTablaDePosiciones, sListaEquipos);
									if(!iSalir)
									{
										sEquipoAux = sEquipoAux->siguiente;
									}
									else
									{
										sEquipoAux->iTieneLev = 1;
									}
								}
								if(sEquipoAux == NULL)
								{
									if(!fiCiuEnviarLevExternaTodos(nombreCiudad, sTablaDePosiciones, sListaEquipos))
									{
										fiCiuEnviarTDPRouter(iSockRouter,&sTablaDePosiciones, sListaEquipos, nombreCiudad);
											agregarTexto("Enviando LEV al Router");
										cTipoLEV = 'N';
									}
									else
									{
											agregarTexto("Jugando LEV Externa");
										cTipoLEV = 'E';
										tv.tv_sec = iSegundosBorrar;
									}
								}
								break;

							case MSLEVE:
								sEquipoAux = fiCiuBuscarEquipoSocket(sListaEquipos , i);
								fiCiuProcesarLEV(sTablaDePosiciones, i, sEquipoAux->nombre,nombreCiudad);
								if(sEquipoAux->iTieneLev==1)
								{
									sEquipoAux->iTieneLev = 0;
									if(fiCiuCompleta(sListaEquipos))
									{
										agregarTexto("Enviando LEV al Router");
										fiCiuEnviarTDPRouter(iSockRouter,&sTablaDePosiciones, sListaEquipos, nombreCiudad);
										cTipoLEV = 'N';
									}
								}
								break;

							case MSPedidoPartido:	
								fiCiuManejarPedidoPermiso (i,sListaEquipos, cTipoLEV, iSockRouter,&sListaDiscover, nombreCiudad);
								break;

							case MSDatosEquipo:	
								fvCiuRecibirDatosEquipo(i, &sListaEquipos,&maestro);
								break;

							case MSLibre:	
								fiCiuManejarMensajeLibre(i, sListaEquipos);
								break;

							case MSDesconectado:	
								sEquipoAux =  fiCiuBuscarEquipoSocket(sListaEquipos , i);
								if(cTipoLEV == 'I' ||cTipoLEV == 'N'|| (sEquipoAux->estado == ESTADOvacio))
								{
									if (sEquipoAux->iTieneLev)
									{
										fiCiuProcesarLEV(sTablaDePosiciones, i, sEquipoAux->nombre,nombreCiudad);
										sEquipoAux = sEquipoAux->siguiente;
										iSalir = 0;
										while (!iSalir && sEquipoAux != NULL)
										{ 
											iSalir = fiCiuEnviarLEVInternaEquipo(sEquipoAux->socket, sEquipoAux->nombre, nombreCiudad, sTablaDePosiciones, sListaEquipos);
											if(!iSalir)
											{
												sEquipoAux = sEquipoAux->siguiente;
											}
											else
											{
												sEquipoAux->iTieneLev = 1;
											}
										}
										fiCiuSacarEquipoLista(i, &sListaEquipos);
										fiCiuSacarEquipoColas(i, sListaEquipos);
										if(sEquipoAux == NULL)
										{
											if(!fiCiuEnviarLevExternaTodos(nombreCiudad, sTablaDePosiciones, sListaEquipos))
											{
												fiCiuEnviarTDPRouter(iSockRouter,&sTablaDePosiciones, sListaEquipos, nombreCiudad);
												agregarTexto("Enviando LEV al Router");
												cTipoLEV = 'N';
											}
											else
											{
												agregarTexto("Jugando LEV Externa");
												cTipoLEV = 'E';
												tv.tv_sec = iSegundosBorrar;
											}
										}
									}
									else
									{
									
										fiCiuSacarEquipoLista(i, &sListaEquipos);
										fiCiuSacarEquipoColas(i, sListaEquipos);
									}
								}
								else
									sEquipoAux->socket = 0;

								FD_CLR(i, &maestro);
								close(i);
								break;

							case MSPedidoEntrenamiento:	
								fiCiuManejarPedidoEntrenamiento(i, cpMensajePCDE);
								break;

							case MSMigracion:
								fiCiuManejarMigracion(i, &sListaMig, cpMiIp, cpMiPuerto, cpNombreExecEquipo);
								break;

							case MSPedidoDtMigracion:
								fvCIUEnviarDatosMig(i,&sListaMig);
								break;

						}	/* -----  end switch  ----- */
					}  
				}
			}
			if((tv.tv_sec == 0 && tv.tv_usec == 0) && (cTipoLEV == 'E'))
			{
				agregarTexto("Limpiando Lista de Equipos Locales");
				fiCiuLimpiarListaEquipos(&sListaEquipos);
				sEquipoAux = sListaEquipos;
				iAuxiliar = 0;
				while(sEquipoAux)
				{
					if(sEquipoAux->iTieneLev)
					{
						iAuxiliar = 1;
					}
					sEquipoAux = sEquipoAux->siguiente;
				}
				if(!iAuxiliar)
				{
					agregarTexto("Enviando LEV al Router");
					fiCiuEnviarTDPRouter(iSockRouter,&sTablaDePosiciones, sListaEquipos, nombreCiudad);
					cTipoLEV = 'N';
				}
			}
		}
	}
}

