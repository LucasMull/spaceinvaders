/*	COISAS A FAZER:
 *		CRIAR UMA JANELA SEPARADA OS TIROS E MOTHERSHIP
 *		IMPLEMENTAR BORDA SEM CAUSAR FLICKER
 */


#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <string.h>

#define ROW 38
#define COL 100

#define RIGHT 'r'
#define LEFT 'l'

#define TRUE 1
#define FALSE 0

#define SHIP11 "\n  A  \n AMA \n /X\\ \0"
#define SHIP12 "\n  A  \n AMA \n (X) \0"
#define SHIP21 "\n.v_v.\n}WMW{\n / \\ \0"
#define SHIP22 "\n.o_o.\n}WMW{\n ( ) \0"
#define SHIP31 "\n nmn \ndbMdb\n_/-\\_\0"
#define SHIP32 "\n nmn \nobMdo\n_(-)_\0"

#define TANK1 "\n /^\\ \nMMMMM\0"

#define MOSHIP1 "\n /MMMMM\\ \nAMoMoMoMA\n \\/'-'\\/ \0"
#define MOSHIP2 "\n /MMMMM\\ \nAoMoMoMoA\n \\/'-'\\/ \0"

#define COR_BORDA 0
#define COR_NAVE 1
#define COR_TANQUE 2
#define COR_TIRO_NAVE 3
#define COR_TIRO_TANQUE 4
#define COR_MOSHIP 5

struct t_node
{
 char *sprite1;
 char *sprite2; /*<- ignorado para elementos com só 1 sprite*/
 int posx;
 int posy;
 struct  t_node *prox;
 struct t_node *prev;
}; typedef struct t_node t_node;

struct t_lista
{
 t_node *atual;

 t_node *ini[5]; /*sentinela no ini de cada row*/
 t_node *fim[5]; /*sentinela no fim de cada row*/

 t_node *tanque; /*tanque no inicio da lista*/
 t_node *moship; /*nave mãe no fim da lista*/

 int updateField; /*controla temporização*/
 char direcao; /*alterna dir de mov das naves*/
 int qtd_naves;
 
}; typedef struct t_lista t_lista;

struct t_win /* para facilitar na hora de chamar janela nas funções */
{
 WINDOW *enemy;
 WINDOW *moship;
 WINDOW *enemyFire;

 WINDOW *tank;
 WINDOW *tankFire;
}; typedef struct t_win t_win;


int inicializaLista (t_lista *l)
{
 t_node *first[5];
 t_node *last[5];
 t_node *tanque, *moship;
 int i;

 for ( i=0; i<5; i++)
 {
	first[i] = (t_node*) malloc(sizeof(t_node));
	last[i] = (t_node*) malloc(sizeof(t_node));

	if ( first[i] == NULL || last[i] == NULL )
	{
		free(first[i]);
		free(last[i]);
		return 0;
	}

 }

 tanque = (t_node*) malloc(sizeof(t_node));
 moship = (t_node*) malloc(sizeof(t_node));
 if ( tanque == NULL || moship == NULL )
 {
	free(tanque);
	free(moship);
	return 0;
 }
 l->tanque = tanque;
 tanque->prox = first[0];
 tanque->prev = NULL;

 l->moship = moship;
 moship->prev = last[4];
 moship->prox = NULL;

 l->ini[0] = first[0];
 first[0]->prox = last[0];
 first[0]->prev = tanque;
 l->fim[0] = last[0];
 last[0]->prox = first[1];
 last[0]->prev = first[0];
 
 for ( i=1; i<5; i++ )
 {
 	l->ini[i] = first[i];
	first[i]->prox = last[i];
	first[i]->prev = last[i-1];
 	l->fim[i] = last[i];
	if ( i+1 < 5)
		last[i]->prox = first[i+1];
	else
		last[i]->prox = moship;
	last[i]->prev = first[i];
 }
 
 l->qtd_naves = 0;

 return 1;
}

