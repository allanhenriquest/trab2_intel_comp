import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import os
import sys
import subprocess
import argparse

# Configuração
OUTPUT_DIR = "results/stats_plots/"
RAW_CSV = "results/stats_raw_30runs_combined.csv"

def run_cpp_solver(executable, instances_dir):
    print(f">>> STARTING FULL STATISTICAL ANALYSIS (30 runs per k)...")
    
    # Lista para acumular todos os resultados antes de salvar
    all_dfs = []

    # Loop para rodar os 3 níveis de incerteza
    for k in [5, 10, 20]:
        print(f"\n>>> Running Batch for k={k}...")
        
        # Arquivo temporário para este k
        temp_csv = f"results/temp_stats_k{k}.csv"
        
        # Chama o C++ (o código C++ precisa ser compilado com a lógica de salvar no arquivo definido ou padrão)
        # Nota: Como o C++ atual salva fixo em "results/stats_raw_30runs.csv", 
        # nós vamos renomear esse arquivo após cada execução para não sobrescrever.
        
        cmd = [executable, "-all", instances_dir, "--stats", "--k", str(k)]
        subprocess.run(cmd, check=True)
        
        # Lê o arquivo gerado pelo C++
        default_out = "results/stats_raw_30runs.csv"
        if os.path.exists(default_out):
            df = pd.read_csv(default_out)
            df['k'] = k # Adiciona coluna k
            all_dfs.append(df)
            
            # (Opcional) Salva backup parcial
            df.to_csv(f"results/stats_raw_k{k}.csv", index=False)
        else:
            print(f"Error: Output for k={k} not found.")

    # Consolidar tudo
    if all_dfs:
        full_df = pd.concat(all_dfs, ignore_index=True)
        full_df.to_csv(RAW_CSV, index=False)
        print(f"\n>>> Combined Statistics saved to {RAW_CSV}")
        return full_df
    else:
        return None

def generate_charts(df=None):
    if df is None:
        if os.path.exists(RAW_CSV):
            df = pd.read_csv(RAW_CSV)
        else:
            print(f"Error: No data found in {RAW_CSV}")
            return

    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)

    # 1. Tabela de Estatísticas Descritivas (Agrupado por Instância e k)
    stats = df.groupby(['k', 'Instance'])['Gap_OBS(%)'].agg(['mean', 'std', 'min', 'max'])
    stats_file = os.path.join(OUTPUT_DIR, "descriptive_statistics_all_k.csv")
    stats.to_csv(stats_file)
    print(f">>> Descriptive Stats saved to {stats_file}")

    # 2. Boxplots separados por k
    # Vamos gerar um gráfico para cada nível de k para não ficar poluído
    sns.set_style("whitegrid")
    
    # Ordenar para visualização
    df['Size'] = df['Instance'].apply(lambda x: int(x.split('-')[0]) if '-' in x else 0)
    df = df.sort_values(by=['Size', 'Instance'])

    for k in [5, 10, 20]:
        subset = df[df['k'] == k]
        if subset.empty: continue

        plt.figure(figsize=(16, 8))
        sns.boxplot(x='Instance', y='Gap_OBS(%)', data=subset, palette="viridis")
        
        plt.title(f"Stability Analysis (30 Runs) - Uncertainty k={k}", fontsize=16)
        plt.ylabel("Gap w.r.t BKS (%)")
        plt.xlabel("Instance")
        plt.xticks(rotation=45)
        plt.tight_layout()
        
        plot_file = os.path.join(OUTPUT_DIR, f"boxplot_stability_k{k}.png")
        plt.savefig(plot_file, dpi=300)
        plt.close()
        print(f">>> Boxplot for k={k} saved.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--run-cpp", help="Path to C++ executable")
    parser.add_argument("--dir", help="Instances directory")
    args = parser.parse_args()

    df = None
    if args.run_cpp and args.dir:
        df = run_cpp_solver(args.run_cpp, args.dir)
    
    generate_charts(df)