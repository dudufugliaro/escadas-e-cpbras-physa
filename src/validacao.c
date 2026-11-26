/*
 * validacao.c — testes de sanidade do código.
 *
 * Uma simulação sempre devolve algum número, mesmo com erro de lógica.
 * Estes testes comparam o código com resultados que sabemos de antemão.
 */
#include <stdio.h>
#include <math.h>
#include "validacao.h"
#include "tabuleiro.h"
#include "jogo.h"
#include "simulacao.h"
#include "aleatorio.h"

/* Mais partidas que nas perguntas, para os testes ficarem bem precisos. */
#define PARTIDAS_VALIDACAO 100000

/* Lançamentos usados para testar o dado (rápido em C: alguns milissegundos). */
#define LANCAMENTOS_TESTE_DADO 6000000

/* Imprime [OK] ou [FALHOU] e devolve 1 se falhou (para somar as falhas). */
static int rotulo(int passou)
{
    printf("%s", passou ? "[OK]     " : "[FALHOU] ");
    return passou ? 0 : 1;
}

/* Teste 1: os dados do tabuleiro fazem sentido? */
static int validar_tabuleiro(void)
{
    int falhas = rotulo(tabuleiro_consistente());
    printf("Tabuleiro: escadas sobem, cobras descem, sem casas com as duas coisas e sem saltos encadeados\n");
    return falhas;
}

/*
 * Teste 2: o dado é justo e um lançamento não influencia o seguinte?
 * Cada face deve sair em ~1/6 das vezes, e um lançamento deve repetir o
 * anterior também em ~1/6 das vezes (se repetisse mais, haveria "memória").
 */
static int validar_dado(uint64_t semente)
{
    int frequencia[7] = {0, 0, 0, 0, 0, 0, 0};   /* frequencia[face]; a posição 0 não é usada */
    int iguais_ao_anterior = 0;
    int anterior = 0;

    aleatorio_semear(semente);
    for (int i = 0; i < LANCAMENTOS_TESTE_DADO; i++) {
        int dado = aleatorio_dado();
        frequencia[dado]++;
        if (dado == anterior) {
            iguais_ao_anterior++;
        }
        anterior = dado;
    }

    double menor = 1.0;
    double maior = 0.0;
    for (int face = 1; face <= 6; face++) {
        double proporcao = (double)frequencia[face] / LANCAMENTOS_TESTE_DADO;
        if (proporcao < menor) {
            menor = proporcao;
        }
        if (proporcao > maior) {
            maior = proporcao;
        }
    }
    double repeticao = (double)iguais_ao_anterior / (LANCAMENTOS_TESTE_DADO - 1);
    double um_sexto = 1.0 / 6.0;

    /* Com 6 milhões de lançamentos, o erro padrão de cada proporção é ~0,015 p.p.;
       a tolerância de 0,1 p.p. é bem folgada. */
    int passou = (um_sexto - menor) < 0.001 && (maior - um_sexto) < 0.001
                 && fabs(repeticao - um_sexto) < 0.001;
    int falhas = rotulo(passou);
    printf("Dado justo: faces entre %.2f%% e %.2f%%; repete o anterior em %.2f%% (esperado 16.67%%)\n",
           100.0 * menor, 100.0 * maior, 100.0 * repeticao);
    return falhas;
}

/*
 * Valor exato para o tabuleiro vazio (sem cobras e escadas).
 * esperado[k] = número médio de lançamentos para andar pelo menos k casas.
 *   - Se k <= 0, não precisa lançar: 0.
 *   - Senão, lança uma vez; para cada face d (chance 1/6) ainda faltam k - d:
 *       esperado[k] = 1 + (esperado[k-1] + ... + esperado[k-6]) / 6
 */
static double lancamentos_esperados_tabuleiro_vazio(int casas)
{
    double esperado[CASA_FINAL + 1];

    esperado[0] = 0.0;
    for (int k = 1; k <= casas; k++) {
        double soma = 0.0;
        for (int d = 1; d <= 6; d++) {
            if (k - d > 0) {
                soma += esperado[k - d];
            }
        }
        esperado[k] = 1.0 + soma / 6.0;
    }
    return esperado[casas];
}

/* Teste 3: sem cobras e escadas, o movimento bate com a conta exata? */
static int validar_tabuleiro_vazio(uint64_t semente)
{
    Regras vazio = regras_originais();
    vazio.tabuleiro_vazio = 1;

    double soma = 0.0;
    double soma_quad = 0.0;
    aleatorio_semear(semente);
    for (int i = 0; i < PARTIDAS_VALIDACAO; i++) {
        int turnos = turnos_para_terminar_sozinho(vazio, 1);
        soma += turnos;
        soma_quad += (double)turnos * turnos;
    }

    Estimativa media = estimar_media(soma, soma_quad, PARTIDAS_VALIDACAO);
    double exato = lancamentos_esperados_tabuleiro_vazio(CASA_FINAL - 1);  /* da casa 1 à 36: 35 casas */

    /* Simulação nunca bate exatamente: aceitamos diferença de até 2 margens. */
    int falhas = rotulo(fabs(media.valor - exato) <= 2.0 * media.margem);
    printf("Tabuleiro vazio: %.3f lançamentos por jogador (simulado) vs %.3f (exato)\n",
           media.valor, exato);
    return falhas;
}

