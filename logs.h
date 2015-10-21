enum eTiposLog
{
    TLInformacion,
    TLAdvertencia,
    TLError,
    TLMensaje
};          

void fvStdIniciarLogs(char *, char *);
void fvStdLog(char *, int , int , char* , ...);
