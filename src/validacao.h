/*
 * validacao.h — testes que mostram que o código faz o que as regras dizem.
 */
#ifndef VALIDACAO_H
#define VALIDACAO_H

#include <stdint.h>

/*
 * Roda todas as validações, imprime [OK] ou [FALHOU] para cada uma e mostra
 * uma partida passo a passo. Devolve o número de validações que falharam.
 */
int executar_validacoes(uint64_t semente);

#endif
