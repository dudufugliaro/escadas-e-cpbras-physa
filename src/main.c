/*
 * main.c — roteiro do relatório: valida o código e responde às 5 perguntas.
 *
 * Compilar (na pasta do projeto):
 *   gcc -std=c99 -Wall -Wextra -O2 src/main.c src/tabuleiro.c src/aleatorio.c
 *       src/jogo.c src/simulacao.c src/validacao.c -o cobras -lm
 * Rodar:     ./cobras
 * Registro:  ./cobras > resultados.txt
 */
#include <stdio.h>
#include <math.h>
#include "tabuleiro.h"
#include "jogo.h"
#include "simulacao.h"
#include "validacao.h"

#define PARTIDAS             10000   /* partidas por cenário, como pede o enunciado */
#define PARTIDAS_CONFIRMACAO 200000  /* para desempatar casas vizinhas na Pergunta 4 */
#define SEMENTE              2026    /* semente fixa: resultados reproduzíveis */

/* ---------------------------------------------------------------------------
 * Funções de impressão
 * ------------------------------------------------------------------------- */

static void titulo(const char *texto)
{
    printf("\n============================================================\n");
    printf("%s\n", texto);
    printf("============================================================\n");
}

/* Imprime uma probabilidade como "52.3% (± 1.0 p.p.)". */
static void imprimir_percentual(Estimativa estimativa)
{
    printf("%.1f%% (± %.1f p.p.)", 100.0 * estimativa.valor, 100.0 * estimativa.margem);
}

/* Barra de texto: cada '#' vale 2 pontos percentuais e o '|' marca 50%. */
static void imprimir_barra(double probabilidade)
{
    int tamanho = (int)(probabilidade * 50.0 + 0.5);   /* + 0.5 arredonda */

    for (int i = 0; i < 50; i++) {
        if (i == 25) {
            printf("|");                 /* marca de 50% */
        }
        if (i < tamanho) {
            printf("#");
        } else if (i < 25) {
            printf(" ");                 /* espaço só até a marca de 50% */
        }
    }
}

static void cabecalho(void)
{
    titulo("COBRAS E ESCADAS - SIMULAÇÃO DE MONTE CARLO");
    printf("Tabuleiro de %d casas. Os dois jogadores começam na casa 1; J1 joga primeiro.\n",
           CASA_FINAL);
    imprimir_tabuleiro();
    printf("Premissas: chegar ou passar da casa %d vence na hora; cobra e escada só valem\n",
           CASA_FINAL);
    printf("quando o movimento termina nelas; sem jogada extra ao tirar 6; cada pergunta\n");
    printf("muda uma única regra em relação ao jogo original (cenários independentes).\n");
    printf("Partidas por cenário: %d | semente: %d\n", PARTIDAS, SEMENTE);
    printf("Margens (±): intervalo de 95%% de confiança. p.p. = pontos percentuais.\n");
}

/* ---------------------------------------------------------------------------
 * Perguntas
 * ------------------------------------------------------------------------- */

static void pergunta_1(Resumo base)
{
    titulo("PERGUNTA 1 - Chance de vitória de quem começa (J1)");

    Estimativa j1 = estimar_proporcao(base.vitorias_j1, base.partidas);
    printf("J1 venceu %d de %d partidas.\n", base.vitorias_j1, base.partidas);
    printf("P(J1 vence) = ");
    imprimir_percentual(j1);
    printf("\n");

    /* Mostra o ruído da simulação: outras sementes dão números parecidos,
       todos dentro da margem de erro. */
    printf("Mesma conta com as sementes 1 a 5:");
    for (int semente = 1; semente <= 5; semente++) {
        Resumo outro = simular(regras_originais(), PARTIDAS, semente);
        printf("  %.1f%%", 100.0 * outro.vitorias_j1 / outro.partidas);
    }
    printf("\n");
}

static void pergunta_2(Resumo base)
{
    titulo("PERGUNTA 2 - Cobras por partida");

    Estimativa cobras = estimar_media(base.soma_cobras, base.soma_cobras_quad, base.partidas);
    printf("Média de cobras por partida, somando os dois jogadores: %.2f (± %.2f)\n",
           cobras.valor, cobras.margem);
    printf("Por jogador: J1 %.2f | J2 %.2f\n",
           base.soma_cobras_j1 / base.partidas, base.soma_cobras_j2 / base.partidas);
}

static void pergunta_3(Resumo base)
{
    titulo("PERGUNTA 3 - Lançamentos por partida com escadas de 50%");

    Regras regras = regras_originais();
    regras.prob_subir_escada = 0.5;          /* a única mudança deste cenário */
    Resumo meio = simular(regras, PARTIDAS, SEMENTE);

    Estimativa original = estimar_media(base.soma_lancamentos, base.soma_lancamentos_quad,
                                        base.partidas);
    Estimativa com_50 = estimar_media(meio.soma_lancamentos, meio.soma_lancamentos_quad,
                                      meio.partidas);

    printf("Lançamentos por partida, somando os dois jogadores:\n");
    printf("  regra original (sempre sobe): %.2f (± %.2f)\n", original.valor, original.margem);
    printf("  escada com 50%% de chance:     %.2f (± %.2f)\n", com_50.valor, com_50.margem);
    printf("  diferença: %+.2f lançamentos (%+.0f%%)\n",
           com_50.valor - original.valor, 100.0 * (com_50.valor / original.valor - 1.0));
}

