import matplotlib
matplotlib.use('Agg') # Backend não-interativo

import pandas as pd
import matplotlib.pyplot as plt
import os
import sys

# --- CONFIGURAÇÃO ---
RESULTS_DIR = "./results/"
OUTPUT_DIR = "./results/comparative_analysis/"

# Mapa de estilos (Mantido)
STYLE_MAP = {
    "LS-OFF_SL-OFF": {"color": "#e74c3c", "style": ":", "label": "Basal (No LS, No SL)"},
    "LS-ON_SL-OFF":  {"color": "#3498db", "style": "--", "label": "Local Search Only"},
    "LS-OFF_SL-ON":  {"color": "#f1c40f", "style": "-.", "label": "Smart Leader Only"},
    "LS-ON_SL-ON":   {"color": "#2ecc71", "style": "-",  "label": "Full (LS + SL)"}
}

def get_config_key(folder_name):
    ls = "LS-ON" if "LS-ON" in folder_name else "LS-OFF"
    sl = "SL-ON" if "SL-ON" in folder_name else "SL-OFF"
    return f"{ls}_{sl}"

def analyze_instance(instance_name):
    instance_path = os.path.join(RESULTS_DIR, instance_name)
    if not os.path.isdir(instance_path):
        return None

    subfolders = [f for f in os.listdir(instance_path) if os.path.isdir(os.path.join(instance_path, f))]
    
    # Criar figura com 2 subplots (Custo e Diversidade)
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(18, 7))
    
    has_data = False

    for sub in subfolders:
        csv_path = os.path.join(instance_path, sub, "ga_metrics.csv")
        if not os.path.exists(csv_path):
            continue

        try:
            df = pd.read_csv(csv_path)
            if df.empty: continue

            config_key = get_config_key(sub)
            style = STYLE_MAP.get(config_key, {"color": "gray", "style": "-", "label": sub})
            
            # --- GRÁFICO 1: Convergência (Melhor Custo) ---
            ax1.plot(df['Generation'], df['BestCost'], 
                     label=style['label'], color=style['color'], linestyle=style['style'], linewidth=2, alpha=0.8)
            
            # --- GRÁFICO 2: Evolução da Diversidade (StdDev) ---
            # Verifica se a coluna nova existe (para compatibilidade)
            if 'StdDevCost' in df.columns:
                ax2.plot(df['Generation'], df['StdDevCost'], 
                         label=style['label'], color=style['color'], linestyle=style['style'], linewidth=2, alpha=0.8)
            
            has_data = True

        except Exception as e:
            print(f"Error reading {csv_path}: {e}")

    if has_data:
        # Configuração Gráfico 1 (Custo)
        ax1.set_title(f'Convergence: {instance_name}', fontsize=12, fontweight='bold')
        ax1.set_xlabel('Generation')
        ax1.set_ylabel('Best Cost')
        ax1.legend()
        ax1.grid(True, alpha=0.3)

        # Configuração Gráfico 2 (Diversidade)
        ax2.set_title(f'Diversity Evolution (Std Dev): {instance_name}', fontsize=12, fontweight='bold')
        ax2.set_xlabel('Generation')
        ax2.set_ylabel('Cost Standard Deviation')
        ax2.legend()
        ax2.grid(True, alpha=0.3)
        
        plt.tight_layout()
        
        # Salvar
        out_file = os.path.join(OUTPUT_DIR, f"analysis_{instance_name}.png")
        plt.savefig(out_file, dpi=150)
        plt.close()
        print(f"Saved analysis chart: {out_file}")
        return True
    else:
        plt.close()
        return False

def main():
    if not os.path.exists(RESULTS_DIR):
        print(f"Error: {RESULTS_DIR} not found.")
        return

    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    instances = [d for d in os.listdir(RESULTS_DIR) if os.path.isdir(os.path.join(RESULTS_DIR, d)) and d != "comparative_analysis"]
    instances.sort()

    print(f"Analyzing {len(instances)} instances...")

    for inst in instances:
        analyze_instance(inst)

if __name__ == "__main__":
    main()