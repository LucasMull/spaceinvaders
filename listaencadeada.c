/*	COISAS A FAZER:
 *		FAZER EXPLOSÃO DURAR MAIS TEMPO NA TELA
 *		IMPLEMENTAR DETECÇÃO DE COLISÃO AMPLAMENTE
 *		ARRUMAR BUG DE TIROS NA MESMA LINHA APAGAREM LINHA
 *		TERMINAR DE COMENTAR O CÓDIGO
 *		CRIAR INSTRUÇÃO DE CONTROLES ANTES DO JOGO INICIAR
 *		CRIAR VARIÁVEL CHAVE NO NODO PARA DEFINIR VALOR DE PONTUAÇÃO DA NAVE OU COMO LEITURA DO ID
 */


#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define ROW 38
#define COL 100

#define RIGHT 'r'
#define LEFT 'l'

#define TRUE 1
#define FALSE 0

#define SHIP11 "\n TvT \n /U\\ \n  w w\0"
#define SHIP12 "\n TvT \n (U) \nw w  \0"
#define SHIP21 "\n0'-'0\nd(.)b\n_/ )_\0"
#define SHIP22 "\n0'o'0\n(b.d)\n_( \\_\0"
#define SHIP31 "\n{ovo}\n(^^^)\n W W \0"
#define SHIP32 "\n{OvO}\n/^^^\\\n W W \0"

#define TANK1 "\n , ,m\nm-^-.\0"
#define TANK2 "\nm, m,\n.-^-.\0"

#define MOSHIP1 "\n|\"|'-'L_ \n|SAFARI_|\nJ(x)\"\"(x)\0"
#define MOSHIP2 "\n|\"|'O'L_ \n|SAFARI_|\nJ(+)\"\"(+)\0"

#define TIRO1 "(\0"
#define TIRO2 "#\0"

#define MORTE "\n \\'/ \n-   -\n /,\\ \0"

#define COR_BORDA 0
#define COR_NAVE 1
#define COR_TANQUE 2
#define COR_TIRO_NAVE 3
#define COR_TIRO_TANQUE 4
#define COR_MOSHIP 5
#define COR_EXPLOSAO 6

#define COLOR_BROWN 52
#define COLOR_ORANGE 154
#define COLOR_PINK 229
#define COLOR_YELLOW2 220

struct t_node
{
 int chave; /*para fins de identificação*/

 char *sprite1; /*aparência do elemento do nodo*/
 char *sprite2; /*<- ignorado para elementos com só 1 sprite*/
 
 int posx; /*posição x*/
 int posy; /*posição y*/
 
 struct  t_node *prox; /*próximo nodo*/
 struct t_node *prev; /*nodo anterior*/
}; typedef struct t_node t_node;

struct t_tiro
{
 t_node *atual;

 t_node *ini;
 t_node *fim;

 int qtd_tiros1;
 int qtd_tiros2;
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
 int qtd_naves;
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
}; typedef struct t_win t_win;



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
 first[0]->prev = tanque_node;
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
		last[i]->prox = moship_node;
	last[i]->prev = first[i];
 }
 l->qtd_naves = 0;

 return 1;
}

