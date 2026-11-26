/*
 * tabuleiro.h — os dados do tabuleiro: onde estão as escadas e as cobras.
 *
 * Este módulo só guarda e consulta dados. Ele não sabe nada sobre turnos,
 * jogadores ou dados (de jogar). Assim, trocar de tabuleiro é trocar só
 * este arquivo, sem mexer nas regras do jogo.
 */
#ifndef TABULEIRO_H
#define TABULEIRO_H

/*
->
*/

/* Última casa. Chegar nela ou passar dela vence o jogo. */
#define CASA_FINAL 36

/* Devolve 1 se a casa é a base de uma escada, 0 se não é. */
int eh_base_de_escada(int casa);

/* Devolve a casa do topo da escada que começa em `casa`. */
int topo_da_escada(int casa);

/* Devolve 1 se a casa é a cabeça de uma cobra, 0 se não é. */
int eh_cabeca_de_cobra(int casa);

/* Devolve a casa do rabo da cobra cuja cabeça está em `casa`. */
int rabo_da_cobra(int casa);

/* Imprime a lista de escadas e cobras (usado no cabeçalho do relatório). */
void imprimir_tabuleiro(void);

/*
 * Confere se os dados do tabuleiro fazem sentido: escadas sobem, cobras
 * descem, nenhuma casa tem escada e cobra ao mesmo tempo, e nenhum salto
 * termina em outra escada ou cobra. Devolve 1 se está tudo certo.
 */
int tabuleiro_consistente(void);

#endif
