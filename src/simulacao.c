/*
 * simulacao.c — laço de Monte Carlo e cálculo das margens de erro.
 */
#include <math.h>
#include "simulacao.h"
#include "aleatorio.h"

/* Valor de z para 95% de confiança na distribuição normal. */
#define Z_95 1.96

Resumo simular(Regras regras, int partidas, uint64_t semente)
{
    Resumo resumo = {0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    /* Mesma semente -> mesmas partidas -> mesmo resultado em qualquer máquina. */
    // isso deixa os resultados reproduziveis e faz cada cenário depender depender das próprias regras e não da ordem em que o programa roda
    aleatorio_semear(semente);

    for (int i = 0; i < partidas; i++) {
        ResultadoPartida partida = jogar_partida(regras, 0);
        int cobras = partida.cobras_j1 + partida.cobras_j2;

        resumo.partidas++;
        if (partida.vencedor == 1) {
            resumo.vitorias_j1++;
        } else if (partida.vencedor == 2) {
            resumo.vitorias_j2++;
        }

        resumo.soma_lancamentos      += partida.lancamentos;
        resumo.soma_lancamentos_quad += (double)partida.lancamentos * partida.lancamentos;
        resumo.soma_cobras           += cobras;
        resumo.soma_cobras_quad      += (double)cobras * cobras;
        resumo.soma_cobras_j1        += partida.cobras_j1;
        resumo.soma_cobras_j2        += partida.cobras_j2;
    }
    return resumo;
}

/*
 * Para uma proporção p estimada com n partidas, o erro padrão é
 * raiz(p * (1 - p) / n). A margem de 95% é 1,96 vezes isso.
 * Ex.: p = 0,5 e n = 10.000 -> erro padrão 0,005 -> margem de ~1 ponto percentual.
 */
Estimativa estimar_proporcao(int sucessos, int total)
{
    Estimativa estimativa;
    estimativa.valor = (double)sucessos / total;
    estimativa.margem = Z_95 * sqrt(estimativa.valor * (1.0 - estimativa.valor) / total);
    return estimativa;
}

/*
 * Para uma média, o erro padrão é desvio_padrão / raiz(n).
 * A variância sai das somas: (soma dos quadrados - soma^2 / n) / (n - 1).
 * (O n - 1 é a correção usual para variância amostral.)
 */
Estimativa estimar_media(double soma, double soma_quadrados, int total)
{
    Estimativa estimativa;
    double variancia = (soma_quadrados - soma * soma / total) / (total - 1);

    estimativa.valor = soma / total;
    estimativa.margem = Z_95 * sqrt(variancia / total);
    return estimativa;
}
