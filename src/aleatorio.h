/*
 * aleatorio.h — sorteios: o dado e a "moeda" da escada de 50%.
 *
 * Usa uma semente: a mesma semente produz sempre a mesma sequência de
 * sorteios, em qualquer computador. Isso torna os resultados reproduzíveis.
 */
#ifndef ALEATORIO_H
#define ALEATORIO_H

#include <stdint.h>   /* uint64_t: inteiro sem sinal de exatamente 64 bits */

/* Reinicia o gerador a partir de uma semente. */
void aleatorio_semear(uint64_t semente);

/* Sorteia um número real uniforme no intervalo [0, 1). */
double aleatorio_uniforme(void);

/* Rola um dado justo de 6 lados: devolve um inteiro de 1 a 6. */
int aleatorio_dado(void);

#endif
