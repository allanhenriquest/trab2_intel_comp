import matplotlib
matplotlib.use('Agg') # <--- CORREÇÃO DO ERRO QT/XCB (Define backend não-interativo)

import os
import subprocess
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import sys

# --- CONFIGURAÇÃO ---
EXECUTABLE = "./build/uflp"
INSTANCES_DIR = "./instancias_MED" 
OUTPUT_DIR = "./results/paper_replication/"
K_LEVELS = [5, 10, 20] # Níveis de Incerteza do Artigo

def ensure_dir(path):
    if not os.path.exists(path):
        os.makedirs(path)

def get_instance_suffix(instance_name):
    """
    Extrai o sufixo da instância para separar os gráficos conforme o artigo.
    Ex: '500-10' -> '10', '2000-100' -> '100'
    """
    try:
        parts = instance_name.split('-')
        if len(parts) >= 2:
            return parts[1]
        return "unknown"
    except:
        return "unknown"

def run_experiment():
    ensure_dir(OUTPUT_DIR)
    
    if not os.path.exists(EXECUTABLE):
        print(f"ERRO: Executável não encontrado em {EXECUTABLE}")
        return

    dfs = []

    # 1. LOOP DE EXECUÇÃO (Varia K: 5, 10, 20)
    for k in K_LEVELS:
        print(f"\n>>> [EXPERIMENTO] Rodando Solver para k={k} (Incerteza)...")
        
        # Chama o C++ em modo batch com samples reduzidos para o SA (busca rápida)
        # O C++ fará a validação final com 100k samples automaticamente
        cmd = [EXECUTABLE, "-all", INSTANCES_DIR, "--k", str(k), "--samples", "100"]
        
        try:
            subprocess.run(cmd, check=True)
        except subprocess.CalledProcessError as e:
            print(f"Erro ao executar o solver: {e}")
            return

        # Lê o resultado gerado (summary.csv)
        summary_path = "results/summary.csv"
        if not os.path.exists(summary_path):
            print("ERRO: summary.csv não foi gerado.")
            continue
            
        # Carrega e marca o nível de K
        try:
            df = pd.read_csv(summary_path)
            df['k'] = k  
            dfs.append(df)
        except Exception as e:
            print(f"Erro ao ler CSV: {e}")

    if not dfs:
        print("Nenhum dado foi coletado.")
        return

    # 2. CONSOLIDAÇÃO DOS DADOS
    full_df = pd.concat(dfs, ignore_index=True)
    
    # Salva tabela bruta consolidada para segurança
    full_df.to_csv(os.path.join(OUTPUT_DIR, "raw_consolidated_results.csv"), index=False)
    print(f"\n>>> Dados brutos salvos em {OUTPUT_DIR}raw_consolidated_results.csv")

    # 3. GERAR TABELA ESTILO 'TABLE 6' (Pivot)
    # Queremos colunas: Instance | BKS | OBD | OBD-S_k5 | OBS_k5 | OBD-S_k10 | OBS_k10 ...
    
    # Base: Dados determinísticos (pegamos do primeiro k, pois não mudam)
    base_cols = ['Instance', 'Best', 'OBD'] # Best = BKS
    pivot_base = full_df[full_df['k'] == K_LEVELS[0]][base_cols].copy()
    
    # Adiciona colunas dinâmicas para cada K
    for k in K_LEVELS:
        k_data = full_df[full_df['k'] == k][['Instance', 'OBD_S', 'OBS']]
        k_data = k_data.rename(columns={
            'OBD_S': f'OBD-S_k{k}',
            'OBS': f'OBS_k{k}'
        })
        pivot_base = pd.merge(pivot_base, k_data, on='Instance')

    # Calcula Gaps médios para o console
    print("\n=== RESUMO COMPARATIVO (Médias) ===")
    for k in K_LEVELS:
        # Gap relativo ao Determinístico Simulado (Quanto melhoramos em relação ao OBD?)
        improvement = ((pivot_base[f'OBD-S_k{k}'] - pivot_base[f'OBS_k{k}']) / pivot_base[f'OBD-S_k{k}']) * 100
        print(f"Incerteza k={k}: OBS melhorou o custo em média {improvement.mean():.2f}% sobre OBD-S")

    # Salva Tabela Final
    table_path = os.path.join(OUTPUT_DIR, "Table6_Replication.csv")
    pivot_base.to_csv(table_path, index=False)
    print(f">>> Tabela comparativa salva em {table_path}")

    # 4. GERAR GRÁFICOS (BOXPLOTS) - Figuras 7, 8 e 9
    generate_boxplots(full_df)

def generate_boxplots(df):
    """
    Gera 3 gráficos separados por tipo de instância (10, 100, 1000),
    comparando os Gaps em relação ao BKS.
    """
    print("\n>>> Gerando gráficos comparativos...")
    
    plot_data = []
    
    for _, row in df.iterrows():
        bks = row['Best']
        k = row['k']
        suffix = get_instance_suffix(row['Instance'])
        
        if bks <= 0: continue

        # Cálculo dos Gaps (%) em relação ao BKS
        gap_obd_s = (row['OBD_S'] - bks) / bks * 100.0
        gap_obs = (row['OBS'] - bks) / bks * 100.0
        
        # Adiciona entradas para o plot no formato Long
        # Nota: O artigo plota OBD-S e OBS lado a lado para cada K
        plot_data.append({
            'Suffix': suffix, 'Gap': gap_obd_s, 'Scenario': f'OBD-S k={k}', 'Type': 'OBD-S', 'K': k
        })
        plot_data.append({
            'Suffix': suffix, 'Gap': gap_obs, 'Scenario': f'OBS k={k}', 'Type': 'OBS', 'K': k
        })
        
        # Adiciona o OBD puro (apenas para k=5 para não poluir, ou repetido se preferir igual ao artigo)
        if k == 5:
             gap_obd = (row['OBD'] - bks) / bks * 100.0
             plot_data.append({'Suffix': suffix, 'Gap': gap_obd, 'Scenario': 'OBD', 'Type': 'Deterministic', 'K': 0})

    plot_df = pd.DataFrame(plot_data)

    # Configuração visual
    sns.set_style("whitegrid")
    suffixes = ['10', '100', '1000']
    
    # Ordem de plotagem no eixo X
    order_list = ['OBD']
    for k in K_LEVELS:
        order_list.append(f'OBD-S k={k}')
        order_list.append(f'OBS k={k}')

    for suf in suffixes:
        subset = plot_df[plot_df['Suffix'] == suf]
        
        if subset.empty:
            continue
            
        plt.figure(figsize=(14, 7))
        
        # Cria o Boxplot
        ax = sns.boxplot(x='Scenario', y='Gap', data=subset, order=order_list, palette="Set2", showfliers=False)
        
        # Títulos e Labels
        plt.title(f"Gaps w.r.t BKS (Opening Cost Scheme: sqrt(n)/{suf})", fontsize=16)
        plt.ylabel("Gap w.r.t BKS (%)", fontsize=12)
        plt.xlabel("Strategy & Uncertainty Level", fontsize=12)
        plt.xticks(rotation=45)
        plt.grid(True, linestyle='--', alpha=0.6)
        
        filename = os.path.join(OUTPUT_DIR, f"Figure_Replica_Suffix_{suf}.png")
        plt.tight_layout()
        plt.savefig(filename, dpi=300)
        plt.close()
        print(f"  -> Gráfico salvo: {filename}")

if __name__ == "__main__":
    run_experiment()