/**
 * 
 * ARRUMAR ANIMAÇÃO DE MOVIMENTO DAS NAVES INIMIGAS
 * ARRUMAR TIRO QUE BATE DO LADO DA NAVE E SOME SEM MATAR
 * ARRUMAR TEMPORIZAÇÃO DA VELOCIDADE DAS NAVES
 * REINICIAR JOGO POR INPUT DO USUÁRIO OU AO EXTERMINAR TODAS AS NAVES
 * CRIAR LAÇO EM initEnemy()
 *
**/

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <string.h>

#define LINE 38
#define COLUMN 100

#define TRUE 1
#define FALSE 0

#define RIGHT 0
#define LEFT 1

/*SETA CAMPO E TEMPORIZAÇÃO*/
typedef struct init_gamescr
{
  char field[LINE][COLUMN]; /*matriz campo*/
  unsigned int time_elapsed; /*para fins de temporização de movimento*/
  unsigned int fire_elapsed; /*temporização do tiro*/
  int shiprefresh; /*tempo necessário para atualizar movimento das naves*/
  int enemycount; /*calcular placar e velocidade de movimento baseado na qtd de naves inimigas no campo*/
  int fireCount; /*quando atirar 3x num intervalo menor q 1seg entrar no cooldown*/
  int cooldown; /*entra no cooldown*/
} gameset;

/*MOVIMENTO DAS NAVES INIMIGAS E DO TANQUE*/
typedef struct enemy_rotation
{
  char direction[5]; /*direction 1 = right; 0 = left*/
  char activate;
  int rotate[5]; /*atualiza posição da row de naves conforme elas se movem*/
} movement;

/* MOVIMENTA E POSICIONA O TANQUE NA TELA */
typedef struct tank_movement
{
  int posx;
  int posy;
} user;

/*CORPO DAS NAVES, PREENCHIDO NO MAIN()*/
typedef struct enemy_body_parts
{
  char top[3][6], mid[3][6], bott[3][6];

  int randx, randy; /* para gerar coordenadas random para tiro inimigo */
} enemy;

/*FUNÇÕES DE CRIAÇÃO*/
void initField(gameset *update, enemy *ship); /* monta a matriz campo */
void initEnemy(int i, int j, enemy *ship, gameset *update); /* preenche matriz campo com naves inimigas */
void initTank(int i, int j, gameset *update); /* preenche matriz com o tanque */

/*FUNÇÕES DE ATUALIZAÇÃO*/
void printScreen(gameset *update, WINDOW *win); /* imprime campo na tela do usuário */
void GameOver(); /* TELA DE FIM DE JOGO */

/*FUNÇÕES DE JOGADA*/
int updateField(gameset *update, movement *path, WINDOW *win, enemy *fire, user *tank); /*atualiza o campo com movimentos etc*/
void rotateEnemies(gameset *update, int *i, movement *pathRow); /*movimentação das naves inimigas*/
void rowMove (gameset *update, movement *pathRow);
void enemyFire(gameset *update, enemy *fire); /*aleatoriamente coloca o tiro do inimigo na tela*/
void moveFire(gameset *update); /*move tiro amigo e inimigo*/
void moveTank(int key, gameset *update, user *tank); /*move tanque do usuário*/

