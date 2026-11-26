/*
 * jogo.h — as regras: um turno e uma partida completa.
 *
 * As structs são passadas e devolvidas por valor (cópia). Assim não
 * precisamos de ponteiros: uma função recebe o estado, calcula o novo
 * estado e o devolve.
 */
#ifndef JOGO_H
#define JOGO_H

/*
 * As regras de um cenário. regras_originais() devolve as regras do enunciado;
 * cada pergunta muda um único campo, como um botão (cenários independentes).
 */
typedef struct {
    int    inicio_j2;          /* casa onde o Jogador 2 começa (original: 1)  -> Pergunta 4 */
    double prob_subir_escada;  /* chance de subir ao cair numa escada (1.0)   -> Pergunta 3 */
    int    imunidade_j2;       /* 1 = J2 ignora a primeira cobra (original: 0) -> Pergunta 5 */
    int    tabuleiro_vazio;    /* 1 = ignora cobras e escadas; só para a validação */
} Regras;

/* O que guardamos de cada partida para as análises. */
typedef struct {
    int vencedor;      /* 1 ou 2 (0 só se a partida estourar a trava de segurança) */
    int lancamentos;   /* lançamentos de dado na partida, somando os dois jogadores */
    int cobras_j1;     /* quantas vezes o J1 desceu por uma cobra */
    int cobras_j2;     /* quantas vezes o J2 desceu por uma cobra */
} ResultadoPartida;

/* Devolve as regras do enunciado, sem nenhuma alteração. */
Regras regras_originais(void);

/*
 * Joga uma partida completa entre J1 e J2 (J1 sempre começa).
 * Se mostrar_passos for 1, imprime cada turno (para conferir à mão).
 */
ResultadoPartida jogar_partida(Regras regras, int mostrar_passos);

/*
 * Um jogador sozinho, a partir de casa_inicial: devolve quantos turnos ele
 * precisa para chegar ao fim. Usado na validação (fórmula da Pergunta 1 e
 * teste do tabuleiro vazio).
 */
int turnos_para_terminar_sozinho(Regras regras, int casa_inicial);

#endif
