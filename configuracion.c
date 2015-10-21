#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>

#include "configuracion.h"

void fvStdTomarConfiguracion(char *nomarchivo, char *argumento, char **salida)
{
	FILE *conf;
	char *leido;
	char *var;
	char *datos;

	if ((conf = fopen (nomarchivo, "r")) == NULL)
	{
		printf("error, no se puede abrir el archivo %s\n", nomarchivo);
		exit (1);
	}
	(*salida) = NULL;
	leido = (char *) malloc (1024);
	while ((fgets (leido, 1024, conf) != NULL))
	{
		var = strtok (leido, ":");
		datos = strtok (NULL, "\n");
		if (!strcmp (var, argumento))
		{
			(*salida)= (char*)malloc (strlen(datos)+1);
			if ( (*salida)==NULL )
			{
				return;
			}
//			strcpy ((*salida), datos);
			memcpy(*salida,datos,strlen(datos)+1);
		}
	}
	free(leido);
	fclose (conf);
	return;
}
