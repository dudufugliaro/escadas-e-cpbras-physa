# Cobras e Escadas: simulação de Monte Carlo

Este documento é o registro da solução. Ele explica como li o tabuleiro, que premissas assumi, como o código está organizado, como verifiquei que ele está certo e o que os resultados mostram. A saída completa do programa está em `resultados.txt`.

## Respostas

Todas as respostas vêm de 10.000 partidas por cenário. A margem (±) é o intervalo de 95% de confiança.

| Pergunta | Resposta |
|---|---|
| 1. Chance de vitória de quem começa | **52,3%** (± 1,0 p.p.) |
| 2. Cobras por partida (os dois jogadores) | **3,07** (± 0,05), cerca de 1,5 por jogador |
| 3. Lançamentos por partida com escadas de 50% | **22,4** (± 0,2), contra 19,0 na regra original (+18%) |
| 4. Casa inicial do Jogador 2 que equilibra o jogo | **Casa 7**: o Jogador 1 vence 49,6% (± 0,2 p.p., confirmado com 200.000 partidas) |
| 5. Chance de vitória do Jogador 1 com o Jogador 2 imune à primeira cobra | **38,9%** (± 1,0 p.p.), 13,4 p.p. a menos que na regra original |

## Como rodar

O código é C99 puro, sem bibliotecas externas.

```
gcc -std=c99 -Wall -Wextra -O2 src/main.c src/tabuleiro.c src/aleatorio.c src/jogo.c src/simulacao.c src/validacao.c -o cobras -lm
./cobras                    # mostra o relatório na tela
./cobras > resultados.txt   # salva o relatório em arquivo
```

O programa roda em menos de 1 segundo. Compilei com gcc 13 e clang 18 no Linux, sem nenhum aviso mesmo com `-pedantic`, e os dois geram exatamente a mesma saída. No Windows (MinGW), rode `chcp 65001` antes, para os acentos aparecerem corretamente.

## 1. Leitura do tabuleiro

A numeração vai em zigue-zague: 1 a 6 da esquerda para a direita, 7 a 12 da direita para a esquerda, e assim por diante até a casa 36.

| Escadas (sobe) | Cobras (desce) |
|---|---|
| 3 → 16 | 12 → 2 |
| 5 → 7 | 14 → 11 |
| 15 → 25 | 17 → 4 |
| 18 → 20 | 31 → 19 |
| 21 → 32 | 35 → 22 |

Nenhum topo de escada ou rabo de cobra cai em outra escada ou cobra. Por isso basta aplicar um salto por turno. O programa confere isso antes de simular.

## 2. Premissas

As regras do enunciado deixam alguns pontos em aberto. Estas foram as minhas escolhas:

- **Fim do jogo:** chegar ou passar da casa 36 encerra o jogo na hora. Não é preciso tirar o número exato, e o outro jogador não joga mais.
- **Quando cobra e escada valem:** só quando o movimento termina na casa, não quando o jogador passa por cima dela.
- **Regras que não existem:** tirar 6 não dá jogada extra, e quem passa do 36 não volta.
- **Cenários independentes:** cada pergunta parte das regras originais e muda uma única regra.
- **Pergunta 2:** "os jogadores" é a soma das cobras dos dois na mesma partida. Também mostro o valor por jogador.
- **Pergunta 3:** "lançamentos para completar um jogo" é o total da partida, somando os dois jogadores. O sorteio de 50% é feito de novo a cada vez que o jogador cai numa escada. Se não subir, ele fica na base.
- **Pergunta 4:** o Jogador 1 continua começando na casa 1 e jogando primeiro. Não testei como casa inicial as bases de escada nem as cabeças de cobra: começar no 3 seria o mesmo que começar no 16, e começar no 12 sem descer não faz sentido.
- **Pergunta 5:** a imunidade vale uma única vez. Na primeira vez que o Jogador 2 cai numa cabeça de cobra, ele fica parado nela. Depois disso, as cobras valem normalmente para ele.

## 3. Abordagem

