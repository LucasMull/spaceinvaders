#define ROW 38
#define COL 100

#define TRUE 1
#define FALSE 0

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

void destroiLista (t_node *first);
void removeAtualLista(t_node *atual);

int inicializaListaNaves (t_lista *l);
int inicializaListaTiros (t_tiro *t);
int inicializaListaBarreiras(t_wall *w);

int inicializaNaves (t_lista *l);
int inicializaTiros(t_lista *l, t_tiro *t, int ID);
int inicializaBarreiras(t_wall *w, void *mat[ROW][COL]);
