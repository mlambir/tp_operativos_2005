#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "logs.h"

char cpNombreProceso[1024];
char cpNombreArchivoLogs[1024];

void fvStdIniciarLogs(char *nombreProceso, char *nombreArchivo)
{
	strcpy(cpNombreProceso, nombreProceso);
	strcpy(cpNombreArchivoLogs, nombreArchivo);
}

void fvStdLog(char * nombreArchivo, int nroPagina, int tipoLog, char* form, ...)
{
	time_t t;
	va_list ap;
	char cpFormato[1024];
	char cpFecha[512];
	char cpTipo[32];
	FILE	*archLogs;										/* output-file pointer */
	
	switch ( tipoLog )
	{
		case TLMensaje:	
						strcpy(cpTipo, "Mensaje");
			break;
		case TLInformacion:	
						strcpy(cpTipo, "Informacion");
			break;
		case TLError:	
						strcpy(cpTipo, "Error");
			break;
		case TLAdvertencia:	
						strcpy(cpTipo, "Advertencia");
			break;
		default:	
						strcpy(cpTipo, "");
			break;
	}					
	
	va_start(ap, form);
	t = time(&t);	
	strcpy(cpFecha, ctime(&t));
	cpFecha[strlen(cpFecha) - 1] = ' ';
	sprintf(&(cpFormato[0]), "%s %s[%d]: %s:(%s-%d) %s\n", cpFecha, cpNombreProceso, getpid(), cpTipo, nombreArchivo, nroPagina, form);

	archLogs	= fopen(cpNombreArchivoLogs, "a" );
	if ( archLogs == NULL )
	{
		fprintf ( stderr, "no se puede abrir el archivo '%s'; %s\n",
				cpNombreArchivoLogs, strerror(errno) );
		exit (EXIT_FAILURE);
	}

	vfprintf(archLogs, cpFormato, ap); 

	if( fclose(archLogs) == EOF )			/* close output file */
	{
		fprintf ( stderr, "no se puede cerrar el archivo '%s'; %s\n",
				cpNombreArchivoLogs, strerror(errno) );
		exit (EXIT_FAILURE);
	}
	va_end(ap);
}


