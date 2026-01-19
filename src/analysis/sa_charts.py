import matplotlib
matplotlib.use('Agg') # Backend não-interativo (Essencial para servidores/WSL)

import pandas as pd
import matplotlib.pyplot as plt
import os
import sys
import glob

# --- CONFIGURAÇÃO ---
RESULTS_DIR = "./results/"
OUTPUT_DIR = "./results/charts_sa/"

def ensure_dir(path):
    if not os.path.exists(path):
        os.makedirs(path)

def plot_single_run_dashboard(instance_name, run_id, df):
    """
    Gera um dashboard detalhado para uma única execução do SA (Run ID).
    Analisa: Cooling Schedule, Aceitação de Pioras e Convergência.
    """
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10), sharex=True)
    
    # Título
    fig.suptitle(f'SA Diagnostics: {instance_name} (Run {run_id})', fontsize=16, fontweight='bold')

    # --- 1. EVOLUÇÃO DO CUSTO (Top) ---
    # Plota a linha contínua do custo atual
    ax1.plot(df['Iter'], df['CurrentCost'], color='lightgray', linewidth=1, label='Current Cost', zorder=1)
    
    # Destaca o Melhor Custo encontrado (BestCost)
    ax1.plot(df['Iter'], df['BestCost'], color='#1f77b4', linewidth=2, label='Best Found', zorder=3)

    # Destaca movimentos de PIORA que foram ACEITES (Critério de Metropolis)
    # Filtra: Aceite=True E CurrentCost > Anterior (aproximado)
    # Uma forma mais simples é filtrar onde Accepted=1 e Cost > BestCost (ou média local)
    # Aqui vamos plotar todos os pontos aceites para ver a densidade
    accepted = df[df['Accepted'] == 1]
    worsening = accepted[accepted['CurrentCost'] > accepted['BestCost']] # Aproximação visual
    
    if not worsening.empty:
        ax1.scatter(worsening['Iter'], worsening['CurrentCost'], 
                   color='red', s=10, alpha=0.5, label='Accepted Worsening', zorder=2)

    ax1.set_ylabel('Expected Cost')
    ax1.set_title('Search Trajectory & Metropolis Acceptance')
    ax1.legend(loc='upper right')
    ax1.grid(True, alpha=0.3)

    # --- 2. COOLING SCHEDULE (Bottom) ---
    color = 'tab:orange'
    ax2.set_xlabel('Iteration')
    ax2.set_ylabel('Temperature', color=color)
    ax2.plot(df['Iter'], df['Temp'], color=color, linewidth=2, label='Temperature')
    ax2.tick_params(axis='y', labelcolor=color)
    ax2.set_yscale('log') # Escala logarítmica é melhor para ver o arrefecimento
    ax2.grid(True, alpha=0.3, which="both")
    ax2.set_title('Temperature Decay (Log Scale)')

    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    
    filename = os.path.join(OUTPUT_DIR, f"{instance_name}_sa_run{run_id}_dashboard.png")
    plt.savefig(filename, dpi=150)
    plt.close()
    print(f"  -> Dashboard salvo: {filename}")

def plot_multi_run_convergence(instance_name, all_runs):
    """
    Plota a convergência de TODAS as execuções do SA para esta instância.
    Ajuda a ver se o método é estável (todos convergem para o mesmo ponto).
    """
    plt.figure(figsize=(12, 7))
    
    for run_id, df in all_runs.items():
        # Plota apenas o BestCost para ficar limpo
        plt.plot(df['Iter'], df['BestCost'], alpha=0.6, linewidth=1.5, label=f'Run {run_id}')

    plt.title(f'Robustness Convergence Analysis: {instance_name} ({len(all_runs)} runs)', fontsize=14)
    plt.xlabel('Iteration')
    plt.ylabel('Best Expected Cost')
    plt.grid(True, alpha=0.3)
    
    # Se houver muitas runs, não mostrar legenda de todas
    if len(all_runs) <= 10:
        plt.legend()
    
    filename = os.path.join(OUTPUT_DIR, f"{instance_name}_sa_ALL_runs_convergence.png")
    plt.savefig(filename, dpi=150)
    plt.close()
    print(f"  -> Convergência Global salva: {filename}")

def process_instance(instance_name):
    instance_dir = os.path.join(RESULTS_DIR, instance_name)
    
    # Encontrar todos os arquivos history_sa_run*.csv
    pattern = os.path.join(instance_dir, "history_sa_run*.csv")
    files = glob.glob(pattern)
    
    if not files:
        # Silencioso se não houver SA rodado para esta instância
        return

    print(f"Processando SA para: {instance_name} ({len(files)} execuções encontradas)")
    
    all_runs_data = {}

    for csv_file in files:
        try:
            # Extrair Run ID do nome do arquivo
            filename = os.path.basename(csv_file)
            # Ex: history_sa_run0.csv -> 0
            run_id = filename.replace("history_sa_run", "").replace(".csv", "")
            
            df = pd.read_csv(csv_file)
            
            # Guardar para o plot coletivo
            all_runs_data[run_id] = df
            
            # Gerar dashboard individual apenas para a primeira run (para não encher o disco)
            # Ou para todas se forem poucas. Vamos fazer para a Run 0 sempre.
            if run_id == "0":
                plot_single_run_dashboard(instance_name, run_id, df)
                
        except Exception as e:
            print(f"Erro ao ler {csv_file}: {e}")

    # Gerar plot coletivo se houver mais de uma run
    if len(all_runs_data) > 0:
        plot_multi_run_convergence(instance_name, all_runs_data)

def main():
    ensure_dir(OUTPUT_DIR)
    
    # Listar todas as instâncias em results
    if not os.path.exists(RESULTS_DIR):
        print(f"Diretório {RESULTS_DIR} não encontrado.")
        return

    instances = [d for d in os.listdir(RESULTS_DIR) if os.path.isdir(os.path.join(RESULTS_DIR, d))]
    instances.sort()

    for inst in instances:
        if inst in ["charts", "charts_sa", "paper_replication", "comparative_analysis"]:
            continue
        process_instance(inst)

if __name__ == "__main__":
    if len(sys.argv) > 1:
        process_instance(sys.argv[1])
    else:
        main()