static void pergunta_4(void)
{
    titulo("PERGUNTA 4 - Casa inicial do J2 que equilibra o jogo");
    printf("O J2 começa na casa indicada; o J1 continua na casa 1 e joga primeiro.\n");
    printf("Casas com escada ou cobra ficam de fora (começar no 3 seria como começar no 16).\n");
    printf("Cada '#' vale 2 p.p.; o '|' marca 50%%.\n\n");
    printf("Casa  P(J1 vence)\n");

    double prob[CASA_FINAL + 1];   /* P(J1 vence) para cada casa inicial; -1 = não testada */
    int melhor_casa = 1;

    for (int casa = 1; casa < CASA_FINAL; casa++) {
        prob[casa] = -1.0;
        if (eh_base_de_escada(casa) || eh_cabeca_de_cobra(casa)) {
            printf("%4d  (casa especial, não testada)\n", casa);
            continue;
        }

        Regras regras = regras_originais();
        regras.inicio_j2 = casa;             /* a única mudança deste cenário */
        Resumo resumo = simular(regras, PARTIDAS, SEMENTE);
        Estimativa p = estimar_proporcao(resumo.vitorias_j1, resumo.partidas);
        prob[casa] = p.valor;

        printf("%4d  %5.1f%% ± %.1f  ", casa, 100.0 * p.valor, 100.0 * p.margem);
        imprimir_barra(p.valor);
        printf("\n");

        if (fabs(p.valor - 0.5) < fabs(prob[melhor_casa] - 0.5)) {
            melhor_casa = casa;
        }
    }
    printf("\nMais perto de 50%% com %d partidas: casa %d.\n", PARTIDAS, melhor_casa);

    /* Com cerca de ± 1 p.p. de ruído, casas vizinhas podem empatar. As casas a
       até 2 p.p. de 50% são simuladas de novo, com muito mais partidas e outra
       semente, para confirmar a escolha. */
    printf("\nConfirmação com %d partidas (outra semente), casas a até 2 p.p. de 50%%:\n",
           PARTIDAS_CONFIRMACAO);
    int casa_escolhida = melhor_casa;
    double menor_distancia = 1.0;

    for (int casa = 1; casa < CASA_FINAL; casa++) {
        if (prob[casa] < 0.0 || fabs(prob[casa] - 0.5) > 0.02) {
            continue;
        }
        Regras regras = regras_originais();
        regras.inicio_j2 = casa;
        Resumo resumo = simular(regras, PARTIDAS_CONFIRMACAO, SEMENTE + 1);
        Estimativa p = estimar_proporcao(resumo.vitorias_j1, resumo.partidas);
        printf("%4d  %5.2f%% ± %.2f\n", casa, 100.0 * p.valor, 100.0 * p.margem);

        if (fabs(p.valor - 0.5) < menor_distancia) {
            menor_distancia = fabs(p.valor - 0.5);
            casa_escolhida = casa;
        }
    }
    printf("Resposta: o J2 deve começar na casa %d.\n", casa_escolhida);
}

static void pergunta_5(Resumo base)
{
    titulo("PERGUNTA 5 - J2 imune à primeira cobra");

    Regras regras = regras_originais();
    regras.imunidade_j2 = 1;                 /* a única mudança deste cenário */
    Resumo resumo = simular(regras, PARTIDAS, SEMENTE);

    Estimativa original = estimar_proporcao(base.vitorias_j1, base.partidas);
    Estimativa com_imunidade = estimar_proporcao(resumo.vitorias_j1, resumo.partidas);

    printf("P(J1 vence), regra original:   ");
    imprimir_percentual(original);
    printf("\nP(J1 vence), J2 com imunidade: ");
    imprimir_percentual(com_imunidade);
    printf("\nDiferença: %+.1f p.p.\n", 100.0 * (com_imunidade.valor - original.valor));
}

/* ---------------------------------------------------------------------------
 * Programa principal
 * ------------------------------------------------------------------------- */

int main(void)
{
    cabecalho();

    titulo("VALIDAÇÃO DO CÓDIGO");
    if (executar_validacoes(SEMENTE) > 0) {
        printf("Corrija as falhas antes de confiar nos resultados.\n");
        return 1;
    }

    /* O cenário original é simulado uma única vez e serve às Perguntas 1, 2, 3 e 5. */
    Resumo base = simular(regras_originais(), PARTIDAS, SEMENTE);

    pergunta_1(base);
    pergunta_2(base);
    pergunta_3(base);
    pergunta_4();
    pergunta_5(base);
    return 0;
}