int inicializaNaves (t_lista *l)
{
 t_node* new_ship;
 int posRow = 8, posCol = 5;
 int i, naves = 0;
 
 l->moship->sprite1 = MOSHIP1;
 l->moship->sprite2 = MOSHIP2;
 l->tanque->sprite1 = TANK1;

 l->tanque->posx = COL/2;
 l->tanque->posy = 0; /*posição é 0, porém a window do tanque foi deslocada para baixo das naves*/
 
 l->moship->posx = COL/2;
 l->moship->posy = 1;

 for ( i=0; i<5; i++ )
 {
 	while ( ++naves % 12 != 0 ) /*garante que seja printado 55 naves, 11 por row*/
	{
		new_ship = (t_node*) malloc(sizeof(t_node));
		if ( new_ship == NULL )
			 return 0;
		
		new_ship->prox = l->fim[i];
	        new_ship->prev = l->fim[i]->prev;
		l->fim[i]->prev->prox = new_ship;
		l->fim[i]->prev = new_ship;
		
		new_ship->posx = posRow;
		posRow += 8;	
		new_ship->posy = posCol;
		switch (i)
		{
			case 0:
				new_ship->sprite1 = SHIP11;
				new_ship->sprite2 = SHIP12;
				break;
			case 1:
			case 2:
				new_ship->sprite1 = SHIP21;
				new_ship->sprite2 = SHIP22;
				break;
			case 3:
			case 4:
				new_ship->sprite1 = SHIP31;
				new_ship->sprite2 = SHIP32;
				break;
			default:
				break;
		}

		l->qtd_naves++; /*qtd de naves total*/
	}
	posCol += 4;
	posRow = 8;
 }

 return 1;
}

void printTanque(t_lista *l, WINDOW *win)
{
	int row, col, j;

	row=0; col=-2; j=0;
	while ( l->tanque->sprite1[j] != '\0' )
	{
		if ( l->tanque->sprite1[j] == '\n' )
		{
			row++;
			col=-2;
		}
		else
		{
			wmove(win, l->tanque->posy+row, l->tanque->posx+col);
			waddch(win, l->tanque->sprite1[j]);
		}
		col++;
		j++;
	}
	wrefresh(win);
}

void printNaves(t_lista *l, t_win *win)
{
	int i, j; /*contadores*/
	int col, row; /*para formatação dos chars da nave*/

	/* TEMPORARIO APENAS PARA VISUALIZACAO DA MOSHIP 
	 * CRIAR FÇ DEPOIS */
	row=0; col=-4; j=0;
	while ( l->moship->sprite1[j] != '\0' )
	{
		if ( l->moship->sprite1[j] == '\n' )
		{
			row++;
			col=-4;
		}
		else
		{	
			if ( l->moship->posx % 2 == 0 )
			{
				wmove(win->moship, l->moship->posy+row, l->moship->posx+col);
				waddch(win->moship, l->moship->sprite1[j]);
			}
			else
			{
				wmove(win->moship, l->moship->posy+row, l->moship->posx+col);
				waddch(win->moship, l->moship->sprite2[j]);
			}
		}
		col++;
		j++;
	}

	for ( i=0; i<5; i++ )
	{
		l->atual = l->ini[i]->prox;
		while ( l->atual != l->fim[i] )
		{
		  row=0; col=-2; j=0;
		  while ( l->atual->sprite1[j] != '\0' )
		  {
			if( l->atual->sprite1[j] == '\n' )
			{
				row++;
				col=-2;
			}
			else
			{
				if ( l->atual->posx % 2 == 0 ) /* animação do sprite das naves alterna em posx par e impar*/
				{
					wmove(win->enemy, l->atual->posy+row, l->atual->posx+col);
					waddch(win->enemy, l->atual->sprite1[j]);
				}
				else
				{
					wmove(win->enemy, l->atual->posy+row, l->atual->posx+col);
					waddch(win->enemy, l->atual->sprite2[j]);
				}
			}
			col++;
			j++;
		  }
		  l->atual = l->atual->prox;
		}
	}
	wrefresh(win->enemy);
	wrefresh(win->moship);
}

void desceNaves(t_lista *l, WINDOW *win)
{
	t_node *aux;

	aux = l->fim[4]->prev;
	while ( aux != l->ini[0] )
	{
		if ( aux->posy+1 < ROW-6 )
		{
			aux->posy++;
			aux = aux->prev;
		}
		else /* naves descem no row do tanque e gameover */
		{
			endwin();
			exit(1);
		}
	}
}

void movimentaMoShip(t_lista *l, WINDOW *win)
{
	if ( l->moship->posx+6 < COL )
		l->moship->posx++;
	else
		l->moship->posx = 5;
	wclear(win);
}

void movimentaNaves(t_lista *l, WINDOW *win)
{
	int i;
	int SWITCH = FALSE; /*para controlar saída do nested loop*/ 

	if ( l->direcao == RIGHT )
	{
		for ( i=0; i<5; i++ )
		{
			if ( SWITCH == TRUE )
				break;
			
			l->atual = l->fim[i]->prev;
			while ( l->atual != l->ini[i] )
			{
				if ( l->atual->posx < COL-5 )
					l->atual->posx++;
				else
				{
					l->direcao = LEFT;
					desceNaves(l, win);
					
					SWITCH = TRUE; /*força saída do nested loop*/
					break;
				}

				l->atual = l->atual->prev;
			}
		}
	}
	else/*if ( l->direcao == LEFT ) */
	{
		for ( i=4; i>=0; i-- )
		{
			if ( SWITCH == TRUE )
				break;
			
			l->atual = l->ini[i]->prox;
			while ( l->atual != l->fim[i] )
			{
				if ( l->atual->posx > 2 )
					l->atual->posx--;
				else
				{
					l->direcao = RIGHT;
					desceNaves(l, win);
					
					SWITCH = TRUE; /*força saída do nested loop*/
					break;
				}

				l->atual = l->atual->prox;
			}
		}

	}
	
	wclear(win);
}