int main()
{
/* START ncurse */
  initscr();
  cbreak();
  noecho();
  curs_set(FALSE);
  nodelay(stdscr, TRUE);
/* WINDOW properties */
  int key;
  int starty, startx;
  int gety, getx;
  int i, j;
/* GAME configuration */
  gameset update;
  	  update.enemycount=55; /*contagem começa em 55 inimigos e decrementa a cada nave eliminada*/
	  update.time_elapsed= 0; /*soma com cada usleep para controlar temporização*/
	  update.fire_elapsed= 600000; /*tempo de atualização de movimento do tiro*/
	  update.shiprefresh= 500000; /*tempo de atualização de movimento das naves*/
	  update.fireCount = 0; /*quando tiver 3 tiros consecutivos entra num cooldown antes de poder atirar novamente*/
	  update.cooldown = 1; 
  enemy ship; /*APARÊNCIA DAS NAVES*/
  	   strcpy(ship.top[0],  "(uwu)");   strcpy(ship.top[1], "('-')");   strcpy(ship.top[2], "(*3*)"); 
	   strcpy(ship.mid[0], "\\OMO/");   strcpy(ship.mid[1], "d-M-b");   strcpy(ship.mid[2], "d-M-b"); 
	  strcpy(ship.bott[0],  " / \\ "); strcpy(ship.bott[1], "_( )_"); strcpy(ship.bott[2],  "_/-\\_");
  enemy fire; /*COORDENADAS ALEATÓRIAS DO TIRO INIMIGO A SER GERADA*/
	  fire.randx = 0;
	  fire.randy = 0;
  movement path; /*armazena a posição X da fileira de naves*/
  	  for (i=0, j=24; i<5; path.rotate[i]=j, path.direction[i++]=LEFT, j-=4);
	  path.activate=FALSE; /*se == TRUE então as fileiras descem uma posição, reseta pra FALSE depois de descer*/
  user tank; /*posição do tanque*/
          tank.posx=36;
	  tank.posy=COLUMN/2;


  getmaxyx(stdscr,gety, getx);
  while (key != 'a')
  {
  	if ( getx<COLUMN || gety<LINE )
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

  starty = startx = 0;
  WINDOW * win = newwin(LINE, COLUMN, starty, startx);
  refresh();
  keypad(win, TRUE);
  initField(&update, &ship); /*inicia matriz campo e preenche com naves e tanque*/
  while( ! updateField(&update, &path, win, &fire, &tank) ); /*roda o jogo até que o usuário aperte 'q'*/
  
  return 0;
}

/*POSICIONA NAVES INIMIGAS NO FIELD*/
void initEnemy(int i, int j, enemy *ship, gameset *update)
{

  int ki, kj, kl;
  int TYPE = 0, activate;

  switch(i) /* i = linha */
  {
	case 8:
		TYPE = 0;
		activate = TRUE;
		break;
	case 12:
	case 16:
		TYPE = 1;
		activate = TRUE;
		break;
	case 20:
	case 24:
		TYPE = 2;
		activate = TRUE;
		break;
	default:
		activate = FALSE;
  		break;
  }

  if ( activate == TRUE )
	for ( kl = 0, kj = j-2 ; kl < 5; kl++, kj++ )
	{
		update->field[i-1][kj]= ship->top[TYPE][kl];	
		update->field[i][kj]= ship->mid[TYPE][kl];	
		update->field[i+1][kj]= ship->bott[TYPE][kl];
	}
}

/*POSICIONA TANQUE NO FIELD*/
void initTank(int i, int j, gameset *update)
{
	int ki, kj;

	for(ki=i; ki<=i+1; ki++)  /*limpa tanque atual antes de prox tanque*/
		for (kj=0; kj<COLUMN; kj++)
			update->field[ki][kj] = ' ';
   /*GERA CHARS DO TANQUE*/
	update->field[i][j-1]= '/';
	update->field[i][j]= '^';
	update->field[i][j+1]= '\\';
	update->field[i+1][j-2]= update->field[i+1][j-1]= update->field[i+1][j]= update->field[i+1][j+1]= update->field[i+1][j+2]= 'M';
}

/*POSICIONA TIRO INIMIGO ALEATORIAMENTE*/
void enemyFire(gameset *update, enemy *fire)
{
	int i, j;
   /*GERA COORDENADA ALEATÓRIA PARA O TIRO*/
	fire->randx = ( rand() % COLUMN-1 );
	fire->randy = ( rand() % LINE-1 );
   /*POSICIONA O TIRO EMBAIXO DA NAVE*/
	for ( i = fire->randy; i < LINE; i++ )
		for ( j = fire->randx; j < COLUMN-4; j++ )
			if ( update->field[i][j] == 'M' && update->field[i+5][j] == ' ')
			{
				update->field[i+2][j] = '$';
				break;
			}
}

/* DESCE AS NAVES UMA FILEIRA */
void rowMove (gameset *update, movement *pathRow)
{
	int i,j;

	for ( i = 0; i < 5; i++ )
	{
		for (j=0; j<COLUMN; j++)
		{
			if ( update->field[(pathRow->rotate[i])][j] != ' ' ) /*desce as naves uma fileira*/
			{
				update->field[(pathRow->rotate[i])+2][j]=update->field[(pathRow->rotate[i])+1][j];
				update->field[(pathRow->rotate[i])+1][j]=update->field[(pathRow->rotate[i])][j];
				update->field[(pathRow->rotate[i])][j]=update->field[(pathRow->rotate[i])-1][j];
				update->field[(pathRow->rotate[i])-1][j]= ' ';
			}
			else if ( pathRow->rotate[i] == LINE-4 )
			  GameOver();
		}
		
		if ( pathRow->rotate[i] != FALSE )
			( pathRow->rotate[i] ) += 1;
	}
}	

/* MOVIMENTAÇÃO DAS NAVES INIMIGAS */
void rotateEnemies(gameset *update, int *i, movement *pathRow) /*movimentação das naves inimigas*/
{
	int fileira_nao_nula = FALSE; /*se o valor mudar pra TRUE então há nave na linha*/
	int j, ki, kj; /*contadores*/

	if ( pathRow->direction[0] == 1) /* segue à esquerda */
		if ( (update->field[*i][COLUMN-1] == ' ') ) /* se houver um espaço em branco em frente à nave */
		{
			for (j=COLUMN-1; j>=0; j--)
				if (update->field[*i][j] == 'M') /* se encontrar uma nave na fileira linhanula = 1 */
				{
				     	fileira_nao_nula=TRUE;
					if (update->field[*i+1][j+3] == 'H' || update->field[*i+1][j+3] == 'A') /* come barreira na diagonal */
						 update->field[*i+1][j+3] = ' ';
				     	for (ki= *i-1; ki <= *i+1; ki++) /* movimentação da nave */
				       		for (kj= j+3; kj >= j-2; kj--)
				       		{
					 		if ( update->field[ki][kj-1] == '|' || update->field[ki][kj-1] == '$' || update->field[ki][kj-1] == 'H' || update->field[ki][kj-1] == 'A' )
						 		update->field[ki][kj-1] = ' '; /* come tiros e barreira à frente da nave */
				         		update->field[ki][kj]=update->field[ki][kj-1]; /* anda */
				       		}
				}
			(*i) *= fileira_nao_nula; /* se não houver mais naves na fileira então *i recebe valor nulo */
		}
		else /* inverte direção e desce as fileiras */
		{
		 	pathRow->direction[0] = RIGHT;  /*inverte para direita*/
			pathRow->activate = TRUE; /* permite descer as fileiras */
		}
	else /*segue à direita*/
		if (update->field[*i][0] == ' ')
		{
			for (j=0; j<COLUMN; j++) 
				  if ( update->field[*i][j] == 'M' )
				  {
					if (update->field[*i+1][j-3] == 'H' || update->field[*i+1][j-3] == 'A') /*come barreira*/
						 update->field[*i+1][j-3] = ' ';
			         	fileira_nao_nula=TRUE;
				     	for (ki= *i-1; ki <= *i+1; ki++)
				       		for (kj= j-3; kj <= j+2; kj++)
				       		{
					 		if ( update->field[ki][kj+1] == '|' || update->field[ki][kj+1] == '$' || update->field[ki][kj+1] == 'H' || update->field[ki][kj+1] == 'A' )
						 		update->field[ki][kj+1] = ' ';
				         		update->field[ki][kj]=update->field[ki][kj+1];
				       		}
				  }
			(*i) *= fileira_nao_nula;
		}
		else
		{
			pathRow->direction[0] = LEFT; /*inverte para esquerda*/
			pathRow->activate = TRUE; /*ativa para descer fileiras*/
		}
}

/*INICIA MATRIZ CAMPO*/
void initField(gameset *update, enemy *ship)
{
  int i, j;
  int kj;

  for (i=0; i<LINE; i++)/*preencher matriz campo com espaço*/
    for (j=0; j<COLUMN; j++)
      update->field[i][j]=' ';
  
  for (i=8; i<=24; i+=4)/*preencher matriz campo com naves*/
    for (j=0; j<COLUMN; j++)
      if ( ( (j+2)%9 == 0 ) && i<40)/*pula de 7 em 7 casas por nave*/
          initEnemy(i, j, ship, update);
      
  initTank(36, COLUMN/2, update);  /*cria tanque no row 36*/

  for (j=13, i=33; j<COLUMN; j+=25) /*gera barreiras*/
  {
	for (kj=j-2; kj<=j+2; kj++)	
	  update->field[i][kj] = 'H';
	
	update->field[i-1][j-2] = update->field[i-1][j+2] = update->field[i][j-3] = update->field[i][j+3] = 'A';
	update->field[i-1][j-1] = update->field[i-1][j] = update->field[i-1][j+1] = 'H';
  	update->field[i+1][j-3] = update->field[i+1][j-2] = update->field[i+1][j+2] = update->field[i+1][j+3] = 'H';
  }	  
}

/*IMPRIME MATRIZ CAMPO NA TELA DO USUÁRIO*/
void printScreen(gameset *update, WINDOW *win)
{
	int i, j;
	
	for (i=0;i<LINE;i++)
		for (j=0;j<COLUMN;j++)
			mvprintw(i, j, "%c", update->field[i][j]);
	refresh();
}

/*MATRIZ PRINCIPAL, AVALIA CONDIÇÕES PARA RODAR O JOGO, CONTROLA TEMPORIZAÇÃO*/
int updateField(gameset *update, movement *path, WINDOW *win, enemy *fire, user *tank)/*atualiza o campo com movimentos etc*/
{
  int i; /*contador*/
  int key; /*recebe input do usuário*/
  unsigned int usecs = 5000; /*refresh rate do input*/
  
  path->activate = FALSE; /*enquanto for FALSE as fileiras da naves não descem*/
  printScreen(update, win); /*imprime matriz campo*/
  
  if ( update->fire_elapsed % 40000 == 0 ) /*cooldown do tiro do tanque (se move a cada 40k microssegundos)*/
  	moveFire(update); /*movimenta tiro na tela*/
  
  moveTank(key = getch(), update, tank); /*atualiza ação do tanque pelo input*/
  
  if ( update->time_elapsed % update->shiprefresh  == 0 ) /*refresh rate do movimento das naves que varia com o tempo e qtd de inimigos*/
  {
	for ( i=4; i>=0; i-- ) /*acessa todas as fileiras de naves*/
	{
		rotateEnemies(update, path->rotate+i, path); /*movimenta fileira i*/
		if (path->activate == TRUE) /*se fileira i tiver alcançado extremo, então desce todas as fileiras*/
		{
			rowMove(update, path);
			break;
		}
		
	}
	
	update->time_elapsed = 0; /* reseta tempo decorrido */
  }

  if ( update->fire_elapsed % 500000 == 0 ) /*refresh rate gerar tiro inimigo*/
	enemyFire(update, fire);

  update->time_elapsed += usecs; /*soma refresh rate do input ao tempo decorrido*/
  update->fire_elapsed += usecs; /*soma refresh rate do input ao cooldown do tiro do tanque*/
  usleep(usecs); /* pausa o programa por 5k microssegundos */
}

/* MOVIMENTA TIRO NA TELA */
void moveFire(gameset *update)
{
	int i, j;
	int ki, kj;
	int di, dj;
	
   /*FOGO DO USUÁRIO*/
	for( i=0; i<LINE; i++ )
		for( j=0; j<COLUMN; j++ )
			if ( update->field[i][j] == '|' ) /* se xy corresponder ao tiro do tanque */
			{
				update->field[i][j] = ' ';
				
				if (update->field[i-1][j] == ' ' ) /*atualiza movimento do tiro*/
					update->field[i-1][j] = '|';
				
				else if (i-1 >= 0)/*extermina naves e barreiras*/
				{
					    for ( ki=i-2; ki<=i; ki++) /*checa pra ver se o tiro atingiu uma nave, e então elimina ela*/
						    for ( kj= j-2; kj<=j+2; kj++)
					    		if ( update->field[ki][kj] == 'M' )
							{
				  			   for (di=ki-2; di<=ki+2; di++)
								   for (dj=kj-2;dj<=kj+2; dj++)
									update->field[di][dj] = ' ';
									
								  /*AUMENTA VELOCIDADE DO JOGO A CADA 5 INIMIGOS MORTOS*/
								  if( --(update->enemycount) >= 5 && update->enemycount % 5 == 0 )
									update->shiprefresh -= 10000;
								  if ( update->enemycount == 0 )
									  main();
							}
			          	  update->field[i-1][j] = ' ';
				}
			}

	for ( j=0; j<COLUMN; j++ ) /*apaga animação de tiro do tanque*/
	  if (update->field[LINE-3][j] == '>' || update->field[LINE-3][j] == '<')	
		update->field[LINE-3][j] = ' ';
   /*FOGO DO INIMIGO*/
	if (update->fire_elapsed % 80000 == 0 ) /*refresh rate do tiro inimigo*/
		for( i=LINE-1; i>=0; i-- )
			for( j=0; j<COLUMN; j++ )
				if ( update->field[i][j] == '$' ) /* se xy corresponder ao tiro do inimigo */
				{
					update->field[i][j] = ' ';
					if ( update->field[i+1][j] == ' ' ) /*atualiza movimento do tiro se houver espaço a sua frente*/
						update->field[i+1][j] = '$';
					else if ( update->field[i+1][j] == 'H' || update->field[i+1][j] == 'A' || update->field[i+1][j] == '|' )
						update->field[i+1][j] = ' '; /* some com barreira e choque entre tiros */
					else if ( i >= LINE-3 ) /* fim de jogo se o tiro atingir um objeto na fileira do tanque */
						GameOver();

					if ( i+1 == LINE-1 ) /*apaga tiro da tela quando chegar na borda*/
						update->field[i+1][j] = ' ';
				}
}

/* LEITURA DA AÇÃO DO USUÁRIO */
void moveTank(int key, gameset *update, user *tank)
{
	switch (key)
	{
		case ' ':
			/*contagem de tiro reinicia se fire_elapsed tiver atingido 1seg*/
			if (update->fire_elapsed > 1000000)
				update->fireCount = 0;

			if (update->fire_elapsed > 100000*update->cooldown) /*delay para o tiro do tanque*/
			{
				update->field[tank->posx-1][tank->posy] = '|'; /*posiciona tiro em frente ao tanque*/
				update->fire_elapsed = 0; /*refresh rate do tiro*/
				update->cooldown = 1; /*reseta cooldown*/
			     /*ANIMACAO DO TIRO*/
				update->field[tank->posx-1][tank->posy-1] = '>';
				update->field[tank->posx-1][tank->posy+1] = '<';
				update->fireCount++;
			}
			/*se contagem de tiro for 3 então impede que o usuário atire por 1seg*/
			if ( update->fireCount >= 3 )
			{
				update->cooldown = 10; /*multiplica valor do cooldown com o temporizador de tiro*/
				update->fireCount = 0; /*reinicia contagem de tiro*/
			}

			break;
		case 'a': /*movimenta tanque contanto que n esteja no extremo*/
			if (tank->posy - 1 > 1)
				tank->posy--;
				initTank(tank->posx, tank->posy, update);
			break;
		case 'd': /*movimenta tanque contanto que n esteja no extremo*/
			if (tank->posy + 1 < COLUMN-2)
				tank->posy++;
				initTank(tank->posx, tank->posy, update);
			break;
		case 'q': /*sai do jogo*/
			endwin();
			exit(0);
		default:
			break;
	}
}

/* TELA DE FIM DE JOGO */
void GameOver()
{
	int key, getx, gety;
	getmaxyx(stdscr, gety, getx);

	while (key!='q')
	{
		clear();
		refresh();
		mvprintw(gety/2,getx/3,"GAME OVER");
		mvprintw( (gety/2)+1,getx/3,"Push 'r' to restart");
		mvprintw( (gety/2)+2,getx/3,"Push 'q' to quit");
		key = getch();

		if ( key == 'r' )
			main();

		usleep(100000);
	}
	endwin();
	exit(0);
}