/* Testes 4 e 5: toda partida termina com vencedor, e a contagem de
   lançamentos combina com quem venceu. */
static int validar_partidas(uint64_t semente)
{
    int sem_vencedor = 0;
    int contagem_errada = 0;

    aleatorio_semear(semente);
    for (int i = 0; i < PARTIDAS_VALIDACAO; i++) {
        ResultadoPartida partida = jogar_partida(regras_originais(), 0);

        if (partida.vencedor == 0) {
            sem_vencedor++;
        }
        /* Se o J1 vence no seu k-ésimo turno, o J2 jogou k - 1 vezes:
           total 2k - 1 (ímpar). Se o J2 vence, os dois jogaram k: total 2k (par). */
        if (partida.vencedor == 1 && partida.lancamentos % 2 == 0) {
            contagem_errada++;
        }
        if (partida.vencedor == 2 && partida.lancamentos % 2 != 0) {
            contagem_errada++;
        }
    }

    int falhas = rotulo(sem_vencedor == 0);
    printf("Toda partida terminou com um vencedor (%d de %d sem vencedor)\n",
           sem_vencedor, PARTIDAS_VALIDACAO);

    falhas += rotulo(contagem_errada == 0);
    printf("Lançamentos ímpares quando J1 vence e pares quando J2 vence (%d erros)\n",
           contagem_errada);
    return falhas;
}

/*
 * Teste 6: a fórmula da Pergunta 1.
 * Os jogadores não interagem, então cada um faz uma corrida sozinho.
 * Seja T o número de turnos que um jogador precisa. O J1 vence se T1 <= T2
 * (no empate de turnos ele chega antes, pois joga primeiro). Como T1 e T2
 * são independentes e têm a mesma distribuição:
 *     P(J1 vence) = (1 + P(T1 = T2)) / 2
 */
static int validar_formula_pergunta_1(uint64_t semente)
{
    Regras regras = regras_originais();

    /* Parte 1: duas corridas solo por rodada; conta os empates em turnos. */
    int empates = 0;
    aleatorio_semear(semente);
    for (int i = 0; i < PARTIDAS_VALIDACAO; i++) {
        int t1 = turnos_para_terminar_sozinho(regras, 1);
        int t2 = turnos_para_terminar_sozinho(regras, 1);
        if (t1 == t2) {
            empates++;
        }
    }
    double prob_empate = (double)empates / PARTIDAS_VALIDACAO;
    double previsto = (1.0 + prob_empate) / 2.0;

    /* Parte 2: o jogo de verdade, com outra semente (amostra independente). */
    Resumo jogo = simular(regras, PARTIDAS_VALIDACAO, semente + 1);
    Estimativa simulado = estimar_proporcao(jogo.vitorias_j1, jogo.partidas);

    int falhas = rotulo(fabs(simulado.valor - previsto) <= 2.0 * simulado.margem);
    printf("Fórmula da Pergunta 1: %.2f%% (jogo simulado) vs %.2f%% = (1 + %.2f%%) / 2\n",
           100.0 * simulado.valor, 100.0 * previsto, 100.0 * prob_empate);
    return falhas;
}

/* Não é um teste automático: imprime uma partida para conferir à mão. */
static void mostrar_partida_exemplo(uint64_t semente)
{
    printf("\nPartida de exemplo, passo a passo (para conferir à mão no tabuleiro).\n");
    printf("Formato: jogador: casa + dado = casa onde caiu [-> destino]  (evento)\n");

    aleatorio_semear(semente);
    ResultadoPartida partida = jogar_partida(regras_originais(), 1);

    printf("Resultado: J%d venceu com %d lançamentos no total; cobras: J1 = %d, J2 = %d\n",
           partida.vencedor, partida.lancamentos, partida.cobras_j1, partida.cobras_j2);
}

int executar_validacoes(uint64_t semente)
{
    int falhas = 0;

    falhas += validar_tabuleiro();
    falhas += validar_dado(semente);
    falhas += validar_tabuleiro_vazio(semente);
    falhas += validar_partidas(semente);
    falhas += validar_formula_pergunta_1(semente);

    if (falhas == 0) {
        printf("Todas as validações passaram.\n");
    } else {
        printf("ATENÇÃO: %d validação(ões) falharam.\n", falhas);
    }

    mostrar_partida_exemplo(semente);
    return falhas;
}
