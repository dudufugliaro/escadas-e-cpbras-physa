/*
 * simulacao.h — repete muitas partidas de um cenário e resume os números.
 *
 * Monte Carlo: em vez de calcular as probabilidades à mão, jogamos milhares
 * de partidas e usamos as frequências observadas como estimativas.
 */
#ifndef SIMULACAO_H
#define SIMULACAO_H

#include <stdint.h>
#include "jogo.h"

/*
 * Totais acumulados de várias partidas. Guardamos somas (e somas dos
 * quadrados) em vez de cada partida: com elas já dá para calcular médias
 * e margens de erro, sem precisar de vetores grandes.
 */
typedef struct {
    int    partidas;
    int    vitorias_j1;
    int    vitorias_j2;
    double soma_lancamentos;         /* soma dos lançamentos de todas as partidas */
    double soma_lancamentos_quad;    /* soma dos quadrados (para a margem de erro) */
    double soma_cobras;              /* cobras por partida, somando os dois jogadores */
    double soma_cobras_quad;
    double soma_cobras_j1;
    double soma_cobras_j2;
} Resumo;

/* Uma estimativa e sua margem de erro (intervalo de 95% de confiança). */
typedef struct {
    double valor;
    double margem;   /* o resultado é "valor ± margem" */
} Estimativa;

/* Joga `partidas` partidas com as regras dadas, a partir de uma semente. */
Resumo simular(Regras regras, int partidas, uint64_t semente);

/* Proporção de sucessos (ex.: vitórias do J1) com margem de erro. */
Estimativa estimar_proporcao(int sucessos, int total);

/* Média de uma quantidade (ex.: lançamentos por partida) com margem de erro. */
Estimativa estimar_media(double soma, double soma_quadrados, int total);

#endif
