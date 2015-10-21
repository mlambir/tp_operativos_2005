#include <curses.h>
#include <stdio.h>
#include <stdlib.h>

#define ENTER 10
#define ESCAPE 27

typedef struct listaEsperaCDE
{
	int iSocket;
	int iMinutos;
	char * nombre;
	struct listaEsperaCDE* siguiente;
}listaEsperaCDE;

void init_cde(void);
void cerrar_curses(void);
void init_curses(void);
void clrscr(void);
void wclrscr(WINDOW * );
void printTitle(int , int , char *);
void imprimirEntrenando(struct listaEsperaCDE [], int );
void imprimirCola(struct listaEsperaCDE *);

	WINDOW * wEntrenando;
	WINDOW * wCola;

void init_cde(void)
{
	char c;
	int h, w, wm;
	char * titPrincipal = "LMDF - SISTEMAS OPERATIVOS - GRUPO:S.O.S.";
	char * titEntrenando = "ENTRENANDO";
	char * titCola = "ESPERANDO";
	init_curses();
	
	h = LINES;
	w = COLS;
	wm = w /2;
	
	mvprintw(0,w/2 - strlen(titPrincipal)/2, titPrincipal);	
				
	attrset(COLOR_PAIR(2));	
	mvwhline(stdscr,1,0, ACS_HLINE, w);	
	mvwvline(stdscr,1,0, ACS_VLINE, h-1);	
	mvwhline(stdscr,h-1,0, ACS_HLINE, w);	
	mvwvline(stdscr,1,w-1, ACS_VLINE, h-1);	
	mvwaddch(stdscr, 1, 0, ACS_ULCORNER);
	mvwaddch(stdscr, h-1, 0, ACS_LLCORNER);
	mvwaddch(stdscr, 1, w-1, ACS_URCORNER);
	mvwaddch(stdscr, h-1, w-1, ACS_LRCORNER);
	
	mvwvline(stdscr,1,wm, ACS_VLINE, h);	
	mvwvline(stdscr,1,wm+1, ACS_VLINE, h);	
	mvwaddch(stdscr, 1, wm, ACS_URCORNER);
	mvwaddch(stdscr, 1, wm+1, ACS_ULCORNER);
	mvwaddch(stdscr, h-1, wm, ACS_LRCORNER);
	mvwaddch(stdscr, h-1, wm+1, ACS_LLCORNER);
	
	printTitle(1 , 1 , titEntrenando);
	printTitle(wm+2 , 1 , titCola);

	wEntrenando=subwin(stdscr,h-3,wm-1,2,1);	
	wCola=subwin(stdscr,h-3,w-wm-3,2,wm+2);	
	
	wattrset(wEntrenando,COLOR_PAIR(2));	
	wattrset(wCola,COLOR_PAIR(2));	
	
	wclrscr(wEntrenando);		
	wclrscr(wCola);		

	wrefresh(stdscr);
}

void cerrar_curses(void)
{
	delwin(wEntrenando);   
	delwin(wCola);   
	endwin();          
}

void init_curses(void)
{
        initscr();
        start_color();
        init_pair(1,COLOR_WHITE,COLOR_RED);
        init_pair(2,COLOR_WHITE,COLOR_BLUE);
        init_pair(3,COLOR_YELLOW,COLOR_BLUE);
			attrset(COLOR_PAIR(1));	
			clrscr();
			wrefresh(stdscr);
			curs_set(0);
        noecho();
        keypad(stdscr,TRUE);
}
void clrscr(void)                
{
	wclrscr(stdscr);
}

void wclrscr(WINDOW * win)                
   {                             
   int y, x, maxy, maxx, yact, xact;         
	getyx(win, yact, xact);
   getmaxyx(win, maxy, maxx); 
   for(y=0; y < maxy; y++)       
      for(x=0; x < maxx; x++)    
         mvwaddch(win, y, x, ' ');    
	wmove(win, yact, xact); 
   }                             

void printTitle(int x, int y, char *titleText)
	{
			attrset(COLOR_PAIR(2));	
	mvaddch(y, x, ACS_RTEE);
	addch(' ');
			attrset(COLOR_PAIR(3)|A_BOLD);	
	addstr(titleText);
	addch(' ');
			attrset(COLOR_PAIR(2));	
	addch(ACS_LTEE);
	}

void imprimirEntrenando(struct listaEsperaCDE espera[], int cantidad)
{
	int i;
	wclrscr(wEntrenando);
	for(i=0;i<cantidad;i++)
		mvwprintw(wEntrenando,i,0, "%-15.15s", espera[i].nombre);
	wrefresh(wEntrenando);
}

void imprimirCola(struct listaEsperaCDE *lista)
{
	int i = 0;
	wclrscr(wCola);
	while(lista)
	{
		mvwprintw(wCola,i,0, "%-15.15s(%d)", lista->nombre,lista->iMinutos);
		lista = lista->siguiente;
i++;
	}
	wrefresh(wCola);
}
