import os
import math
from PIL import Image

# --- CONFIGURAÇÃO ---
INPUT_DIR = "./results/comparative_analysis/"
OUTPUT_FILE = "./results/comparative_analysis/global_summary_grid.png"
COLUMNS = 3  # Número de colunas no grid (3 é bom para visualização em monitor)

def combine_images():
    # 1. Encontrar todas as imagens PNG
    if not os.path.exists(INPUT_DIR):
        print(f"Directory not found: {INPUT_DIR}")
        return

    # Pega todos os pngs, exceto o próprio arquivo de saída (para evitar recursão se rodar 2x)
    image_files = [f for f in os.listdir(INPUT_DIR) 
                   if f.endswith(".png") and "global_summary" not in f]
    
    # Ordenar para que fiquem em ordem alfabética (ex: 1000-10, 1000-100...)
    image_files.sort()

    if not image_files:
        print("No PNG images found to combine.")
        return

    print(f"Found {len(image_files)} images. Combining...")

    # 2. Carregar imagens e pegar dimensões
    images = []
    for file in image_files:
        path = os.path.join(INPUT_DIR, file)
        try:
            img = Image.open(path)
            images.append(img)
        except Exception as e:
            print(f"Error loading {file}: {e}")

    if not images:
        return

    # Assume que todas têm o mesmo tamanho da primeira (padrão do matplotlib)
    # Se não tiverem, redimensiona
    width, height = images[0].size
    
    # 3. Calcular tamanho do Grid
    num_images = len(images)
    num_cols = COLUMNS
    num_rows = math.ceil(num_images / num_cols)

    total_width = num_cols * width
    total_height = num_rows * height

    # 4. Criar Canvas em Branco
    print(f"Creating canvas: {total_width}x{total_height} pixels")
    combined_image = Image.new('RGB', (total_width, total_height), color=(255, 255, 255))

    # 5. Colar imagens no Canvas
    for index, img in enumerate(images):
        # Calcular posição (x, y)
        col = index % num_cols
        row = index // num_cols
        
        x = col * width
        y = row * height
        
        # Se a imagem tiver tamanho diferente, redimensionar (segurança)
        if img.size != (width, height):
            img = img.resize((width, height))
            
        combined_image.paste(img, (x, y))
        print(f"  Placed {image_files[index]} at ({row},{col})")

    # 6. Salvar
    combined_image.save(OUTPUT_FILE)
    print(f"\nSuccessfully saved combined image to:\n{OUTPUT_FILE}")

if __name__ == "__main__":
    combine_images()