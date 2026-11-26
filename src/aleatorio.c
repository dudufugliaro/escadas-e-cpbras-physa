/*
 * aleatorio.c — gerador de números pseudoaleatórios com semente.
 *
 * Por que não usar rand() da biblioteca padrão?
 *   O algoritmo por trás do rand() muda de um compilador para outro (Linux,
 *   Windows, macOS), e no Windows ele só gera números até 32767. Com a mesma
 *   semente, cada máquina geraria partidas diferentes e os números do
 *   relatório não seriam reproduzíveis por quem for corrigir.
 *
 *   Por isso usamos o xorshift64*, um gerador pequeno e conhecido
 *   (Marsaglia, 2003; variante de Vigna, 2016). Ele dá a mesma sequência
 *   em qualquer máquina e tem qualidade de sobra para esta simulação.
 */
#include "aleatorio.h"

/* O "estado" é a memória do gerador: cada sorteio parte dele e o atualiza.
   `static` deixa a variável visível só dentro deste arquivo. */
static uint64_t estado = 1; //cada dado sorteado altera o estado, e o próximo parte dele

/* Avança o estado e devolve 64 bits embaralhados (algoritmo xorshift64*).
   Os deslocamentos (>>, <<) e o "ou exclusivo" (^) misturam os bits;
   a multiplicação final espalha ainda mais o resultado. */
static uint64_t proximo_numero(void)
{
    //como o gerador tem uma unica memoria que é a variavel de estado, cada chamada faz duas coisas
    // avança o estado
    //produz uma saída - embaralha o estado mais uma vez
    //pseudoaleatório
    estado ^= estado >> 12;
    estado ^= estado << 25;
    estado ^= estado >> 27;
    return estado * 2685821657736338717ULL;
}

// coloca o estado num ponto conhecido / estado = semente coloca o ponto de partida na fila
void aleatorio_semear(uint64_t semente)
{
    //se o estado for 0, o gerador entra em loop no zero
    estado = (semente != 0) ? semente : 1;

    /* Descarta os primeiros números: uma semente pequena, como 2026, tem
       poucos bits ligados, e algumas rodadas de mistura resolvem isso. */
    for (int i = 0; i < 20; i++) {
        proximo_numero();
    }
}
// simular() chama aleatorio_semear(semente) logo no inicio, sempre
// isso permite reprodutibilidade => rodar o programa de novo, em qualquer máquina, gera as mesmas partidase os mesmos números em resultados.txt

// cada cenário fica independente da ordem do programa, isso é muito importante
// sem semear de novo, a P3 começaria de onde a P2 parou na fila
// o resultado da P3 dependeria de quantos números as perguntas anteriores consumiram
// caso seja acrescentado um teste novo, ou trocasse a ordem as perguntas, todos os resultados seguintes mudariam, mesmo sem mudança na lógica
// com a smente reposionada no inicio de cada simular, o resultado da P3 depende apenas das regras da P3 e da semente

double aleatorio_uniforme(void)
{
    /* Pega os 53 bits mais altos (>> 11) e divide por 2^53. Um double tem
       53 bits de precisão, então o resultado cobre [0, 1) de forma uniforme. */
    return (double)(proximo_numero() >> 11) / 9007199254740992.0;
}

int aleatorio_dado(void)
{
    /* uniforme * 6 cai em [0, 6); a parte inteira é 0, 1, ..., 5, cada uma
       com chance 1/6. Somando 1, o resultado vai de 1 a 6. */
    return 1 + (int)(aleatorio_uniforme() * 6.0);
}