Calcular essas probabilidades à mão é difícil, então usei **simulação de Monte Carlo**: o programa joga milhares de partidas e usa a frequência observada como estimativa. Pela lei dos grandes números, essa frequência se aproxima da probabilidade real conforme o número de partidas cresce.

**Precisão.** Para uma proporção p estimada com n partidas, o erro padrão é √(p(1 − p)/n). Com p ≈ 0,5 e n = 10.000, isso dá 0,005, ou seja, uma margem de cerca de ±1 ponto percentual com 95% de confiança. Para médias (Perguntas 2 e 3), o erro padrão é o desvio padrão dividido por √n. Por isso relato os resultados com uma casa decimal e sempre com a margem.

**Reprodutibilidade.** Toda simulação parte de uma semente fixa (2026), então o programa gera sempre os mesmos números. Não usei o `rand()` da biblioteca padrão, porque o algoritmo dele muda entre compiladores (e no Windows ele só vai até 32767). Com ele, cada máquina geraria partidas diferentes. Em vez disso, implementei o xorshift64\*, um gerador pequeno e conhecido que dá a mesma sequência em qualquer máquina.

**Uma ideia que simplifica o problema.** Os jogadores não interagem: ninguém bloqueia nem captura o outro. Então cada um faz uma corrida sozinho. Seja T o número de turnos que um jogador precisa para terminar. O Jogador 1 vence se T1 ≤ T2, porque, se os dois precisarem do mesmo número de turnos, ele chega antes por jogar primeiro. Como T1 e T2 são independentes e têm a mesma distribuição:

> P(J1 vence) = (1 + P(T1 = T2)) / 2

A vantagem de quem começa é exatamente metade da chance de os dois precisarem do mesmo número de turnos. Uso essa fórmula como uma das validações do código.

## 4. Estrutura do código

Cada módulo tem uma única responsabilidade. Para dispensar ponteiros na lógica, as structs são passadas e devolvidas por valor: uma função recebe o estado, calcula o novo estado e o devolve. O único ponteiro do código é o `const char *` usado para textos.

| Arquivo | Responsabilidade |
|---|---|
| `tabuleiro.c/.h` | Dados do tabuleiro: dois vetores indexados pela casa, com o destino de cada escada e cobra. |
| `aleatorio.c/.h` | Gerador de números aleatórios com semente: o dado (1 a 6) e o sorteio uniforme em [0, 1). |
| `jogo.c/.h` | Regras: um turno (`jogar_turno`) e uma partida completa (`jogar_partida`). |
| `simulacao.c/.h` | Repete N partidas de um cenário, acumula os totais e calcula médias e margens de erro. |
| `validacao.c/.h` | Testes que comparam o código com resultados conhecidos de antemão. |
| `main.c` | Roteiro do relatório: validação e depois as 5 perguntas, na ordem. |

Cada pergunta muda um campo de `Regras`, como se girasse um botão: `inicio_j2` (Pergunta 4), `prob_subir_escada` (Pergunta 3) e `imunidade_j2` (Pergunta 5). Os valores padrão, devolvidos por `regras_originais()`, são as regras do enunciado. Assim existe um único motor de jogo, e os cenários ficam comparáveis entre si. A alternativa seria copiar o jogo cinco vezes com pequenas diferenças.

Um turno segue esta ordem: rola o dado e anda → se chegou ou passou do 36, terminou → se caiu na base de uma escada, sobe → se caiu na cabeça de uma cobra, desce. A partida alterna os turnos do Jogador 1 e do Jogador 2 até alguém terminar.

## 5. Validação

Uma simulação sempre devolve algum número, mesmo com erro de lógica. Por isso o programa roda estes testes antes de responder às perguntas, e só continua se todos passarem:

