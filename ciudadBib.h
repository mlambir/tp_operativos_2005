/*
 * =====================================================================================
 *       Filename:  ciudadBib.h
 * =====================================================================================
 */

typedef struct sEquipoCola
{
	int socket;
	struct sEquipoCola * siguiente;
}sEquipoCola;				/* ----------  end of struct sEquipoCola  ---------- */


typedef struct sTDP
{
	char *ciudad;
	char *nombre;
	int puntos;
	int pg;
	int pe;
	int pp;
	struct sTDP *siguiente;		
}sTDP;

typedef struct sEquipo
{
	char * nombre;
	int ipPuerto[6];
	int estado;
	int iTieneLev;
	int socket;
	struct sEquipoCola *cola;
	struct sEquipo *siguiente;
}sEquipo;				/* ----------  end of struct sEquipo  ---------- */

typedef struct sMigracion
{
	char * cpDatos;
	int iTamanioDatos;
	int iSocket;
	struct sMigracion * siguiente;
}sMigracion;

typedef struct sDiscover
{
	int iSocket;
	char * nombre;
	char * nombreCiudad;
	struct sDiscover * siguiente;	
}sDiscover;

enum eEstado
{
	ESTADOvacio = 0,
	ESTADOocupado,
	ESTADOlibre,
	ESTADOmigrado,
	ESTADOdesconectado
};				/* ----------  end of enum eEstado  ---------- */

enum eMensajesRouter
{
	MSDesconectadoR = 0,
	MSTDP,
	MSFound,
	MSNotFound,
	MSEPregunta
};				/* ----------  end of enum eMensajes  ---------- */

enum eMensajes
{
	MSDesconectado = 0,
	MSLEVI,
	MSLEVE,
	MSDatosEquipo,
	MSPedidoPartido,
	MSLibre,
	MSPedidoEntrenamiento,
	MSOcupado,
	MSMigracion,
	MSPedidoDtMigracion
};				/* ----------  end of enum eMensajes  ---------- */

int fiCiuAgregarEquipoInicial(struct sEquipo ** , int );
struct sEquipo * fiCiuBuscarEquipoNombre(struct sEquipo * , char * );
struct sEquipo * fiCiuBuscarEquipoSocket(struct sEquipo * , int );
int fiCiuRecibirCabeceraEquipo(int );
int fiCiuProcesarLEV(struct sTDP * , int , char * ,char *);
void fvCiuAnotarPartidos(struct sTDP *, char *,char *,char * , char * , unsigned char );
void fvCiuRecibirDatosEquipo(int , struct sEquipo **,fd_set * );
int fiCiuManejarPedidoPermiso (int,struct sEquipo *, char, int, struct sDiscover **, char *);
void fvCiuBorrarEquipo(struct sEquipo ** );
int fiCiuManejarMensajeLibre(int , struct sEquipo * );
int fiCiuRecibirCabeceraRouter(int );
void fiCiuSacarEquipoLista(int , struct sEquipo **);
int fiCiuEnviarDT(int ,char * );
int fiCIURecibirTDPRouter(int , struct sTDP ** , struct sEquipo * , char * );
int fiCiuEnviarLEVInternaEquipo(int , char * , char * , struct sTDP* , struct sEquipo * );
int fiCiuEnviarDT(int ,char * );
int fiCiuEstaEnTDP (struct sTDP *, char *);
int fiCiuEnviarTDPRouter(int ,struct sTDP** , struct sEquipo * ,char * );
int fiCiuManejarPedidoEntrenamiento(int , char * );
void handlerSigchild(int);
void fvCIUAgregarFIFOMigracion (struct sMigracion **, int,  int , char * );
void fvCIUAgregarFIFO (struct sEquipoCola **, int );
int fiCiuManejarMensajeFound(int , struct sDiscover ** , char );
int fiCiuAgregarDatosEquipo (struct sEquipo **, char *, int *, int ,fd_set *);
int fiCiuManejarPregunta(int ,struct sEquipo *,char *,int);
int fiCiuLimpiarListaEquipos(struct sEquipo ** );
int recvAll(int , char * , int , int);
