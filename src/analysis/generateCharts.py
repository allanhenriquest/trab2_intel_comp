import pandas as pd
import matplotlib.pyplot as plt
import os
import math
import sys

RESULTS_DIR = "./results/"
OUTPUT_DIR = "./results/charts/"

def generate_chart(instance_name):
    csv_path = os.path.join(RESULTS_DIR, instance_name, "history_ga.csv")

    df = pd.read_csv(csv_path)

    plt.figure(figsize=(10, 6))
    plt.plot(df['Gen'], df['Best'], label='Best individual', color='blue')
    plt.plot(df['Gen'], df['Avg'], label='Population average', color='orange')
    plt.xlabel('Generation')
    plt.ylabel('Cost')
    plt.title(f'Genetic Algorithm Progress for {instance_name}')
    plt.legend()
    plt.grid(True)
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    plt.savefig(os.path.join(OUTPUT_DIR, f"{instance_name}_ga_progress.png"))
    plt.close()

def generate_all_charts_combined():
    instances = [d for d in os.listdir(RESULTS_DIR) 
                if (os.path.isdir(os.path.join(RESULTS_DIR, d)) and d != "charts")]
    
    if not instances:
        print("No instances found in results directory")
        return
    
    instances.sort()
    
    # Calculate grid dimensions
    num_instances = len(instances)
    cols = 3
    rows = math.ceil(num_instances / cols)
    
    # Create figure with subplots
    fig, axes = plt.subplots(rows, cols, figsize=(18, 5*rows))
    
    # Flatten axes array for easier iteration
    if rows == 1 and cols == 1:
        axes = [axes]
    elif rows == 1 or cols == 1:
        axes = axes.flatten()
    else:
        axes = axes.flatten()
    
    # Plot each instance
    for idx, instance_name in enumerate(instances):
        csv_path = os.path.join(RESULTS_DIR, instance_name, "history_ga.csv")
        
        if not os.path.exists(csv_path):
            print(f"Warning: {csv_path} not found, skipping {instance_name}")
            continue
        
        df = pd.read_csv(csv_path)
        ax = axes[idx]
        
        ax.plot(df['Gen'], df['Best'], label='Best individual', color='blue', linewidth=2)
        ax.plot(df['Gen'], df['Avg'], label='Population average', color='orange', linewidth=2)
        ax.set_xlabel('Generation', fontsize=10)
        ax.set_ylabel('Cost', fontsize=10)
        ax.set_title(f'{instance_name}', fontsize=11, fontweight='bold')
        ax.legend(fontsize=9)
        ax.grid(True, alpha=0.3)
    
    # Hide unused subplots
    for idx in range(num_instances, len(axes)):
        axes[idx].set_visible(False)
    
    plt.tight_layout()
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    output_path = os.path.join(OUTPUT_DIR, "all_instances_ga_progress.png")
    plt.savefig(output_path, dpi=150, bbox_inches='tight')
    print(f"Chart saved to {output_path}")
    plt.close()

if __name__ == "__main__":
    if(len(sys.argv) > 1):
        instance = sys.argv[1]
        print("Generating chart for instance:", instance)
        generate_chart(instance)
    else:
        print("Generating combined chart for all instances")
        generate_all_charts_combined()