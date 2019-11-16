/*	COISAS A FAZER:
 *		INVESTIGAR PORQUE POSIÇÃO DOS ELEMENTOS DA STRUCT N RESETAM NO RETURN MAIN()
 *		FAZER COM QUE O GAMEOVER EXCLUA TODOS OS PONTEIROS E DÊ FREE
 *		CRIAR FUNÇÕES GENÉRICAS
 *		TERMINAR DE COMENTAR O CÓDIGO
 *		CRIAR INSTRUÇÃO DE CONTROLES ANTES DO JOGO INICIAR
 */


#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "asciiART.h"

#define ROW 38
#define COL 100

#define RIGHT 'r'
#define LEFT 'l'

#define TRUE 1
#define FALSE 0

#define COR_TANQUE 1
#define COR_TIRO_TANQUE 2
#define COR_MACACO 3
#define COR_SCORE 4
#define COR_NAVE 5
#define COR_MADEIRA 6
#define COR_MOSHIP 7
#define COR_BARREIRA 8
#define COR_TIRO_NAVE 9
#define COR_BANANA 10

#define COLOR_BROWN 52
#define COLOR_ORANGE 154
#define COLOR_PINK 229
#define COLOR_YELLOW2 220

struct t_node
{
 int chave; /*para fins de identificação*/

 char *sprite1; /*primeira aparência do elemento do nodo*/
 char *sprite2; /*segunda aparência do elemento do nodo*/
 
 int posx; /*posição x*/
 int posy; /*posição y*/
 
 struct  t_node *prox; /*próximo nodo*/
 struct t_node *prev; /*nodo anterior*/
}; typedef struct t_node t_node;

struct t_wall
{
 t_node *atual;

 t_node *ini;
 t_node *fim;

 int tamanho;
}; typedef struct t_wall t_wall;

struct t_tiro
{
 t_node *atual;

 t_node *ini;
 t_node *fim;

 int qtd_tiros1;
 int tamanho;
}; typedef struct t_tiro t_tiro;

struct t_lista
{
 t_node *atual; /*auxiliar para percorrer a lista etc*/

 t_node *ini[5]; /*sentinela no ini de cada row*/
 t_node *fim[5]; /*sentinela no fim de cada row*/

 t_node *tanque; /*tanque no inicio da lista*/
 t_node *moship; /*nave mãe no fim da lista*/

 int updateField; /*controla temporização*/
 char direcao; /*alterna dir de mov das naves*/
 int tamanho;
 int score;
 unsigned int speed;
}; typedef struct t_lista t_lista;

struct t_win /* para facilitar na hora de chamar janela nas funções */
{
 WINDOW *enemy; /*janela que contém naves inimigas*/
 WINDOW *moship; /*janela que contém nave mãe*/
 WINDOW *tank; /*janela que contém tanque*/
 WINDOW *fire1; /*janela que contém tiro do player*/
 WINDOW *fire2; /*janela que contém tiro inimigo*/
 WINDOW *score; /*janela que contém animação de pontuação*/
 
 int gety, getx; /*para guardar definições das dimensões do terminal*/
}; typedef struct t_win t_win;

void titleScreen(t_win *win)
{
  int key;
  WINDOW *temp, *temp2;
  
  while ( win->getx<COL || win->gety<ROW )
  {
  	clear();
  	getmaxyx(stdscr, win->gety, win->getx);
        mvprintw(win->gety/2,win->getx/2-12,"TERMINAL SIZE: %dx%d", win->getx, win->gety);
        mvprintw((win->gety/2)+1, win->getx/2-12, "MINIMUM SIZE REQUIRED: 100x38");
	usleep(60000);
        flushinp(); /*limpa buffer de input*/
	refresh();
  }
  
  temp = newwin(ROW/2+1,COL-5,(win->gety-ROW)/2+5,(win->getx-COL)/2+4);
  init_pair(30, COLOR_GREEN, COLOR_BLACK);
  wattron(temp, COLOR_PAIR(30) | A_BOLD );
  temp2 = newwin(ROW,COL,(win->gety-ROW)/2,(win->getx-COL)/2);
  clear(); 
  while (key != 'a')
  {
	wclear(temp);
	wclear(temp2);
	wmove(temp,win->gety/2-15,win->getx/2);
        wprintw(temp,"\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s",T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15);
	wmove(temp2,win->gety-10,win->getx/2-43);
        wprintw(temp2,"PUSH 'a' TO BEGIN\n\n\n\n\n\n\n github.com/LucasMull");
	box(temp2, '*', '*');
	wrefresh(temp2);
	wrefresh(temp);
	
	key = getch();
	usleep(60000);
  }
  flushinp(); /*limpa buffer de input*/
  
  clear();
}