int inicializaListaTiros (t_tiro *t)
{
 t_node *first;
 t_node *last;

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
 t->qtd_tiros2 = 0;

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
 l->moship->posx = COL/2;
 l->moship->posy = 1;
 l->moship->chave = 100;

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
		l->qtd_naves++; /*qtd de naves total*/
	}
	posCol += 4; /*pula fileira de 4 em 4*/
 }

 return 1;
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
			col=-2;
		}
		else
		{
			if ( l->tanque->posx % 2 == 0 ){
				wmove(win, l->tanque->posy+row, l->tanque->posx+col); /*atualiza posição para printar o char*/
				waddch(win, l->tanque->sprite1[j]); /*printa o char na posição*/
			}
			else{
				wmove(win, l->tanque->posy+row, l->tanque->posx+col); /*atualiza posição para printar o char*/
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

	/* TEMPORARIO APENAS PARA VISUALIZACAO DA MOSHIP 
	 * CRIAR FÇ DEPOIS */
	j=0;
	row=0; 
	while ( l->moship->sprite1[j] != '\0' )
	{
		if ( l->moship->sprite1[j] == '\n' ){
			row++;
			col=-4;
		}
		else
		{	
			if ( l->moship->posx % 3 == 0 ){
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
		  row=0;
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

void desceNaves(t_lista *l)
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
			endwin();
			exit(1);
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
			fim_i = 3;
			ini_j = -3;
			fim_j = 3;
			break;
		case 100:
			fim_i = 4;
			ini_j = -5;
			fim_j= 5;
			break;
		default:
			fim_i = 4;
			ini_j = -3;
			fim_j = 3;
			break;
	}		
	 
	for ( i=-1;i<fim_i;i++ )
		for ( j=ini_j;j<=fim_j;j++ )
				mat[atual->posy+i][atual->posx+j] = NULL;
}

void atualizaMatriz(void *mat[ROW][COL], t_node *atual)
{
	 int row, col;
	 int j, ini_j;
	 
	 j=0;
	 row=0;
	  
	 if ( atual->chave == 100 )
		 ini_j = -4;
	 else
		 ini_j = -2;

         while ( atual->sprite1[j] != '\0' )
         {
         	if( atual->sprite1[j] == '\n' ){
                	row++;
                        col=ini_j;
                }
		mat[atual->posy+row][atual->posx+col] = atual;
                
		col++;
                j++;
	 }
}

void movimentaMoShip(t_lista *l, WINDOW *win, void *mat[ROW][COL])
{
	limpaNodoMatriz(mat, l->moship);
	if ( l->moship->posx+6 < COL )
		l->moship->posx++;
	else
		l->moship->posx = 5;
	atualizaMatriz(mat, l->moship);
}

void movimentaNaves(t_lista *l, WINDOW *win, void *mat[ROW][COL])
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
					desceNaves(l);
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
					desceNaves(l);
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
		new_fire->posx = l->tanque->posx+1;
		new_fire->posy = ROW-4;
		
		new_fire->sprite1 = TIRO1;
		t->qtd_tiros1++;
	}
	else/*if ( ID == 2 )*/
	{
		new_fire->posx = l->atual->posx+1;
		new_fire->posy = l->atual->posy+3;
		
		new_fire->sprite1 = TIRO2;
		t->qtd_tiros2++;
	}
	new_fire->sprite2 = MORTE;
	return 1;
}

void removeAtualLista(t_node *atual)
{
	if (atual->prox != NULL){
		atual->prev->prox = atual->prox;
		atual->prox->prev = atual->prev;
		free(atual);
		atual = NULL;
	}
}

void printaExplosao(t_node *atual, t_win *win)
{
	int row, col, j;
	
	j=0;
	row=-3; 
	while ( atual->sprite2[j] != '\0' )
	{
			if( atual->sprite2[j] == '\n' ){
				row++;
				col=-2;
			}
			else{
				wmove(win->score, atual->posy+row, atual->posx+col);
				waddch(win->score, atual->sprite2[j]);
			}
			col++;
			j++;
	}
}

/*FUNÇÃO RESPONSÁVEL POR MOVIMENTAR O TIRO DO USUÁRIO
 * E DOS INIMIGOS, A PARTIR DE UMA LISTA LINKADA
 * E CHECAR AS CONDIÇÕES DE COLISÃO ENTRE TIRO E OUTROS ELEMENTOS */
void movimentaTiros(t_tiro *t, t_lista *l, t_win *win, void *mat[ROW][COL])
{
	t_node *nodoDetectado;

	t->atual = t->ini->prox;
	if ((t->qtd_tiros1) || (t->qtd_tiros2))
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
					l->score += nodoDetectado->chave;
					if (nodoDetectado->chave != 100)	
						l->qtd_naves--;
					
					removeAtualLista(nodoDetectado);
					limpaNodoMatriz(mat, nodoDetectado);
					removeAtualLista(t->atual);

					printaExplosao(t->atual, win);	
					t->qtd_tiros1--;
				}
				else if ( t->atual->posy > 0 )
					t->atual->posy--;
				else{
					removeAtualLista(t->atual);
					t->qtd_tiros1--;
				}
			}
			else/*if ( t->atual->chave == 2 )*/
			{
				wmove(win->fire2, t->atual->posy, t->atual->posx);
				waddch(win->fire2, ' ');
				
				if ( nodoDetectado && t->atual->posy >= ROW-2 )
				{
					printaExplosao(t->atual, win);
					wrefresh(win->score);
					sleep(2);
					endwin();
					exit(1);	
				}
				if ( t->atual->posy < ROW-2 )
					t->atual->posy++;
				else{
					removeAtualLista(t->atual);
					t->qtd_tiros2--;
				}
			}
			t->atual = t->atual->prox;
		}
	}
}

