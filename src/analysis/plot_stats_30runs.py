import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import os
import sys
import subprocess
import argparse

# Configuração
CSV_FILE = "results/stats_raw_30runs.csv"
OUTPUT_DIR = "results/stats_plots/"

def run_cpp_solver(executable, instances_dir):
    print(f">>> Executing C++ Solver in STATS mode (30 runs)...")
    cmd = [executable, "-all", instances_dir, "--stats", "--k", "10"] # Default k=10 for stats
    subprocess.run(cmd, check=True)

def generate_charts():
    if not os.path.exists(CSV_FILE):
        print(f"Error: {CSV_FILE} not found.")
        return

    df = pd.read_csv(CSV_FILE)
    
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)

    # 1. Tabela de Estatísticas Descritivas
    # Agrupa por instância e calcula estatísticas sobre o GAP OBS
    stats = df.groupby('Instance')['Gap_OBS(%)'].agg(['mean', 'std', 'min', 'max', 'count'])
    stats['std'] = stats['std'].fillna(0) # Caso só tenha 1 run
    
    stats_file = os.path.join(OUTPUT_DIR, "descriptive_statistics.csv")
    stats.to_csv(stats_file)
    print(f">>> Descriptive Stats saved to {stats_file}")
    print(stats.head())

    # 2. Boxplot de Estabilidade (Distribuição dos Gaps nas 30 execuções)
    plt.figure(figsize=(15, 8))
    sns.set_style("whitegrid")
    
    # Ordenar instâncias por tamanho/nome para o gráfico ficar bonito
    df['Size'] = df['Instance'].apply(lambda x: int(x.split('-')[0]) if '-' in x else 0)
    df = df.sort_values(by=['Size', 'Instance'])

    sns.boxplot(x='Instance', y='Gap_OBS(%)', data=df, palette="viridis")
    plt.title("Stability Analysis: Gap Distribution over 30 Independent Runs (k=10)", fontsize=16)
    plt.ylabel("Gap w.r.t BKS (%)")
    plt.xlabel("Instance")
    plt.xticks(rotation=45)
    plt.tight_layout()
    
    plot_file = os.path.join(OUTPUT_DIR, "boxplot_stability_30runs.png")
    plt.savefig(plot_file, dpi=300)
    print(f">>> Boxplot saved to {plot_file}")

    # 3. Gráfico de Violino (Densidade + Boxplot) - opcional, mas bonito para papers
    plt.figure(figsize=(15, 8))
    sns.violinplot(x='Instance', y='Gap_OBS(%)', data=df, inner="quartile", palette="muted")
    plt.title("Density of Solutions over 30 Runs", fontsize=16)
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, "violin_stability.png"), dpi=300)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--run-cpp", help="Path to C++ executable")
    parser.add_argument("--dir", help="Instances directory")
    args = parser.parse_args()

    if args.run_cpp and args.dir:
        run_cpp_solver(args.run_cpp, args.dir)
    
    generate_charts()