void GameOver(t_lista *l, t_win *win)
{
  	WINDOW *temp, *temp2;
	int key;

	wclear(stdscr);
	temp = newwin(ROW/2-5,COL-15,(win->gety-ROW)/2+5,(win->getx-COL)/2+15);
	init_pair(30, COLOR_RED, COLOR_BLACK);
	wattron(temp, COLOR_PAIR(30) | A_BOLD );
	temp2 = newwin(ROW-1,COL,(win->gety-ROW)/2+1,(win->getx-COL)/2);
	
	wmove(temp,win->gety/2-15,win->getx/2);
	wprintw(temp,"\n%s\n%s\n%s\n%s\n%s\n%s\n%s",GO1,GO2,GO3,GO4,GO5,GO6,GO7);
	wmove(temp2,win->gety/2,win->getx/2-38);
	wprintw(temp2,"SCORE: %d", l->score);
	mvwprintw(temp2,ROW-10,win->getx/2-46,"PUSH 'r' TO START NEW GAME" );
	mvwprintw(temp2,ROW-9,win->getx/2-41,"PUSH 'q' TO QUIT" );
	
	while ( (key=getch())!='q' )
	{	
		if (key == 'r') /*termina processo atual e reinicia um novo no lugar, a partir de um código bash*/
		{	
			endwin();
			system("./.newgame");
			exit(1);
		}
		wrefresh(temp2);
		wrefresh(temp);
		usleep(10000);
	}	
	endwin();
	exit(1);	
}

/* CRIA SENTINELAS NA LISTA PARA FACILITAR O ACESSO E MANIPULAÇÃO
 * COLOCA O TANQUE NO INICIO DA LISTA E NAVE MAE NO FIM */
int inicializaListaNaves (t_lista *l)
{
 t_node *first[5];
 t_node *last[5];
 t_node *tanque_node, *moship_node;
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

 tanque_node = (t_node*) malloc(sizeof(t_node));
 moship_node = (t_node*) malloc(sizeof(t_node));
 if ( tanque_node == NULL || moship_node == NULL )
 {
	free(tanque_node);
	free(moship_node);
	return 0;
 }
 l->tanque = tanque_node;
 tanque_node->prox = first[0];
 tanque_node->prev = NULL;

 l->moship = moship_node;
 moship_node->prev = last[4];
 moship_node->prox = NULL;

 l->ini[0] = first[0];
 first[0]->prox = last[0];
 first[0]->prev = NULL;
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
		last[i]->prox = NULL;
	last[i]->prev = first[i];
 }
 l->tamanho = 0;

 return 1;
}

void destroiListaBarreira (t_wall *w)
{
	t_node *aux;

	if( w->tamanho > 0 )
	{
		w->atual = w->ini->prox;
		while(w->atual->prox != w->fim)
		{
			aux = w->atual->prox;
			aux->prev->prev = NULL;
			aux->prev->prox = NULL;
			free(w->atual);
			w->atual = aux;
		}
		free(aux);
		aux = NULL;
	}
}

void destroiListaTiros (t_tiro *t)
{
	t_node *aux;

	if( t->tamanho > 0 )
	{
		t->atual = t->ini->prox;
		while(t->atual->prox != t->fim)
		{
			aux = t->atual->prox;
			aux->prev->prev = NULL;
			aux->prev->prox = NULL;
			free(t->atual);
			t->atual = aux;
		}
		free(aux);
		aux = NULL;
	}
}

int inicializaListaTiros (t_tiro *t)
{
 t_node *first;
 t_node *last;
 
 if (t->ini)
	 free(t->ini);
 if (t->fim)
 	free(t->fim);

 first = (t_node*)malloc(sizeof(t_node));
 last = (t_node*)malloc(sizeof(t_node));
 if ( first == NULL || last == NULL )
 {
	free(first);
	free(last);
	return 0;
 }
 t->ini = first;
 first->prox = last;
 first->prev = NULL;
 t->fim = last;
 last->prev = first;
 last->prox = NULL;

 t->qtd_tiros1 = 0;
 t->tamanho = 0;

 return 1;
}

/* PREENCHE A LISTA COM OS ELEMENTOS 
 * DÁ APARÊNCIA AOS MESMOS */
