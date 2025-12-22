# UFLP sob Incerteza -- Guia Completo do Projeto

## 1. Objetivo do Trabalho

Implementar e comparar soluções para o **Uncapacitated Facility Location
Problem under Uncertainty (UFLP-U)** utilizando: - um **algoritmo
populacional** (Algoritmo Genético -- GA); - um **algoritmo de busca
local** (Simulated Annealing -- SA); - considerando primeiro o **caso
determinístico** e depois o **caso estocástico**.

O foco é obter soluções **robustas** sob incerteza, mantendo o custo
computacional controlado.

------------------------------------------------------------------------

## 2. Descrição do Problema

### 2.1 Conjuntos

-   Facilities (instalações): ( i = 1, `\dots`{=tex}, n )
-   Clientes (cidades): ( j = 1, `\dots`{=tex}, m )

### 2.2 Parâmetros

-   ( f_i ): custo fixo de abrir a facility ( i )
-   ( c\_{ij} ): custo determinístico de atender o cliente ( j ) pela
    facility ( i )

### 2.3 Variáveis de decisão

-   ( y_i `\in `{=tex}{0,1} ): indica se a facility ( i ) está aberta

### 2.4 Função objetivo (determinística)

\[ `\min `{=tex}f(y) = `\sum`{=tex}*i f_i y_i + `\sum`{=tex}*j
`\min`{=tex}*{i:y_i=1} c*{ij} \]

Cada cliente é atendido pela facility aberta de menor custo.

------------------------------------------------------------------------

## 3. Estratégia Metodológica Geral

### 3.1 Motivação

-   Resolver o problema determinístico primeiro fornece boas soluções
    iniciais;
-   Permite definir corretamente os parâmetros de penalização no caso
    estocástico;
-   Reduz o espaço de busca para a fase sob incerteza.

### 3.2 Estrutura em Duas Fases

1.  **GA determinístico**: busca global por boas soluções.
2.  **SA estocástico**: refinamento local usando simulação de Monte
    Carlo.

Não existe GA estocástico.

------------------------------------------------------------------------

## 4. Estrutura das Instâncias

### 4.1 Estrutura interna (independente do formato de arquivo)

``` text
n : int
m : int

opening_costs : vector<double>          // tamanho n
allocation_costs : vector<vector<double>> // dimensão n x m
```

Capacidades e demandas são ignoradas (problema uncapacitated).

------------------------------------------------------------------------

## 5. Representação da Solução

``` text
y : vector<bool>                 // vetor binário de facilities abertas
assigned_facility : vector<int>  // facility que atende cada cliente
current_cost : vector<double>    // custo atual de atendimento por cliente
total_cost : double              // custo total da solução
```

Deve existir sempre **ao menos uma facility aberta**.

------------------------------------------------------------------------

## 6. Avaliação do Custo (Determinístico)

### 6.1 Avaliação inicial (completa)

-   Soma dos custos fixos das facilities abertas;
-   Para cada cliente, seleção da facility aberta de menor custo.

Complexidade: ( O(n `\cdot `{=tex}m) )

### 6.2 Avaliação incremental (essencial para SA)

#### Abrir uma facility k

-   Soma ( f_k ) ao custo total;
-   Para cada cliente:
    -   se ( c\_{kj} \< current_cost\[j\] ), atualizar.

Custo: ( O(m) )

#### Fechar uma facility k

-   Subtrai ( f_k );
-   Apenas clientes atendidos por k são reavaliados.

Custo médio: ( O(m) )

------------------------------------------------------------------------

## 7. Algoritmo Genético (GA) -- Fase Determinística

### 7.1 Representação

-   Cromossomo: vetor binário ( y )

### 7.2 Operadores

-   Seleção: torneio ou roleta;
-   Crossover: uniforme ou 1-ponto;
-   Mutação: flip de bits;
-   Elitismo: manter melhores soluções.

### 7.3 Avaliação

-   Apenas custo determinístico.

### 7.4 Saída

-   Conjunto elite de soluções determinísticas.

------------------------------------------------------------------------

## 8. Simulated Annealing (SA) -- Fase Estocástica

### 8.1 Ponto inicial

-   Soluções elite geradas pelo GA.

### 8.2 Vizinhança

-   Flip de uma facility (abrir ou fechar).

### 8.3 Avaliação estocástica

-   Custos ( C\_{ij} ) simulados via Monte Carlo;
-   Estima-se custo esperado + penalidades.

### 8.4 Aceitação

-   Critério clássico do SA: \[ P = `\exp`{=tex}(-`\Delta `{=tex}/ T) \]

### 8.5 Resfriamento

-   Temperatura inicial ( T_0 );
-   Fator ( `\alpha `{=tex}`\in `{=tex}(0,1) ).

------------------------------------------------------------------------

## 9. Simulação de Monte Carlo

-   Utilizada apenas no SA;
-   Estima:
    -   custo esperado;
    -   penalidades por violação;
-   Número de simulações pode variar com a temperatura.

------------------------------------------------------------------------

## 10. Fluxo Geral do Algoritmo

1.  Ler instância;
2.  Executar GA determinístico;
3.  Selecionar soluções elite;
4.  Para cada elite:
    -   executar SA estocástico;
5.  Simulação longa final;
6.  Selecionar solução robusta final.

------------------------------------------------------------------------

## 11. Boas Práticas de Implementação

-   Separar claramente:
    -   leitura da instância;
    -   representação da solução;
    -   avaliação de custo;
    -   metaheurísticas.
-   Usar avaliação incremental;
-   Evitar estruturas ordenadas dinâmicas;
-   Controlar custo computacional do Monte Carlo.

------------------------------------------------------------------------

## 12. Resultado Esperado

-   Implementação clara e modular em C++;
-   Comparação entre soluções determinísticas e estocásticas;
-   Solução final robusta sob incerteza;
-   Código facilmente extensível e compreensível.