void movimentaTanque(t_lista *l, WINDOW *win, int key)
{
	switch (key)
	{
		case ' ': /* BOTÃO DE TIRO */
			break;
		case 'a': /* BOTÃO DE ANDAR PARA ESQUERDA */
			if ( l->tanque->posx - 1 > 2 )
				l->tanque->posx--;
			wclear(win);
			break;
		case 'd': /* BOTÃO DE ANDAR PARA DIREITA */
			if ( l->tanque->posx + 1 < COL-5 )
				l->tanque->posx++;
			wclear(win);
			break;	
		case 'q': /* BOTÃO DE EXIT */
			endwin();
			exit(1);
		case 'p': /* BOTÃO DE PAUSE */
			key = 0;
			while ( (key=getch()) != 'p')
			{
				if ( key == 'q' ) /* PARA PODER SAIR DO JOGO DURANTE O PAUSE */
				{
					endwin();
					exit(1);
				}
				usleep(10000);
			}
			break;
		default:
			break;
	}
}

int GameOn(t_lista *l, t_win *win)
{
	int key;
	int utime = 10000;
 
	if ( l->updateField % 70000 == 0 )
		movimentaMoShip(l, win->moship);

	if ( l->updateField % 100000 == 0 )
	{	
		printTanque(l, win->tank);
		printNaves(l, win);
	}

	key = getch();
	movimentaTanque(l, win->tank, key);
	if ( l->updateField % 300000 == 0 )
	{
		movimentaNaves(l, win->enemy);
		l->updateField = 0;
	}

	usleep(utime);
	l->updateField += utime;
	if ( l->qtd_naves == 0 )
		return 0;

	return 1;
}

void borda(int y1, int x1, int y2, int x2)
{
    mvhline(y1, x1, 0, x2-x1);
    mvhline(y2, x1, 0, x2-x1);
    mvvline(y1, x1, 0, y2-y1);
    mvvline(y1, x2, 0, y2-y1);
    mvaddch(y1, x1, ACS_ULCORNER);
    mvaddch(y2, x1, ACS_LLCORNER);
    mvaddch(y1, x2, ACS_URCORNER);
    mvaddch(y2, x2, ACS_LRCORNER);
}

int main()
{
/* START ncurse */
  initscr();
  cbreak();
  noecho();
  curs_set(FALSE);
  nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE);
  start_color();
/* WINDOW properties */
  int gety, getx;
  int key;
/* GAME properties */
  t_lista l;
  	l.updateField = 0;
	l.direcao = RIGHT;
  t_win win;
	win.moship = newwin(5,COL,1,1);
  	win.enemy = newwin(ROW-3,COL,1,1);
  	win.tank = newwin(3,COL,ROW-3,1);

  getmaxyx(stdscr, gety, getx);
  while (key != 'a')
  {
        if ( getx<COL || gety<ROW )
        {
                clear();
                mvprintw(gety/2,getx/3,"TERMINAL SIZE: %dx%d", getx, gety);
                mvprintw( (gety/2)+1, (getx/3), "MINIMUM SIZE REQUIRED: 100x38");
                flushinp(); /*limpa buffer de input*/
        }
        else
        {
                clear();
                mvprintw(gety/2,(getx/2)-7,"PUSH 'a' TO BEGIN");
                key = getch();
        }
        refresh();
        getmaxyx(stdscr,gety, getx);
        usleep(10000);
  }
  clear();

  init_pair(COR_NAVE, COLOR_GREEN, COLOR_BLACK);
  wattron(win.enemy, COLOR_PAIR(COR_NAVE));
  wattron(win.enemy, A_BOLD);
  
  init_pair(COR_TANQUE, COLOR_MAGENTA, COLOR_BLACK);
  wattron(win.tank, COLOR_PAIR(COR_TANQUE));
  wattron(win.tank, A_BOLD);
 
  init_pair(COR_MOSHIP, COLOR_RED, COLOR_BLACK);
  wattron(win.moship, COLOR_PAIR(COR_MOSHIP));
  wattron(win.moship, A_BOLD);
  borda(0, 0, ROW, COL+1);
  
  inicializaLista(&l);
  inicializaNaves(&l);
  
  while (GameOn(&l, &win));
  sleep(2);
  
  endwin();
  return 0;
}