| Teste | O que verifica | Resultado |
|---|---|---|
| Tabuleiro | Escadas sobem, cobras descem, nenhuma casa tem as duas coisas, nenhum salto termina em outro | OK |
| Dado justo | Em 6 milhões de lançamentos, cada face sai ~1/6 das vezes, e um lançamento repete o anterior ~1/6 das vezes (sem "memória") | Faces entre 16,65% e 16,69%; repetição em 16,67% |
| Tabuleiro vazio | Sem cobras e escadas, a média de lançamentos bate com um cálculo exato (programação dinâmica) | 10,476 simulado vs 10,476 exato |
| Sempre há vencedor | Nenhuma partida esbarra na trava de segurança contra laço infinito | 0 de 100.000 |
| Contagem de lançamentos | Se o J1 vence no k-ésimo turno, a partida tem 2k − 1 lançamentos (ímpar); se o J2 vence, tem 2k (par) | 0 erros |
| Fórmula da Pergunta 1 | O jogo simulado bate com (1 + P(T1 = T2)) / 2, estimada com corridas solo | 52,63% vs 52,56% |

O programa também imprime uma partida passo a passo, para conferir à mão no tabuleiro. Nas três primeiras rodadas dessa partida, os dois jogadores tiraram os mesmos números. Isso é coincidência (chance de 1 em 216): o teste do dado mostra que um lançamento não influencia o seguinte.

## 6. Resultados e interpretação

### Pergunta 1: chance de vitória de quem começa

O Jogador 1 venceu 5.228 de 10.000 partidas: **52,3% (± 1,0 p.p.)**. Com as sementes 1 a 5, o resultado ficou entre 52,0% e 52,6%, sempre dentro da margem.

A vantagem de quem começa é pequena, mas real. Pela fórmula da seção 3, ela vale metade da chance de os dois precisarem do mesmo número de turnos. Essa chance é de cerca de 5%, então a vantagem fica em torno de 2,5 p.p.

### Pergunta 2: cobras por partida

Em média, os jogadores caem em **3,07 cobras por partida (± 0,05)**, somando os dois: 1,59 para o Jogador 1 e 1,48 para o Jogador 2. O Jogador 2 cai um pouco menos porque, quando o Jogador 1 vence, o Jogador 2 jogou um turno a menos.

### Pergunta 3: lançamentos com escadas de 50%

| Regra | Lançamentos por partida |
|---|---|
| Original (sempre sobe) | 18,97 (± 0,16) |
| Escada com 50% de chance | **22,37 (± 0,17)** |

As escadas são atalhos. Quando elas passam a falhar metade das vezes, a partida fica **3,4 lançamentos mais longa (+18%)**.

### Pergunta 4: casa inicial que equilibra o jogo

Testei como casa inicial do Jogador 2 todas as casas de 1 a 35, menos as especiais. Com 10.000 partidas cada, a casa mais próxima de 50% foi a 7. Como as casas vizinhas podiam estar empatadas dentro da margem de ±1 p.p., simulei de novo as casas a até 2 p.p. de 50%, com 200.000 partidas e outra semente:

| Casa inicial do J2 | P(J1 vence) |
|---|---|
| 6 | 51,98% (± 0,22) |
| **7** | **49,64% (± 0,22)** |
| 8 | 48,61% (± 0,22) |

**Resposta: casa 7.** Com ela, o Jogador 1 vence 49,6% das partidas e o Jogador 2, 50,4%.

Uma observação: a curva não sobe de forma suave, então não daria para achar a resposta por busca binária. Começar na casa 2 ajuda muito pouco, porque é só uma casa de vantagem. Começar na 4 não ajuda nada (53,5% ± 1,0, contra 52,3% ± 1,0 na casa 1), porque quem está no 4 já passou da escada 3 → 16, a melhor do início do tabuleiro. A tabela completa está em `resultados.txt`.

### Pergunta 5: imunidade à primeira cobra

| Regra | P(J1 vence) |
|---|---|
| Original | 52,3% (± 1,0) |
| J2 imune à primeira cobra | **38,9% (± 1,0)** |

A imunidade inverte a vantagem com folga: o Jogador 1 perde 13,4 p.p., um efeito muito maior que o de jogar primeiro. Faz sentido. Cada jogador cai em cerca de 1,5 cobra por partida, e 4 das 5 cobras fazem perder 10 casas ou mais. Escapar de uma delas vale muito.