int inicializaNaves (t_lista *l)
{
 t_node* new_ship;
 int posRow, posCol; /*posições iniciais para posicionar naves*/
 int i, naves; /*contadores*/
 
 l->moship->sprite1 = MOSHIP1;
 l->moship->sprite2 = MOSHIP2;
 l->moship->posx = 5;
 l->moship->posy = 1;
 l->moship->chave = FALSE;

 l->tanque->sprite1 = TANK1;
 l->tanque->sprite2 = TANK2;
 l->tanque->posx = COL/2;
 l->tanque->posy = ROW-3;
 l->tanque->chave = -1;

 posCol = 5;
 naves = 0;
 for ( i=0; i<5; i++ ) /*percorre as 5 sentinelas da lista*/
 {
	posRow = 8; /*nave da próxima fileira retorna à posx inicial*/
 	while ( ++naves % 12 != 0 ) /*garante que seja printado 55 naves, 11 por row*/
	{
		new_ship = (t_node*) malloc(sizeof(t_node));
		if ( new_ship == NULL )
			 return 0;
		
		/*organiza as naves como linked list*/
		new_ship->prox = l->fim[i];
	        new_ship->prev = l->fim[i]->prev;
		l->fim[i]->prev->prox = new_ship;
		l->fim[i]->prev = new_ship;
		
		/*atualiza a posição individual de cada nave*/
		new_ship->posx = posRow;
		posRow += 8; /*posiciona naves de 8 em 8 casas*/
		new_ship->posy = posCol;
		switch (i) /*atualiza o sprite da nave a partir de sua sentinela*/
		{
			case 0:
				new_ship->sprite1 = SHIP11;
				new_ship->sprite2 = SHIP12;
				new_ship->chave = 30;
				break;
			case 1:
			case 2:
				new_ship->sprite1 = SHIP21;
				new_ship->sprite2 = SHIP22;
				new_ship->chave = 20;
				break;
			case 3:
			case 4:
				new_ship->sprite1 = SHIP31;
				new_ship->sprite2 = SHIP32;
				new_ship->chave = 15;
				break;
			default:
				break;
		}
		l->tamanho++; /*qtd de naves total*/
	}
	posCol += 4; /*pula fileira de 4 em 4*/
 }

 return 1;
}

int inicializaListaBarreiras(t_wall *w)
{
 t_node *first;
 t_node *last;

 if (w->ini)
	 free(w->ini);
 if (w->fim)
 	free(w->fim);

 first = (t_node*)malloc(sizeof(t_node));
 last = (t_node*)malloc(sizeof(t_node));
 if ( first == NULL || last == NULL )
 {
	free(first);
	free(last);
	return 0;
 }
 w->ini = first;
 first->prox = last;
 first->prev = NULL;
 w->fim = last;
 last->prev = first;
 last->prox = NULL;

 w->tamanho = 0;

 return 1;
}

int inicializaBarreiras(t_wall *w, void *mat[ROW][COL])
{
	int row, col;
	int i,j;
	t_node *nodo_solto;
	
	for( i=0; i<84; i+=21)
	{
		j=0;
		col=14;
		row = ROW-8;
		while ( BARREIRA[j] != '\0' )
		{
			if( BARREIRA[j] == '\n' ){
				row++;
				col=14;
			}
			else if ( BARREIRA[j] != ' ' )
			{
				nodo_solto = (t_node*)malloc(sizeof(t_node));
				if (nodo_solto == NULL){
					free(nodo_solto);
					return 0;
				}
				nodo_solto->prox = w->fim;
				nodo_solto->prev = w->fim->prev;
				w->fim->prev->prox = nodo_solto;
				w->fim->prev = nodo_solto;
				
				nodo_solto->chave = -2;
				nodo_solto->posy = row;
				nodo_solto->posx = col+i;
				mat[row][col+i] = nodo_solto;
				
				w->tamanho++;
			}
			col++;
			j++;
		}
	}
	return 1;
}

