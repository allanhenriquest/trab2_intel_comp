#!/bin/bash

# 1. Compilar o gerador (caso ainda não esteja compilado)
# Verifica se o compilador g++ está disponível
if ! command -v g++ &> /dev/null; then
    echo "Erro: g++ não encontrado. Instale o compilador C++."
    exit 1
fi

echo "Compilando o gerador..."
g++ kmed.cpp -o kmgen

# Verifica se a compilação deu certo
if [ ! -f "./kmgen" ]; then
    echo "Erro na compilação!"
    exit 1
fi

echo "Compilação concluída com sucesso."
echo "----------------------------------------"

# 2. Definição dos parâmetros conforme o artigo
# Tamanhos (Número de instalações/clientes) 
SIZES=(500 1000 1500 2000 2500 3000)

# Esquemas de custo (Sufixos 10, 100, 1000) 
# O código C++ usa esse valor como denominador no cálculo do custo de abertura.
DENOMS=(10 100 1000)

# Flag para métrica Euclidiana (1 = true)
METRIC=1

# Cria uma pasta para organizar os outputs
mkdir -p instancias_MED

# 3. Loop para gerar todas as 18 combinações
for S in "${SIZES[@]}"; do
    for D in "${DENOMS[@]}"; do
        # Define o nome do arquivo no padrão X-Y (Ex: 500-10)
        FILENAME="instancias_MED/${S}-${D}.txt"
        
        echo "Gerando instância: Tamanho $S, Esquema $D -> $FILENAME"
        
        # Executa o gerador: ./kmgen [LOCATIONS] [DENOM] [METRIC] [OUTPUT_FILE]
        ./kmgen $S $D $METRIC $FILENAME
    done
done

echo "----------------------------------------"
echo "Todas as 18 instâncias foram geradas na pasta 'instancias_MED'."