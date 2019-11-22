#include <stdio.h>
#include <stdlib.h>
#include "lib_lista.h"
#include "asciiART.h"

void destroiLista (t_node *first)
{
        t_node *aux;
        t_node *aux2;

        aux2 = first;
        while(aux2->prox->prox != NULL)
        {
                aux = aux2->prox;
                aux->prev->prev = NULL;
                aux->prev->prox = NULL;
                free(aux2);
                aux2 = aux;
        }
        free(aux);
        aux = NULL;
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