void printTiros(t_tiro *t, t_win *win)
{
	t->atual = t->ini->prox;
	if ((t->qtd_tiros1) || (t->qtd_tiros2))
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

	limpaNodoMatriz(mat, l->tanque);
	switch (key)
	{
		case ' ': /* BOTÃO DE TIRO */
			if (t->qtd_tiros1 < 3)
				inicializaTiros(l, t, 1);
			break;
		case 'a': /* BOTÃO DE ANDAR PARA ESQUERDA */
			if ( l->tanque->posx - 1 > 2 )
			{
				for (i=-1; i<=1; i++)
					for (j=-2; j<=3; j++){
					wmove(win, l->tanque->posy+i, l->tanque->posx+j);
					waddch(win, ' ');
				}
				l->tanque->posx--;
			}
			break;
		case 'd': /* BOTÃO DE ANDAR PARA DIREITA */
			if ( l->tanque->posx + 1 < COL-5 )
			{
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
			while ( (key=getch()) != 'p')
			{
				if ( key == 'q' ){ /* PARA PODER SAIR DO JOGO DURANTE O PAUSE */
					endwin();
					exit(1);
				}
				usleep(10000);
			}
			break;
		default:
			break;
	}
	atualizaMatriz(mat, l->tanque);
}

int GameOn(t_lista *l, t_tiro *t, t_win *win, void *mat[ROW][COL])
{
	int key;
	int utime = 10000;
	int rand_range;
	int i, j, SWITCH;
	
	if ( l->updateField % 60000 == 0 )
	{	
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

	key = getch();
	movimentaTanque(l, t, win->tank, key, mat);
	if ( l->updateField % 40000 == 0 )
	{
		if ( rand()%50 == 0 )
		{
			rand_range = rand() % l->qtd_naves;
			
			j=0;
			SWITCH = FALSE;
			for ( i=0; i<5; i++ )
			{
				if (SWITCH == TRUE)
					break;
				
				l->atual = l->ini[i]->prox;
				while (l->atual != l->fim[i]){
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
		movimentaTiros(t, l, win, mat);
	}
	if ( l->updateField % 70000 == 0 ){
		wclear(win->moship);
		movimentaMoShip(l, win->moship, mat);
	}
	if ( l->updateField % l->speed == 0 ){
		wclear(win->enemy);
		movimentaNaves(l, win->enemy, mat);
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
/* WINDOW properties */
  int gety, getx;
  int key;
/* GAME properties */
  int i, j;
  /* matriz de ponteiros que guarda a posição dos objetos do campo 
   * em suas coordenadas em forma do endereço do mesmo, para facilitar
   * detecção de colisão */
  void *mat[ROW][COL];
  
  t_lista l;
  t_tiro t;
  t_win win;
/* START ncurse */
  initscr();
  cbreak();
  noecho();
  curs_set(FALSE);
  nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE);
  start_color();

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
  
  srand(time(NULL)); /*cria seed para rand()*/
  
  for ( i=0; i<ROW; i++ )
      for ( j=0; j<COL; j++ )
          mat[i][j] = NULL; /*preenche elementos da matriz de ponteiros com NULL*/

  l.updateField = 0; /*temporizador de controle começa no 0*/
  l.direcao = RIGHT; /*naves começam se movendo para a direita*/
  l.score = 0; /*inicializa score no zero*/
  l.speed = 300000;

  /*INICIALIZA JANELAS A SEREM UTILIZADAS*/
  win.moship = newwin(5,COL,1,1);
  win.enemy = newwin(ROW-3,COL,1,1);
  win.tank = newwin(ROW,COL,1,1);
  win.fire1 = newwin(ROW,COL,1,1);
  win.fire2 = newwin(ROW,COL,1,1);
  win.score = newwin(ROW,COL,1,1);
  
  borda(0, 0, ROW, COL+1); /*inicializa borda*/
  
  /*INICIALIZA PARES DE COR E ATRIBUTOS PARA CADA JANELA*/
  init_pair(COR_NAVE, COLOR_ORANGE, COLOR_BLACK);
  wattron(win.enemy, COLOR_PAIR(COR_NAVE));
  wattron(win.enemy, A_BOLD);
  
  init_pair(COR_TANQUE, COLOR_PINK, COLOR_BLACK);
  wattron(win.tank, COLOR_PAIR(COR_TANQUE));
  wattron(win.tank, A_BOLD);
 
  init_pair(COR_MOSHIP, COLOR_BLUE, COLOR_BLACK);
  wattron(win.moship, COLOR_PAIR(COR_MOSHIP));
  wattron(win.moship, A_BOLD);
  
  init_pair(COR_TIRO_TANQUE, COLOR_YELLOW, COLOR_BLACK);
  wattron(win.fire1, COLOR_PAIR(COR_TIRO_TANQUE));
  wattron(win.fire1, A_BOLD);
  
  init_pair(COR_TIRO_NAVE, COLOR_BROWN, COLOR_BLACK);
  wattron(win.fire2, COLOR_PAIR(COR_TIRO_NAVE));
  wattron(win.fire2, A_BOLD);

  init_pair(COR_EXPLOSAO, COLOR_YELLOW2, COLOR_BLACK);
  wattron(win.score, COLOR_PAIR(COR_EXPLOSAO));
  wattron(win.score, A_BOLD);

  /*INICIALIZA LinkedLists A SEREM UTILIZADAS*/
  inicializaListaNaves(&l);
  inicializaListaTiros(&t);

  inicializaNaves(&l); /*cria e insere os nodos das naves na lista*/
  
  /*função principal que controla o tempo e chama todas as outras funções pro funcionamento do jogo*/
  while ( GameOn(&l, &t, &win, mat) );
  sleep(2);
  
  endwin();
  return 0;
}
