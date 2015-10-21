
enum MSCiu
{
	MSDT = 0,
	MSTDP,
	MSDesconectadoC,
	MSPedido,
	MSEncontrado,
	MSNoEncontrado
};

enum MSRu
{
	MSDiscover=0, 
	MSFound=1, 
	MSNotFound=2,
	MSDatos=3,
	MSDesconectadoR
};

typedef struct sCiudad
{
	char orden;
	char *nombre;
	char ipPuerto[6];	
	struct sCiudad *siguiente;
}sCiudad;

typedef struct sDiscover
{
	char iIdentificador[2];
	int iSocket;
	char * cpNombreCiudad;
	char * cpNombreEquipo;
	time_t tiempo;
	char ttl;
	char hops;
	struct sDiscover * siguiente;
}sDiscover;

typedef struct sRouter
{
	int iTengoDatos;
	char cpIpPuerto[6];
	int iSocket;
	struct sRouter * siguiente;
}sRouter;

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
int fiRouRecibirCabeceraCiudad(int );

int fiRouRecibirCabeceraRouter(int , char * , char * , char * , int * );

void  fiRouRecibirDatosCiudad(int , char ** );

int fiRouRecibirTDP(int , struct sTDP** );

int fiRouRecibirDiscoverC(int ,char * );

struct sDiscover * fiRouBuscarDiscoverID(struct sDiscover *, char * );

int fiRouAgregarRouter(struct sRouter ** , int );
			
struct sRouter * fiRouBuscarRouterSocket(struct sRouter * , int );

void fvRouEnviarTokenInicial(struct sRouter * ,char * , char * , struct sCiudad ** , int , char * );

int fiRouEnviarTokenProximoRouter(struct sRouter ** ,char * ,struct sTDP ** ,struct sCiudad ** , char * );

int fiRouConectarProximoRouter(struct sCiudad ** , char * ,struct sTDP ** );

int fiRouSacarListas(struct sTDP ** ,struct sCiudad ** ,char * );

struct sCiudad * fiRouBuscarCiudadIPPuerto(struct sCiudad * , char * );

int comparaVector(char * , char * , int );

struct sCiudad * fiRouBuscarCiudadOrden(struct sCiudad * , int );

int fiRouConectar(char* );

int fiRouEnviarTDP(int ,struct sTDP** );

int fiRouRecibirPedido(int ,struct sDiscover ** , struct sRouter * ,char );
 
int fiRouEnviarDiscovers( int ,struct sRouter * , char ,char ,char * , char * ,char * );

int fiRouRecibirToken(int , char * , struct sTDP ** , struct sCiudad ** , struct sRouter *, char * , char *);

int fiRouManejarDiscover(int ,struct sDiscover** ,struct sRouter * , char * , int , char * , char , char , int );

int fiRouManejarEncontrado(int ,struct sDiscover ** ,char * );

int fiRouManejarNoEncontrado(int ,struct sDiscover ** , char *);

int fiEnviarFound(int ,char * ,char ,char ,char * ,char * ,char *);

int fiEnviarNotFound(int ,char * ,char ,char ,char * ,char * );

void fvRouManejarFound(int ,struct sDiscover ** , char * ,int ,char * ,char , char , int );

void fvRouManejarNotFound(int ,struct sDiscover ** , char * ,int ,char * ,char , char , int );

int fiRouEnviarFoundCiudad(int ,char *,char *,char *);

int fiRouEnviarNotFoundCiudad(int ,char *,char *);

int fiRouLimpiarListaTiempo(int,struct sDiscover ** , int );

void fvRouManejarDatos(int , struct sRouter * );

void fvEnviarDatos(int iSocket, char * );

int fiRouSacarRouterLista(struct sRouter ** ,int );

int sendall(int , char *, int );

int fiRouCompletarToken(struct sCiudad ** , struct sRouter * );

char * itoa( unsigned char, char *);

int fiRouPrintfIP(char *,char * , int *);

int recvAll(int , char * , int , int );
