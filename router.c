#include        <stdio.h>
#include	<stddef.h>
#include	<time.h>
#include	<ctype.h>
#include  	<string.h>
#include  	<unistd.h>
#include        <stdlib.h>
#include        <sys/types.h>
#include        <sys/socket.h>
#include        <netinet/in.h>
#include        <arpa/inet.h>
#include	"configuracion.h"
#include	"routerBib.h"

#define STDIN 0

int main(int argc, char * argv[])
{
		int iSockRouter;
		int iSockCiudad;
		int iSockAcceptR;
		int iSockAcceptC;
		int iSockAcceptT;
		int iSockRecibirT;
		socklen_t iAddrLen;
		int iDatos=0;
		char * cpTDP;
		struct sCiudad * sListaC = NULL;
		struct sCiudad * sDatosCiudad=NULL;
		struct sTDP * sListaTDP = NULL;
		struct sRouter * sListaR = NULL;
		struct sDiscover * sListaD = NULL;
		struct sockaddr_in sMiDireccionAcceptR;
		struct sockaddr_in sMiDireccionAcceptC;
		struct sockaddr_in sMiDireccionAcceptT;
		struct sockaddr_in sDireccionRouter;
		struct sockaddr_in sDireccionExterna;
		char cpIpPuertoTok[6];
		char cpID[5];
		char * cpStdin;
		char cTTL;
		char cHops;
		int fdmax;
		int iMensaje;
		int si = 1;
		int iTengoToken=0;
		int iTengoCiudad=0;
		//int iRouter=0;
		int i;
		char cpIdMensaje[2];
		int iLargoMensaje;
		int iTiempoBorrar;
		char * cpNombreCiu = "";

		char * cpPuertoRouter;
		char * cpIpRouter;
		char * cpPuertoCiudad;
		char * cpIpCiudad;
		char * cpPuertoToken;
		int iPuertoToken;
		char * cpIpToken;
		char * cpTTLEnviar;
		int iTTLEnviar;
		char * cpTiempoBorrar;
		struct timeval tv;

		fd_set maestro;
		fd_set readfds;

		fvStdTomarConfiguracion("./CONFIGR","PUERTOR", &cpPuertoRouter);
		fvStdTomarConfiguracion("./CONFIGR","IPR", &cpIpRouter);
		fvStdTomarConfiguracion("./CONFIGR","PUERTOC", &cpPuertoCiudad);
		fvStdTomarConfiguracion("./CONFIGR","IPC", &cpIpCiudad);
		fvStdTomarConfiguracion("./CONFIGR","PUERTOT", &cpPuertoToken);
		iPuertoToken = atoi(cpPuertoToken);
		fvStdTomarConfiguracion("./CONFIGR","IPT", &cpIpToken);
		fvStdTomarConfiguracion("./CONFIGR","TTL", &cpTTLEnviar);
		iTTLEnviar = atoi(cpTTLEnviar);
		fvStdTomarConfiguracion("./CONFIGR","TIEMPOBORRAR", &cpTiempoBorrar);
		iTiempoBorrar = atoi(cpTiempoBorrar);

	  fiRouScanfIP(&(cpIpPuertoTok[0]), cpIpToken, iPuertoToken);

		FD_ZERO(&maestro);
		FD_ZERO(&readfds);
		
		tv.tv_sec = iTiempoBorrar;
		tv.tv_usec = 0;

		iSockRecibirT = 0;

		FD_SET(STDIN, &maestro);

		cpStdin = (char *) malloc(64);

		iAddrLen =sizeof(sDireccionExterna);

		if((iSockAcceptR = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
				perror("socket");
				exit(1);
		}
		if(setsockopt(iSockAcceptR, SOL_SOCKET, SO_REUSEADDR, &si, sizeof(int)) == -1)
		{
				perror("setsockopt");
				exit(1);
		}

		sMiDireccionAcceptR.sin_family = AF_INET;
		sMiDireccionAcceptR.sin_port = htons(atoi(cpPuertoRouter));                  
		inet_aton(cpIpRouter, &(sMiDireccionAcceptR.sin_addr));      
		memset(&(sMiDireccionAcceptR.sin_zero), '\0', 8);

		if (bind(iSockAcceptR, (struct sockaddr *) &sMiDireccionAcceptR, sizeof(sMiDireccionAcceptR))== -1)
		{
				perror("bind");
				exit(1);
		}
		if(listen(iSockAcceptR, 10) == -1)
		{
				perror("listen");
				exit(1);
		}
		FD_SET(iSockAcceptR,&maestro);
		fdmax = iSockAcceptR;

		if(argc==3)
		{
				if((iSockRouter = socket(AF_INET, SOCK_STREAM, 0)) == -1)
				{
						perror("socket");
						exit(1);
				}
				sDireccionRouter.sin_family = AF_INET;
				sDireccionRouter.sin_port = htons(atoi(argv[2]));   
				inet_aton(argv[1], &(sDireccionRouter.sin_addr));   
				memset(&(sDireccionRouter).sin_zero, '\0', 8);

				if (connect(iSockRouter, (struct sockaddr *) &sDireccionRouter, sizeof(struct sockaddr)))
				{
						perror("connect");
						exit(1);
				}
				FD_SET(iSockRouter,&maestro);
				fdmax=iSockRouter;
				
				fiRouAgregarRouter(&sListaR,iSockRouter);
				fvEnviarDatos(iSockRouter, cpIpPuertoTok);

		}

		if((iSockAcceptC = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
				perror("socket");
				exit(1);
		}
		if(setsockopt(iSockAcceptC, SOL_SOCKET, SO_REUSEADDR, &si, sizeof(int)) == -1)
		{
				perror("setsockopt");
				exit(1);
		}

		sMiDireccionAcceptC.sin_family = AF_INET;
		sMiDireccionAcceptC.sin_port = htons(atoi(cpPuertoCiudad));
		inet_aton(cpIpCiudad, &(sMiDireccionAcceptC.sin_addr));
		memset(&(sMiDireccionAcceptC.sin_zero), '\0', 8);

		if (bind(iSockAcceptC, (struct sockaddr *) &sMiDireccionAcceptC, sizeof(sMiDireccionAcceptC))== -1)
		{
				perror("bind");
				exit(1);
		}
		if(listen(iSockAcceptC, 10) == -1)
		{
				perror("listen");
				exit(1);
		}
		FD_SET(iSockAcceptC, &maestro);
		if(fdmax < iSockAcceptC)
				fdmax=iSockAcceptC;	

		if((iSockAcceptT = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
				perror("socket");
				exit(1);
		}
		if(setsockopt(iSockAcceptT, SOL_SOCKET, SO_REUSEADDR, &si, sizeof(int)) == -1)
		{
				perror("setsockopt");
				exit(1);
		}

		sMiDireccionAcceptT.sin_family = AF_INET;
		sMiDireccionAcceptT.sin_port = htons(iPuertoToken);
		inet_aton(cpIpToken, &(sMiDireccionAcceptT.sin_addr));
		memset(&(sMiDireccionAcceptT.sin_zero), '\0', 8);

		if (bind(iSockAcceptT, (struct sockaddr *) &sMiDireccionAcceptT, sizeof(sMiDireccionAcceptT))== -1)
		{
				perror("bind");
				exit(1);
		}
		if(listen(iSockAcceptT, 10) == -1)
		{
				perror("listen");
				exit(1);
		}
		FD_SET(iSockAcceptT, &maestro);
		if(fdmax < iSockAcceptT)
				fdmax=iSockAcceptT;	


		for(;;)
		{
				readfds = maestro;

				if((select(fdmax+1, &readfds,NULL,NULL,&tv))==-1) 
				{
						perror("select");
						exit(1);
				}
				if(tv.tv_sec == 0)
				{
					fiRouLimpiarListaTiempo(iSockCiudad,&sListaD, iTiempoBorrar);
					tv.tv_sec = iTiempoBorrar;
					tv.tv_usec = 0 ;
				}
				else
				{
					for(i=0;i<=fdmax;i++)
					{
						if(FD_ISSET(i,&readfds))
						{
							if(i == STDIN)
							{
								fgets(cpStdin, 64, stdin);	
								if(!strncmp(cpStdin, "start", 5))
									if(sListaR && iTengoCiudad)
									{
										fvRouEnviarTokenInicial(sListaR, cpNombreCiu, cpIpPuertoTok, &sListaC, iSockCiudad, cpID);
										iTengoToken = 0;
									}
									else
									{
										printf("Conectar Router y Ciudad antes de comenzar la simulacion ;)");
									}
							}
							else if(i==iSockAcceptR)
							{
								if((iSockRouter = accept(iSockAcceptR, (struct sockaddr *)&sDireccionExterna, &iAddrLen)) == -1)
								{
									perror("accept");
									exit(1);
								}
								else
								{
									FD_SET(iSockRouter, &maestro);
									if (iSockRouter > fdmax)
										fdmax = iSockRouter;
									fiRouAgregarRouter(&sListaR, iSockRouter);
								}
							}
							else if(i==iSockAcceptC)
							{
								if((iSockCiudad = accept(iSockAcceptC, (struct sockaddr *)&sDireccionExterna, &iAddrLen)) == -1)
								{
									perror("accept");
									exit(1);
								}
								else
								{
									FD_SET(iSockCiudad, &maestro);
									if (iSockCiudad > fdmax)
										fdmax = iSockCiudad;
									//FD_CLR(iSockAcceptC,&maestro);
									//close(iSockAcceptC);
									iTengoCiudad = 1;
								}
							}
							else if((iTengoCiudad==1)&&(i==iSockCiudad))
							{
								iMensaje = fiRouRecibirCabeceraCiudad(i);
								switch(iMensaje)
								{
									case MSDT:
										fiRouRecibirDatosCiudad(i, &cpNombreCiu);
										break;

									case MSTDP:
										fiRouRecibirTDP(i,&sListaTDP);
										if(!fiRouEnviarTokenProximoRouter(&sListaR,cpID,&sListaTDP,&sListaC, cpIpPuertoTok))
											fiRouEnviarTDP(iSockCiudad, &sListaTDP);
										else
											iTengoToken = 0;
										break;

									case MSPedido:
										fiRouRecibirPedido(i, &sListaD, sListaR, iTTLEnviar); 
										break;

									case MSEncontrado:
										fiRouManejarEncontrado(i,&sListaD,cpNombreCiu);
										break;

									case MSNoEncontrado:
										fiRouManejarNoEncontrado(i,&sListaD, cpNombreCiu);
										break;

									case MSDesconectadoC:
										exit(0);
										break;
									default:
										printf("ERROR Mensaje no Conocido");
										break;
								}
							}
							else if(i == iSockAcceptT)
							{
								if((iSockRecibirT = accept(iSockAcceptT, (struct sockaddr *)&sDireccionExterna, &iAddrLen)) == -1)
								{
									perror("accept");
									exit(1);
								}
								else
								{
									FD_SET(iSockRecibirT, &maestro);
									if (iSockRecibirT > fdmax)
										fdmax = iSockRecibirT;
								}
							}
							else if(iSockRecibirT != 0 && i == iSockRecibirT)
							{
								fiRouRecibirToken(i, cpID, &sListaTDP, &sListaC, sListaR, cpIpPuertoTok, cpNombreCiu);
								if(iTengoCiudad)
								{
									fiRouEnviarTDP(iSockCiudad, &sListaTDP);
									iTengoToken=1;
								}
								else
								{
									fiRouEnviarTokenProximoRouter(&sListaR,cpID,&sListaTDP,&sListaC, cpIpPuertoTok);
									iTengoToken=0;
								}
								close(iSockRecibirT);
								FD_CLR(iSockRecibirT,&maestro);
								iSockRecibirT = 0;
								sleep(1);
							}
							else
							{
								iMensaje = fiRouRecibirCabeceraRouter(i, cpIdMensaje, &cTTL, &cHops, &iLargoMensaje);
								switch(iMensaje)
								{
									case MSDiscover:
										fiRouManejarDiscover(i, &sListaD, sListaR, cpNombreCiu, iSockCiudad,cpIdMensaje,cTTL, cHops, iLargoMensaje);
										break;
									case MSFound:
										fvRouManejarFound(i, &sListaD, cpNombreCiu, iSockCiudad, cpIdMensaje, cTTL, cHops, iLargoMensaje);
										break;
									case MSNotFound:
										fvRouManejarNotFound(i, &sListaD, cpNombreCiu, iSockCiudad, cpIdMensaje, cTTL, cHops, iLargoMensaje);
										break;
									case MSDatos:
										fvRouManejarDatos(i, sListaR);
										if(iTengoToken)
										{
											fiRouCompletarToken(&sListaC, sListaR);
										}
										break;
									case MSDesconectadoR:
										fiRouSacarRouterLista(&sListaR, i);
										close(i);
										FD_CLR(i,&maestro);
										break;

								}
							}
						}
					}
				}
		}
}

