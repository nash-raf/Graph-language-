import os
import shutil
from pathlib import Path

# Source directory (where .txt files are)
source_dir = "synth_graphs_weighted/"

# Destination directory (where .wel files will be created)
dest_dir = "synth_graphs_weighted_graphit/"

def convert_txt_to_wel():
    """
    Converts all .txt files from synth_graphs_weighted/ to .wel files
    in synth_graphs_weighted_graphit/ directory
    """
    # Create destination directory if it doesn't exist
    os.makedirs(dest_dir, exist_ok=True)
    
    # Get all .txt files from source directory
    source_path = Path(source_dir)
    
    if not source_path.exists():
        print(f"Error: Source directory '{source_dir}' does not exist!")
        return
    
    txt_files = list(source_path.glob("*.txt"))
    
    if not txt_files:
        print(f"No .txt files found in '{source_dir}'")
        return
    
    print(f"Found {len(txt_files)} .txt files to convert")
    print(f"Source: {source_dir}")
    print(f"Destination: {dest_dir}\n")
    
    converted = 0
    skipped = 0
    
    for txt_file in txt_files:
        # Get the base name without extension
        base_name = txt_file.stem
        
        # Create .wel filename
        wel_filename = f"{base_name}.wel"
        dest_path = Path(dest_dir) / wel_filename
        
        # Check if .wel file already exists
        if dest_path.exists():
            print(f"  Skipping {txt_file.name} (already exists: {wel_filename})")
            skipped += 1
            continue
        
        # Copy file and rename extension
        try:
            shutil.copy2(txt_file, dest_path)
            converted += 1
            if converted % 5 == 0:  # Print progress every 5 files
                print(f"  Converted {converted} files...")
        except Exception as e:
            print(f"  Error converting {txt_file.name}: {e}")
    
    print(f"\nConversion complete!")
    print(f"  Converted: {converted} files")
    print(f"  Skipped: {skipped} files")
    print(f"  Total: {len(txt_files)} files")

if __name__ == "__main__":
    convert_txt_to_wel()

