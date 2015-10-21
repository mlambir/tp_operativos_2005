#include <netinet/in.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define STDIN 0

struct mjeTiempo 
{
	char cabecera [3];
	int tiempo;
};

struct mjeIpPuerto
{
	char cabecera [3];
	unsigned char ip[4];
	int puerto;
};

typedef struct lLevInt
{
	char * cpNombreCiudad;
	char * cpNombreEquipo;
	int iResultado;
	struct lLevInt *siguiente;
} lLevInt;


struct mjeIpPuertoSC
{
	unsigned char ip[4];
	int puerto;
};

struct DatosMigrar
{
	char cpMiIp[16];
	int iMiPuerto;
	char cpIpCiudad[16];
	int iPuertoCiudad;
	int iMaxTiempo;
	int iDurPartido;
	int iTiempoMaxCansado;
	int iCansancio;
	int iMaxCansancio;
};


int fiEquConectar(char*, int);

char* fcEquMayusculas(char []);

int fiEquReciboCabecera(int);

int fiEquRecibirLevI(int ,struct lLevInt **);

int fiEquAgregarALista(struct lLevInt * , struct lLevInt ** );

int fiEquJugarLevI(struct lLevInt *, int ,int * ,int , int,int , char*);

int fiEquEnviarResultados(int ,struct lLevInt ** ,int ,char * );

int fiEquPermisoJugar(int ,char * ,char *,char * ,int *);

int fiEquReciboRespuesta(int ,char * , int *);

void fvEquConcatenar(char* , char* , int , int );

int fiEquCalcularResultado(void);

int fiEquCansancio(void);

int fiEquDescansarTotalmente(int ,int *, int,int, char*);

int contar(char * , char , int );

void reemplazar(char*, char , char );

int sendall(int , char *, int);

char * itoa(int , char*);

int fiEquEscucharAEquipos(struct sockaddr *);

int fiEquRtaCiudadOcupado(int);

int fiEquRtaCiudadLibre(int,char *,int);

int fiEquAceptoEquipo(int);

int fiEquReciboMensajeCde(int);

int fiEquReciboRespuestaCde(int);

int fiEquHandShake(int);

int fiEquReciboRtaHS(int);

int fiEquPuedoJugar(int , int * ,int, struct timeval , int,int);

int fiEquCalculoCansancio(int, int ,struct timeval , int );

int fiEquAsignarTiempo(int , int , int );

int fiEquEnviarDatos(int ,char *,int ,char *);

int fiEquAvisarTiempoCde( int , int , char*);
	
int fiEquPedidoDeCde(int);

int fiEquPedidoDeCdeLI(int ,char *,int *);
	
int fiEquRtaEntrenamiento(int ,char * , int *);

int fiEquRtaEntrenamientoLI(int ,char * , int *);

int fiEquEnviarMigracion(int ,struct lLevInt * ,struct DatosMigrar ,char * );

struct DatosMigrar fiEquManejarMigracion(int , char *, int ,char **, struct lLevInt **,int*);
		
void fiEquSacarEquipoLev(struct lLevInt ** ,char * , char * );

int fiEquJugarLevE(int ,struct lLevInt ** ,int ,int * ,int ,int ,int ,char * ,int ,char *,int ,char *,char *,int );
 
int fiRouPrintfIP(char *,char * , int *);

int recvAll(int , char * , int , int );
