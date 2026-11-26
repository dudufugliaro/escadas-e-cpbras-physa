/*
 * tabuleiro.c — escadas e cobras do tabuleiro do teste.
 *
 * Leitura do tabuleiro (numeração em zigue-zague, de 1 a 36):
 *   Escadas: 3 -> 16, 5 -> 7, 15 -> 25, 18 -> 20, 21 -> 32
 *   Cobras:  12 -> 2, 14 -> 11, 17 -> 4, 31 -> 19, 35 -> 22
 */
#include <stdio.h>
#include "tabuleiro.h"

/*
 * Cada vetor é indexado pelo número da casa (de 0 a 36; a posição 0 não é
 * usada). O valor guardado é a casa de destino, e 0 quer dizer "nada aqui".
 *
 * A sintaxe [3] = 16 (inicializador designado, do C99) preenche só as
 * posições indicadas; todas as outras ficam com 0 automaticamente.
 * Exemplo: ESCADA[3] vale 16, então quem cai na casa 3 sobe para a 16.
 */
static const int ESCADA[CASA_FINAL + 1] = {
    [3] = 16, [5] = 7, [15] = 25, [18] = 20, [21] = 32
};

static const int COBRA[CASA_FINAL + 1] = {
    [12] = 2, [14] = 11, [17] = 4, [31] = 19, [35] = 22
};

/* Garante que nunca vamos ler fora dos vetores. */
static int casa_valida(int casa)
{
    return casa >= 1 && casa <= CASA_FINAL;
}

int eh_base_de_escada(int casa)
{
    return casa_valida(casa) && ESCADA[casa] != 0;
}

int topo_da_escada(int casa)
{
    return ESCADA[casa];
}

int eh_cabeca_de_cobra(int casa)
{
    return casa_valida(casa) && COBRA[casa] != 0;
}

int rabo_da_cobra(int casa)
{
    return COBRA[casa];
}

void imprimir_tabuleiro(void)
{
    printf("Escadas (sobe):");
    for (int casa = 1; casa <= CASA_FINAL; casa++) {
        if (eh_base_de_escada(casa)) {
            printf("  %d -> %d", casa, topo_da_escada(casa));
        }
    }
    printf("\n");

    printf("Cobras (desce):");
    for (int casa = 1; casa <= CASA_FINAL; casa++) {
        if (eh_cabeca_de_cobra(casa)) {
            printf("  %d -> %d", casa, rabo_da_cobra(casa));
        }
    }
    printf("\n");
}

int tabuleiro_consistente(void)
{
    for (int casa = 1; casa <= CASA_FINAL; casa++) {
        /* Uma casa não pode ter escada e cobra ao mesmo tempo. */
        if (eh_base_de_escada(casa) && eh_cabeca_de_cobra(casa)) {
            return 0;
        }

        if (eh_base_de_escada(casa)) {
            int topo = topo_da_escada(casa);
            /* A escada precisa subir e terminar antes da última casa. */
            if (topo <= casa || topo >= CASA_FINAL) {
                return 0;
            }
            /* O topo não pode cair em outra escada ou cobra: o motor do
               jogo aplica um único salto por turno (sem encadeamento). */
            if (eh_base_de_escada(topo) || eh_cabeca_de_cobra(topo)) {
                return 0;
            }
        }

        if (eh_cabeca_de_cobra(casa)) {
            int rabo = rabo_da_cobra(casa);
            /* A cobra precisa descer e terminar dentro do tabuleiro. */
            if (rabo >= casa || rabo < 1) {
                return 0;
            }
            if (eh_base_de_escada(rabo) || eh_cabeca_de_cobra(rabo)) {
                return 0;
            }
        }
    }
    return 1;
}