void printBarreiras(WINDOW *win, void *mat[ROW][COL])
{
	int row, col;
	int i,j;
	t_node *nodoDetectado;

	for( i=0; i<84; i+=21)
	{
		j=0;
		col=14;
		row = ROW-8;
		
		while ( BARREIRA[j] != '\0' )
		{
			if( BARREIRA[j] == '\n' ){
				row++;
				col=14;
			}
			else if (( nodoDetectado = mat[row][col+i] ) && ( nodoDetectado->chave == -2 ))
			{
				wmove(win, row, col+i);
				if (BARREIRA[j] == ')')
					waddch(win, BARREIRA[j] | COLOR_PAIR(COR_BANANA));
				else if (BARREIRA[j] == '(')
					waddch(win, BARREIRA[j] | COLOR_PAIR(COR_TIRO_TANQUE));
				else if (BARREIRA[j] != '#')
					waddch(win, BARREIRA[j] | COLOR_PAIR(COR_BARREIRA));
				else
					waddch(win, BARREIRA[j] | COLOR_PAIR(COR_MADEIRA));
			}

			col++;
			j++;
		}
	}
}

/* PRINTA O TANQUE DO USUÁRIO 
 * 	lembrando que a posição do tanque é virtual uma
 * 	vez que a sua window foi arrastada para baixo 
 * 	e não a posy do tanque */
void printTanque(t_lista *l, WINDOW *win)
{
	int j; /*contador*/
	int row, col; /*variáveis para posicionar o char na tela*/

	j=0;
	row=-1; 
	while ( l->tanque->sprite1[j] != '\0' ) /*percorre string até seu fim*/
	{
		if ( l->tanque->sprite1[j] == '\n' ){ /*pula uma casa junto com o newline*/
			row++;
			col=-3;
		}
		else
		{
			if ( l->tanque->posx % 2 == 0 ){
				wmove(win, l->tanque->posy+row, l->tanque->posx+col); /*atualiza posição para printar o char*/
				if (l->tanque->sprite1[j] != 'm')
					waddch(win, l->tanque->sprite1[j]|COLOR_PAIR(COR_MACACO)); /*printa o char na posição*/
				else
					waddch(win, l->tanque->sprite1[j]); /*printa o char na posição*/
			}
			else{
				wmove(win, l->tanque->posy+1, l->tanque->posx+3);
				waddch(win, '(' | COLOR_PAIR(COR_TIRO_TANQUE) );
				wmove(win, l->tanque->posy+row, l->tanque->posx+col); /*atualiza posição para printar o char*/
				if (l->tanque->sprite2[j] != 'm')
					waddch(win, l->tanque->sprite2[j]|COLOR_PAIR(COR_MACACO)); /*printa o char na posição*/
				else
					waddch(win, l->tanque->sprite2[j]); /*printa o char na posição*/
			}
		}
		col++;
		j++;
	}
}

