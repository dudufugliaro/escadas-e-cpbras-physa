/*
 * jogo.c — o motor do jogo.
 *
 * Fluxo de um turno:
 *   1. rola o dado e anda;
 *   2. se chegou ou passou da casa 36, terminou;
 *   3. se caiu na base de uma escada, sobe (na Pergunta 3, só com 50%);
 *   4. se caiu na cabeça de uma cobra, desce (na Pergunta 5, o J2 ignora a primeira).
 *
 * Premissas (ver README):
 *   - cobra e escada só valem quando o movimento TERMINA na casa;
 *   - não há jogada extra ao tirar 6, nem "voltar" ao passar do 36;
 *   - o jogo acaba no momento em que alguém chega ou passa do 36.
 */
#include <stdio.h>
#include "jogo.h"
#include "tabuleiro.h"
#include "aleatorio.h"

/* Trava de segurança: se houver um erro de lógica, a partida para em vez de
   rodar para sempre. Na prática nenhuma partida chega perto disso. */
#define LIMITE_DE_LANCAMENTOS 100000

/* Estado de um jogador durante a partida (só este arquivo usa). */
typedef struct {
    int numero;   /* 1 ou 2, usado só nas mensagens */
    int casa;     /* posição atual */
    int cobras;   /* quantas vezes desceu por uma cobra */
    int imune;    /* 1 = ainda tem a imunidade à primeira cobra (Pergunta 5) */
} Jogador;

Regras regras_originais(void)
{
    Regras regras;
    regras.inicio_j2 = 1;
    regras.prob_subir_escada = 1.0;
    regras.imunidade_j2 = 0;
    regras.tabuleiro_vazio = 0;
    return regras;
}

static Jogador novo_jogador(int numero, int casa_inicial, int imune)
{
    Jogador jogador;
    jogador.numero = numero;
    jogador.casa = casa_inicial;
    jogador.cobras = 0;
    jogador.imune = imune;
    return jogador;
}

/*
 * Um turno de um jogador. Recebe uma cópia do jogador, atualiza a cópia e a
 * devolve. Quem chama faz: j1 = jogar_turno(j1, regras, 0);
 */
static Jogador jogar_turno(Jogador jogador, Regras regras, int mostrar_passos)
{
    int dado = aleatorio_dado();
    int casa_antes = jogador.casa;
    const char *evento = "";   /* texto só para o modo passo a passo */

    jogador.casa = jogador.casa + dado;
    int casa_do_dado = jogador.casa;   /* onde caiu, antes de escada ou cobra */

    /* Basta um único salto por turno: nenhuma escada ou cobra termina em
       outra escada ou cobra (conferido em tabuleiro_consistente). */
    if (jogador.casa >= CASA_FINAL) {
        /* Chegou ou passou da última casa: terminou. Não precisa do número exato. */
        evento = "(chegou ao fim)";

    } else if (regras.tabuleiro_vazio) {
        /* Só na validação: ignora cobras e escadas. */

    } else if (eh_base_de_escada(jogador.casa)) {
        /* Pergunta 3: sobe com a probabilidade das regras. Com chance 1.0 nem
           sorteia, para o cenário original não gastar sorteios à toa. */
        if (regras.prob_subir_escada >= 1.0 || aleatorio_uniforme() < regras.prob_subir_escada) {
            jogador.casa = topo_da_escada(jogador.casa);
            evento = "(escada: subiu)";
        } else {
            evento = "(escada: não subiu)";
        }

    } else if (eh_cabeca_de_cobra(jogador.casa)) {
        if (jogador.imune) {
            /* Pergunta 5: gasta a imunidade e fica parado na cabeça da cobra. */
            jogador.imune = 0;
            evento = "(cobra: estava imune, ficou)";
        } else {
            jogador.casa = rabo_da_cobra(jogador.casa);
            jogador.cobras = jogador.cobras + 1;
            evento = "(cobra: desceu)";
        }
    }

    if (mostrar_passos) {
        printf("  J%d: %2d + %d = %2d", jogador.numero, casa_antes, dado, casa_do_dado);
        if (jogador.casa != casa_do_dado) {
            printf(" -> %2d", jogador.casa);   /* mostra o salto da escada ou da cobra */
        }
        if (evento[0] != '\0') {               /* só escreve o evento se houver um */
            printf("  %s", evento);
        }
        printf("\n");
    }
    return jogador;
}

ResultadoPartida jogar_partida(Regras regras, int mostrar_passos)
{
    Jogador j1 = novo_jogador(1, 1, 0);
    Jogador j2 = novo_jogador(2, regras.inicio_j2, regras.imunidade_j2);
    ResultadoPartida resultado = {0, 0, 0, 0};

    /* Cada volta do laço é uma rodada: J1 joga, depois J2.
       Quem chegar ao fim primeiro vence, e o jogo acaba na hora (break). */
    while (resultado.lancamentos < LIMITE_DE_LANCAMENTOS) {
        j1 = jogar_turno(j1, regras, mostrar_passos);
        resultado.lancamentos++;
        if (j1.casa >= CASA_FINAL) {
            resultado.vencedor = 1;
            break;
        }

        j2 = jogar_turno(j2, regras, mostrar_passos);
        resultado.lancamentos++;
        if (j2.casa >= CASA_FINAL) {
            resultado.vencedor = 2;
            break;
        }
    }

    resultado.cobras_j1 = j1.cobras;
    resultado.cobras_j2 = j2.cobras;
    return resultado;
}

int turnos_para_terminar_sozinho(Regras regras, int casa_inicial)
{
    Jogador jogador = novo_jogador(1, casa_inicial, 0);
    int turnos = 0;

    while (jogador.casa < CASA_FINAL && turnos < LIMITE_DE_LANCAMENTOS) {
        jogador = jogar_turno(jogador, regras, 0);
        turnos++;
    }
    return turnos;
}
