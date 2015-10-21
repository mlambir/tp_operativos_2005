#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include "ciudadBib.h"
#include "ciudadCurs.h"

#define ENTER 10
#define ESCAPE 27

WINDOW * wTDP;
WINDOW * wEquipos;
WINDOW * wSalida;

void init_ciudad(void)
{
	char c;
	int h, w, hm, wm;
	char * titPrincipal = "LMDF - SISTEMAS OPERATIVOS - GRUPO:S.O.S.";
	char * titTDP = "TDP";
	char * titEquipos = "EQUIPOS";
	char * titInfo = "INFORMACION";
	init_curses();
	
	h = LINES;
	w = COLS;
	hm = h * 3/4;
	wm = w * 2/3;
	
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
	
	mvwvline(stdscr,1,wm, ACS_VLINE, hm-1);	
	mvwvline(stdscr,1,wm+1, ACS_VLINE, hm-1);	
	mvwhline(stdscr,hm,0, ACS_HLINE, w-1);	
	mvwhline(stdscr,hm+1,0, ACS_HLINE, w-1);	
	mvwaddch(stdscr, 1, wm, ACS_URCORNER);
	mvwaddch(stdscr, 1, wm+1, ACS_ULCORNER);
	mvwaddch(stdscr, hm, 0, ACS_LLCORNER);
	mvwaddch(stdscr, hm+1, 0, ACS_ULCORNER);
	mvwaddch(stdscr, hm, wm, ACS_LRCORNER);
	mvwaddch(stdscr, hm, wm+1, ACS_LLCORNER);
	mvwaddch(stdscr, hm, w-1, ACS_LRCORNER);
	mvwaddch(stdscr, hm+1, w-1, ACS_URCORNER);
	
	printTitle(1 , 1 , titTDP);
	printTitle(wm+2 , 1 , titEquipos);
	printTitle(1 , hm+1 , titInfo);

	wTDP=subwin(stdscr,hm-2,wm-1,2,1);	
	wEquipos=subwin(stdscr,hm-2,w-wm-3,2,wm+2);	
	wSalida=subwin(stdscr,h-hm-3,w-2,hm+2,1);	
	
	wbkgdset(wSalida, COLOR_PAIR(2));


	scrollok(wSalida, TRUE);
	

	wattrset(wTDP,COLOR_PAIR(2));	
	wattrset(wEquipos,COLOR_PAIR(2));	
	wattrset(wSalida,COLOR_PAIR(2));	
	
	wclrscr(wTDP);		
	wclrscr(wEquipos);		
	wclrscr(wSalida);		

	wrefresh(stdscr);
	
	touchwin(wEquipos);
	touchwin(wSalida);
	touchwin(wTDP);

	wrefresh(wEquipos);
	wrefresh(wSalida);
	wrefresh(wTDP);
}

void cerrar_curses(void)
{
	delwin(wTDP);   
	delwin(wEquipos);   
	delwin(wSalida);   
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

void imprimirTDP(struct sTDP* tabla)
{
	int i = 1;
	wattrset(wTDP, COLOR_PAIR(2)|A_BOLD);	
	wclrscr(wTDP);
	mvwprintw(wTDP,0,0, "%-19s %4s%4s%4s%7s", "NOMBRE", "E", "P", "G", "PUNTOS");
	wattrset(wTDP, COLOR_PAIR(2));	
	while(tabla)
	{
		mvwprintw(wTDP,i,0, "%-19.19s %4u%4u%4u%7u",tabla->nombre,(unsigned char) tabla->pe,(unsigned char) tabla->pp,(unsigned char) tabla->pg,(unsigned char) tabla->puntos);
		i++;
		tabla = tabla->siguiente;
	}
	wrefresh(wTDP);
}

void imprimirEquipos(struct sEquipo* lista)
{	
	struct sEquipo * aux;
	int i=0;
	char est;
	
	wclrscr(wEquipos);
	aux = lista;
	while(lista)
	{	if(lista->estado != ESTADOvacio)
		{
			if(lista->socket == 0)
				est = 'M';
			else if(lista->iTieneLev)
				est = 'L';
			else
				est = ' ';
			mvwprintw(wEquipos,i,0, "%-15.15s(%c)", lista->nombre, est);
			i++;
		}
		else
		{
			mvwprintw(wEquipos,i,0, "%s","Equipo Migrado");
			i++;
		}	
		lista = lista ->siguiente;
	}
	wrefresh(wEquipos);
}

void agregarTexto(char * texto)
{
	int y;
	int x;
	scroll(wSalida);
	getmaxyx(wSalida, y, x);
	mvwprintw(wSalida,y-1,0, "%s",texto);
	
	wrefresh(wSalida);
}