void printNaves(t_lista *l, t_win *win)
{
	int i, j; /*contadores*/
	int col, row; /*variáveis para posicionar o char na tela*/

	j=0;
	row=-1; 
	while ((l->moship->chave == TRUE) && (l->moship->sprite1[j] != '\0'))
	{
		if ( l->moship->sprite1[j] == '\n' ){
			row++;
			col=-4;
		}
		else
		{	
			if ( l->moship->posx % 2 == 0 ){
				wmove(win->moship, l->moship->posy+row, l->moship->posx+col);
				waddch(win->moship, l->moship->sprite1[j]);
			}
			else{
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
		  j=0;
		  row=-1;
		  while ( l->atual->sprite1[j] != '\0' )
		  {
			if( l->atual->sprite1[j] == '\n' ){
				row++;
				col=-2;
			}
			else
			{
				if ( l->atual->posx % 2 == 0 ){ /* animação do sprite das naves alterna em posx par e impar*/
					wmove(win->enemy, l->atual->posy+row, l->atual->posx+col);
					waddch(win->enemy, l->atual->sprite1[j]);
				}
				else{
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
}

void desceNaves(t_lista *l, t_win *win)
{
	t_node *aux;

	aux = l->fim[4]->prev;
	while ( aux != l->ini[0] )
	{
		if ( aux->posy+1 < ROW-6 ){
			aux->posy++;
			aux = aux->prev;
		}
		else{ /* naves descem no row do tanque e gameover */
			sleep(2);
			GameOver(l, win);
		}
	}
}

void limpaNodoMatriz(void *mat[ROW][COL], t_node *atual)
{
	int i, j;
	int fim_i;
	int ini_j, fim_j;

	switch ( atual->chave )
	{
		case -1:
			fim_i = 1;
			ini_j = -2;
			fim_j = 3;
			break;
		case TRUE: /*caso da NAVE MAE*/
			fim_i = 2;
			ini_j = -4;
			fim_j= 5;
			break;
		default:
			fim_i = 2;
			ini_j = -2;
			fim_j = 4;
			break;
	}		
	 
	for ( i=-1;i<=fim_i;i++ )
		for ( j=ini_j;j<=fim_j;j++ ){
			/*mvaddch(atual->posy+i,atual->posx+j+COL/2-16, '@');*/
			mat[atual->posy+i][atual->posx+j] = NULL;
		}
}

void atualizaMatriz(void *mat[ROW][COL], t_node *atual)
{
	 int row, col;
	 int j, ini_col;

	 j=0;row=-1;
	 if ( atual->chave == TRUE )
		 ini_col = -5;
	 else
		 ini_col = -2;


         while ( atual->sprite1[j] != '\0' )
         {
         	if( atual->sprite1[j] == '\n' ){
                	row++;
                        col=ini_col;
                }
		else if ( atual->sprite1[j] != ' ' ){
			/*mvaddch(atual->posy+row,atual->posx+col+COL/2-16, '@');*/
			mat[atual->posy+row][atual->posx+col] = atual;
		}
		
		col++;
                j++;
	 }
}

void movimentaMoShip(t_lista *l, WINDOW *win, void *mat[ROW][COL])
{
	limpaNodoMatriz(mat, l->moship);
	if ( l->moship->posx+6 < COL )
		l->moship->posx++;
	else{
		l->moship->posx = 5;
		l->moship->chave = FALSE;
	}
	atualizaMatriz(mat, l->moship);
}

void movimentaNaves(t_lista *l, t_win *win, void *mat[ROW][COL])
{
	int i;
	int SWITCH; /*para controlar saída do nested loop*/ 

	SWITCH = FALSE;
	for ( i=0; i<5; i++ )
	{
		if ( SWITCH == TRUE )
			break;

		if ( l->direcao == RIGHT )
		{
			l->atual = l->fim[i]->prev;
			while ( l->atual != l->ini[i] )
			{
				limpaNodoMatriz(mat, l->atual);
				if ( l->atual->posx < COL-5 ){
					l->atual->posx++;
					atualizaMatriz(mat, l->atual);
				}
				else
				{
					l->direcao = LEFT;
					desceNaves(l, win);
					atualizaMatriz(mat, l->atual);
						
					SWITCH = TRUE; /*força saída do nested loop*/
					break;
				}
				l->atual = l->atual->prev;
			}
		}
		else/*if ( l->direcao == LEFT ) */
		{
			l->atual = l->ini[i]->prox;
			while ( l->atual != l->fim[i] )
			{
				limpaNodoMatriz(mat, l->atual);
				if ( l->atual->posx > 2 ){
					l->atual->posx--;
					atualizaMatriz(mat, l->atual);
				}
				else
				{
					l->direcao = RIGHT;
					desceNaves(l, win);
					atualizaMatriz(mat, l->atual);
						
					SWITCH = TRUE; /*força saída do nested loop*/
					break;
				}
				l->atual = l->atual->prox;
			}

		}
	}
}

int inicializaTiros(t_lista *l, t_tiro *t, int ID)
{
	t_node *new_fire;

	new_fire = (t_node*) malloc(sizeof(t_node));
	if ( new_fire == NULL )
		return 0;
		
	/*organiza os tiros como linked list*/
	new_fire->prox = t->fim;
	new_fire->prev = t->fim->prev;
	t->fim->prev->prox = new_fire;
	t->fim->prev = new_fire;

	new_fire->chave = ID;
	if ( ID == 1 )
	{
		/*atualiza a posição individual de cada tiro*/
		new_fire->posx = l->tanque->posx;
		new_fire->posy = ROW-4;
		
		new_fire->sprite1 = TIRO1;
		t->qtd_tiros1++;
	}
	else/*if ( ID == 2 )*/
	{
		new_fire->posx = l->atual->posx+1;
		new_fire->posy = l->atual->posy+3;
		
		new_fire->sprite1 = TIRO2;
	}
	t->tamanho++;
	new_fire->sprite2 = MORTE;
	return 1;
}

void removeAtualLista(t_node *atual)
{
	if ( atual->prev )
		atual->prev->prox = atual->prox;
	if ( atual->prox )
		atual->prox->prev = atual->prev;
	free(atual);
	atual = NULL;
}

/*FUNÇÃO RESPONSÁVEL POR MOVIMENTAR O TIRO DO USUÁRIO
 * E DOS INIMIGOS, A PARTIR DE UMA LISTA LINKADA
 * E CHECAR AS CONDIÇÕES DE COLISÃO ENTRE TIRO E OUTROS ELEMENTOS */
void movimentaTiros(t_tiro *t, t_lista *l, t_wall *w, t_win *win, void *mat[ROW][COL])
{
	t_node *nodoDetectado;

	t->atual = t->ini->prox;
	if ( (t->tamanho) )
	{
		while (t->atual != t->fim)
		{
			nodoDetectado = mat[t->atual->posy][t->atual->posx];
			if ( t->atual->chave == 1 )
			{
				wmove(win->fire1, t->atual->posy, t->atual->posx);
				waddch(win->fire1, ' ');

				if ( nodoDetectado ) /*detecção de colisão do tiro com nave inimiga*/
				{
					if (nodoDetectado->chave == -2){ /*se o nodoDetectado for uma barreira*/
						mat[t->atual->posy][t->atual->posx] = NULL;
						removeAtualLista(nodoDetectado);
						w->tamanho--;
					}
					else if (nodoDetectado->chave == TRUE){
						l->score += 100;
						limpaNodoMatriz(mat, nodoDetectado);
						l->moship->chave = FALSE;
						l->moship->posx = 5;
					}
					else /*se for uma nave*/
					{
						l->score += nodoDetectado->chave;
						l->tamanho--;
						if( l->speed > 10000 )
							l->speed-=10000;	

						removeAtualLista(nodoDetectado);
						limpaNodoMatriz(mat, nodoDetectado);
					}
					removeAtualLista(t->atual);

					wmove(win->fire1, t->atual->posy, t->atual->posx-2);
					waddstr(win->fire1, EXPLOSAO_BANANA);	
					t->qtd_tiros1--;
					t->tamanho--;
				}
				else if ( t->atual->posy > 0 )
					t->atual->posy--;
				else{
					removeAtualLista(t->atual);
					t->qtd_tiros1--;
					t->tamanho--;
				}
			}
			else/*if ( t->atual->chave == 2 )*/	/*MOVIMENTA TIRO DAS NAVES*/
			{
				wmove(win->fire2, t->atual->posy, t->atual->posx);
				waddch(win->fire2, ' ');
				
				if ( nodoDetectado )
				{
					if (nodoDetectado->chave == -2) /*se o nodoDetectado for uma barreira*/
					{
						mat[t->atual->posy][t->atual->posx] = NULL;
						removeAtualLista(nodoDetectado);
						removeAtualLista(t->atual);
						
						wmove(win->fire2, t->atual->posy, t->atual->posx);
						waddstr(win->fire2, EXPLOSAO_COCO);	
						w->tamanho--;
						t->tamanho--;
					}
					else if (nodoDetectado->chave == -1) /*se o nodoDetectado for o tanque*/
					{
						wmove(win->fire2, t->atual->posy, t->atual->posx-2);
						waddstr(win->fire2, EXPLOSAO_COCO);	
						wrefresh(win->fire2);
						sleep(2);
						GameOver(l, win);
					}
				}
				
				if ( t->atual->posy < ROW-2 )
					t->atual->posy++;
				else{
					removeAtualLista(t->atual);
					t->tamanho--;
				}
			}
			t->atual = t->atual->prox;
		}
	}
}

void printTiros(t_tiro *t, t_win *win)
{
	t->atual = t->ini->prox;
	if ( (t->tamanho) )
	{
		while (t->atual != t->fim)
		{
			if ( t->atual->chave == 1 ){
				wmove(win->fire1, t->atual->posy, t->atual->posx);
				waddch(win->fire1, t->atual->sprite1[0]);
			}
			else{
				wmove(win->fire2, t->atual->posy, t->atual->posx);
				waddch(win->fire2, t->atual->sprite1[0]);
			}

			t->atual = t->atual->prox;	
		}
	}
}

void movimentaTanque(t_lista *l, t_tiro *t, WINDOW *win, int key, void *mat[ROW][COL])
{
	int i, j;

	switch (key)
	{
		case ' ': /* BOTÃO DE TIRO */
			if (t->qtd_tiros1 < 3)
				inicializaTiros(l, t, 1);
			break;
		case KEY_LEFT: /* BOTÃO DE ANDAR PARA ESQUERDA */
			if ( l->tanque->posx - 1 > 2 )
			{
				limpaNodoMatriz(mat, l->tanque);
				for (i=-1; i<=1; i++)
					for (j=-2; j<=3; j++){
					wmove(win, l->tanque->posy+i, l->tanque->posx+j);
					waddch(win, ' ');
				}
				l->tanque->posx--;
			}
			break;
		case KEY_RIGHT: /* BOTÃO DE ANDAR PARA DIREITA */
			if ( l->tanque->posx + 1 < COL-3 )
			{
				limpaNodoMatriz(mat, l->tanque);
				for (i=-1; i<=1; i++)
					for (j=-2; j<=3; j++){
					wmove(win, l->tanque->posy+i, l->tanque->posx+j);
					waddch(win, ' ');
				}
				l->tanque->posx++;
			}
			break;	
		case 'q': /* BOTÃO DE EXIT */
			endwin();
			exit(1);
		case 'p': /* BOTÃO DE PAUSE */
                	flushinp(); /*limpa buffer de input*/
  			nodelay(stdscr, FALSE);
			while ( (key=getch()) != 'p')
			{
				if ( key == 'q' ){ /* PARA PODER SAIR DO JOGO DURANTE O PAUSE */
					endwin();
					exit(1);
				}
				usleep(10000);
			}
  			nodelay(stdscr, TRUE);
			break;
		default:
			break;
	}
	atualizaMatriz(mat, l->tanque);
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

void restartGame(int *level, t_lista *l, t_tiro *t, t_wall *w, t_win *win, void *mat[ROW][COL])
{
	int i, j;
	
	*level+=10000;
	l->updateField = 0;
	l->direcao = RIGHT;
	l->speed = 500000 - *level;
	
	destroiListaTiros(t);
	destroiListaBarreira(w);
	for ( i=0; i<ROW; i++)
		for ( j=0; j<COL; j++ )
			mat[i][j] = NULL;
	
	inicializaListaBarreiras(w);
	inicializaListaTiros(t);
  	
	inicializaBarreiras(w, mat); 
  	inicializaNaves(l);

	clear();
	mvprintw((win->gety)/2, (win->getx)/2, "LEVEL %d", (*level/10000)+1);
	refresh();
	sleep(2);
	
  	borda((win->gety-ROW)/2,(win->getx-COL)/2 -1,((win->gety-ROW)/2)+ROW,((win->getx-COL)/2)+COL); /*inicializa borda*/
	wattron(win->enemy, COLOR_PAIR(rand()%4 +1) | A_BOLD);
	
	flushinp();
}

int GameOn(int *level, t_lista *l, t_tiro *t, t_wall *w, t_win *win, void *mat[ROW][COL])
{
	int key;
	int utime = 10000;
	int rand_range;
	int i, j, SWITCH;
	
	if ( l->updateField % 60000 == 0 )
	{	
		/*para ajudar a debugar
		mvprintw(0,0,"NAVES: %d ",l->tamanho);
		mvprintw(1,0,"TIROS USER: %d     ",t->qtd_tiros1);
		mvprintw(2,0,"TIROS: %d     ",t->tamanho);
		mvprintw(3,0,"BARREIRA: %d",w->tamanho);
		refresh();
		*/
		printBarreiras(win->enemy, mat);
		printNaves(l, win);
		printTanque(l, win->tank);
		printTiros(t, win);

		i=0; j=l->score+1;
		while (j>0){
			j/=10;
			i++;
		}
		mvwprintw(win->score, 0, COL/2, "000000",l->score);	
		mvwprintw(win->score, 0, (COL/2)+(6-i), "%d",l->score);	
		
		wrefresh(win->enemy);
		wrefresh(win->moship);
		wrefresh(win->fire1);
		wrefresh(win->fire2);
		wrefresh(win->tank);
		wrefresh(win->score);
	}

	if ( l->updateField % 40000 == 0 )
	{
		if (( l->moship->chave == FALSE ) && ( rand()%500 == 0 ))
			l->moship->chave = TRUE;
		else if (l->moship->chave == TRUE){
			wclear(win->moship);
			movimentaMoShip(l, win->moship, mat);
		}

		if ( rand()%50 == 0 )
		{
			rand_range = rand() % l->tamanho;
			
			j=0;
			SWITCH = FALSE;
			for ( i=0; i<5; i++ )
			{
				if (SWITCH == TRUE)
					break;
				
				l->atual = l->ini[i]->prox;
				while (l->atual != l->fim[i])
				{
					if ( j == rand_range ){
						SWITCH = TRUE;
						break;
					}
					l->atual = l->atual->prox;
					j++;
				}
			}
			inicializaTiros(l, t, 2);
		}	
		
		movimentaTiros(t, l, w, win, mat);
	}
	if ( l->updateField % (l->speed+50000) == 0 ){
		wclear(win->enemy);
		movimentaNaves(l, win, mat);
		l->updateField = 0;
	}
	
	usleep(utime);
	l->updateField += utime;

	if ( l->tamanho == 0 )
		restartGame(level, l, t, w, win, mat);
	
	key = getch();
	movimentaTanque(l, t, win->tank, key, mat);
	
	return 1;
}

int main()
{
/* GAME properties */
  int i, j;
  int size_y, size_x; /*para posicionar tela no centro*/
  /* matriz de ponteiros que guarda a posição dos objetos do campo 
   * em suas coordenadas em forma do endereço do mesmo, para facilitar
   * detecção de colisão */
  void *mat[ROW][COL];
  int level;
  
  t_lista l;
  t_tiro t;
  t_wall w;
  t_win win;
/* START ncurse */
  initscr();
  cbreak();
  noecho();
  curs_set(FALSE);
  nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE);
  start_color();
  
  titleScreen(&win);
  
  srand(time(NULL)); /*cria seed para rand()*/
  size_y = (win.gety-ROW)/2;
  size_x = (win.getx-COL)/2;
  
  for ( i=0; i<ROW; i++ )
      for ( j=0; j<COL; j++ )
          mat[i][j] = NULL; /*preenche elementos da matriz de ponteiros com NULL*/
  
  level = 0;
  
  l.updateField = 0; /*temporizador de controle começa no 0*/
  l.direcao = RIGHT; /*naves começam se movendo para a direita*/
  l.score = 0; /*inicializa score no zero*/
  l.speed = 500000;

  /*INICIALIZA JANELAS A SEREM UTILIZADAS*/
  win.moship = newwin(5,COL,size_y+1,size_x);
  win.enemy = newwin(ROW-3,COL,size_y+1,size_x);
  win.tank = newwin(ROW-1,COL,size_y+1,size_x);
  win.fire1 = newwin(ROW-1,COL,size_y+1,size_x);
  win.fire2 = newwin(ROW-1,COL,size_y+1,size_x);
  win.score = newwin(ROW-1,COL,size_y+1,size_x);
  
  
  /*INICIALIZA PARES DE COR E ATRIBUTOS PARA CADA JANELA*/
  init_pair(COR_TANQUE, COLOR_PINK, COLOR_BLACK);
  wattron(win.tank, COLOR_PAIR(COR_TANQUE) | A_BOLD);
  
  init_pair(COR_MOSHIP, COLOR_BLUE, COLOR_BLACK);
  wattron(win.moship, COLOR_PAIR(COR_MOSHIP) | A_BOLD);
  
  init_pair(COR_TIRO_TANQUE, COLOR_YELLOW2, COLOR_BLACK);
  wattron(win.fire1, COLOR_PAIR(COR_TIRO_TANQUE) | A_BOLD);
  
  init_pair(COR_TIRO_NAVE, COLOR_WHITE, COLOR_BLACK);
  wattron(win.fire2, COLOR_PAIR(COR_TIRO_NAVE) | A_BOLD);

  init_pair(COR_SCORE, COLOR_YELLOW2, COLOR_BLACK);
  wattron(win.score, COLOR_PAIR(COR_SCORE) | A_BOLD);
  
  init_pair(COR_NAVE, COLOR_ORANGE, COLOR_BLACK);
  wattron(win.enemy, COLOR_PAIR(COR_NAVE) | A_BOLD);
  
  init_pair(COR_BANANA, COLOR_YELLOW2, COLOR_GREEN);
  init_pair(COR_MACACO, 130, COLOR_BLACK);
  init_pair(COR_MADEIRA, 130, COLOR_BROWN);
  init_pair(COR_BARREIRA, COLOR_GREEN, COLOR_GREEN);
  

  /*INICIALIZA LISTA
   * AS OUTRAS LISTAS SÃO INICIADAS E PREENCHIDAS
   * NA FUNÇÃO restartGame()*/
  inicializaListaNaves(&l);
  inicializaListaTiros(&t);
  inicializaListaBarreiras(&w);
  	
  inicializaNaves(&l);
  inicializaBarreiras(&w, mat); 
  
  mvprintw((win.gety)/2, (win.getx)/2, "LEVEL 1");
  refresh();
  borda(size_y,size_x-1,size_y+ROW,size_x+COL); /*inicializa borda*/
  sleep(2);
  /*função principal que controla o tempo e chama todas as outras funções pro funcionamento do jogo*/
  while ( GameOn(&level, &l, &t, &w, &win, mat) );
  sleep(2);
  
  endwin();
  return 0;
}
