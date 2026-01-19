import matplotlib
matplotlib.use('Agg')  # Backend não-interativo

import pandas as pd
import matplotlib.pyplot as plt
import os
import sys

# --- CONFIGURAÇÃO ---
RESULTS_DIR = "./results/"
OUTPUT_DIR = "./results/charts/"

def get_col_name(df, candidates):
    """Retorna o primeiro nome de coluna da lista 'candidates' que existe no df."""
    for col in candidates:
        if col in df.columns:
            return col
    return None

def generate_dashboard(instance_name):
    csv_path = os.path.join(RESULTS_DIR, instance_name, "history_ga.csv")

    if not os.path.exists(csv_path):
        print(f"Skipping {instance_name}: history_ga.csv not found.")
        return

    try:
        df = pd.read_csv(csv_path)
    except Exception as e:
        print(f"Error reading csv for {instance_name}: {e}")
        return

    # Mapeamento de colunas flexível
    col_gen = get_col_name(df, ['Gen', 'Generation'])
    col_best = get_col_name(df, ['Best', 'BestCost'])
    col_avg = get_col_name(df, ['Avg', 'AvgCost'])
    col_std = get_col_name(df, ['StdDev', 'CostStdDev', 'StdDevCost'])
    col_unique = get_col_name(df, ['UniqueRatio', 'Unique'])
    col_improv = get_col_name(df, ['Improv', 'LS_Improv'])
    col_open = get_col_name(df, ['Open', 'AvgOpen', 'AvgOpenFacilities'])

    # Criar figura
    fig, axes = plt.subplots(2, 2, figsize=(16, 10))
    fig.suptitle(f'Deep Analysis: {instance_name}', fontsize=16, fontweight='bold')

    # --- 1. CONVERGÊNCIA (Top Left) ---
    ax1 = axes[0, 0]
    if col_best and col_avg:
        ax1.plot(df[col_gen], df[col_best], label='Best Cost', color='#1f77b4', linewidth=2)
        ax1.plot(df[col_gen], df[col_avg], label='Avg Cost', color='#ff7f0e', linestyle='--', alpha=0.7)
    ax1.set_title("Convergence & Selective Pressure")
    ax1.set_xlabel("Generation")
    ax1.set_ylabel("Cost")
    ax1.legend()
    ax1.grid(True, alpha=0.3)

    # --- 2. DIVERSIDADE (Top Right) - Eixo Duplo ---
    ax2 = axes[0, 1]
    lines = []
    
    # Eixo Primário: Desvio Padrão (Diversidade Fenotípica)
    if col_std:
        color = 'tab:red'
        ax2.set_ylabel('Cost Std Dev (Phenotypic)', color=color)
        l1 = ax2.plot(df[col_gen], df[col_std], color=color, label='Std Dev', alpha=0.8)
        ax2.tick_params(axis='y', labelcolor=color)
        lines += l1

    # Eixo Secundário: Unique Ratio (Diversidade Genotípica)
    if col_unique:
        ax2b = ax2.twinx()
        color = 'tab:green'
        ax2b.set_ylabel('Unique Ratio (Genotypic)', color=color)
        l2 = ax2b.plot(df[col_gen], df[col_unique], color=color, linestyle=':', label='Unique Ratio', linewidth=2)
        ax2b.tick_params(axis='y', labelcolor=color)
        ax2b.set_ylim(0, 1.1)
        lines += l2

    if lines:
        labels = [l.get_label() for l in lines]
        ax2.legend(lines, labels, loc='upper right')
    ax2.set_title("Diversity Dynamics (Adaptation & Cataclysm)")
    ax2.grid(True, alpha=0.3)

    # --- 3. ATIVIDADE DA BUSCA LOCAL (Bottom Left) ---
    ax3 = axes[1, 0]
    if col_improv:
        ax3.bar(df[col_gen], df[col_improv], color='#9467bd', alpha=0.6, label='Improvements')
        # Média móvel
        if len(df) > 10:
            ma = df[col_improv].rolling(window=5).mean()
            ax3.plot(df[col_gen], ma, color='purple', linewidth=2, label='Moving Avg (5)')
    ax3.set_title("Local Search Intensity")
    ax3.set_xlabel("Generation")
    ax3.set_ylabel("Number of Improvements")
    ax3.legend()
    ax3.grid(True, alpha=0.3)

    # --- 4. ESTRUTURA DA SOLUÇÃO (Bottom Right) ---
    ax4 = axes[1, 1]
    if col_open:
        ax4.plot(df[col_gen], df[col_open], color='#2ca02c', linewidth=2)
        ax4.set_title("Phenotypic Evolution (Avg Open Facilities)")
        ax4.set_xlabel("Generation")
        ax4.set_ylabel("Count of Open Facilities")
        ax4.grid(True, alpha=0.3)

    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    output_path = os.path.join(OUTPUT_DIR, f"dashboard_{instance_name}.png")
    plt.savefig(output_path, dpi=150)
    plt.close()
    print(f"Generated dashboard: {output_path}")

def generate_all():
    instances = [d for d in os.listdir(RESULTS_DIR) 
                 if (os.path.isdir(os.path.join(RESULTS_DIR, d)) 
                     and d not in ["charts", "paper_replication", "comparative_analysis"])]
    instances.sort()
    
    if not instances:
        print("No instances found.")
        return

    print(f"Generating dashboards for {len(instances)} instances...")
    for instance in instances:
        generate_dashboard(instance)

if __name__ == "__main__":
    if(len(sys.argv) > 1):
        instance = sys.argv[1]
        generate_dashboard(instance)
    else:
        generate_all()