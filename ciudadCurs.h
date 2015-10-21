#define ENTER 10
#define ESCAPE 27

void init_curses(void);
void clrscr(void);                
void wclrscr(WINDOW *);                
void hacerenter(WINDOW *);                
void printTitle(int , int , char *);
void imprimirTDP(struct sTDP *);
void imprimirEquipos(struct sEquipo *);
void init_ciudad(void);
void agregarTexto(char * );
void InsertLineAtBottom(WINDOW *);
