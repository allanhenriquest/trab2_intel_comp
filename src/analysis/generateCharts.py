import pandas as pd
import matplotlib.pyplot as plt
import os

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

if __name__ == "__main__":
    instances = [d for d in os.listdir(RESULTS_DIR) 
                if (os.path.isdir(os.path.join(RESULTS_DIR, d)) and d != "charts")]
    for instance in instances:
        generate_chart(instance)