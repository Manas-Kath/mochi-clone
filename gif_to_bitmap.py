import os
import re
import io
from PIL import Image

# Path configuration
input_dir = 'backups/v2.0_GIF_Backup/DasaiOled'
output_dir = 'hinhdongesp32/DasaiBitmaps'

if not os.path.exists(output_dir):
    os.makedirs(output_dir)

def convert_gif_h_to_bitmap_h(file_path, filename):
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Extract hex bytes using regex
    hex_pattern = re.compile(r'0x[0-9a-fA-F]{2}')
    bytes_list = [int(h, 16) for h in hex_pattern.findall(content)]
    gif_bytes = bytes(bytes_list)
    
    # Open GIF with PIL
    try:
        img = Image.open(io.BytesIO(gif_bytes))
    except Exception as e:
        print(f"Error opening {filename}: {e}")
        return

    frames = []
    try:
        while True:
            # Process current frame
            # 1. Convert to RGB then to 1-bit (threshold)
            frame = img.convert('RGB').convert('1')
            frame = frame.resize((128, 64)) # Ensure correct size
            
            # 2. Convert to SSD1306/SH1106 bitmap format (Vertical bytes)
            # U8G2/SSD1306 format: 128x64 pixels = 1024 bytes
            # Pixels are packed into bytes vertically in pages of 8
            bitmap = bytearray(1024)
            pixels = frame.load()
            for y in range(64):
                for x in range(128):
                    if pixels[x, y]: # If white
                        # SH1106/SSD1306 page mapping
                        bitmap[x + (y // 8) * 128] |= (1 << (y % 8))
            
            frames.append(bitmap)
            img.seek(img.tell() + 1)
    except EOFError:
        pass # End of frames

    # Generate output .h file
    var_name = os.path.splitext(filename)[0]
    if var_name[0].isdigit():
        var_name = 'anim_' + var_name
    
    out_path = os.path.join(output_dir, filename)
    with open(out_path, 'w') as f:
        f.write(f"#ifndef {var_name.upper()}_BIT-MAP_H\n")
        f.write(f"#define {var_name.upper()}_BIT-MAP_H\n\n")
        f.write("#include <pgmspace.h>\n\n")
        f.write(f"const int {var_name}_frame_count = {len(frames)};\n")
        f.write(f"const uint8_t {var_name}_frames[{len(frames)}][1024] PROGMEM = {{\n")
        
        for i, frame in enumerate(frames):
            f.write("  {\n    ")
            hex_data = [f"0x{b:02x}" for b in frame]
            for j in range(0, len(hex_data), 16):
                f.write(", ".join(hex_data[j:j+16]))
                if j + 16 < len(hex_data):
                    f.write(",\n    ")
            f.write("\n  }")
            if i < len(frames) - 1:
                f.write(",")
            f.write("\n")
            
        f.write("};\n\n")
        f.write("#endif\n")
    
    print(f"Converted {filename}: {len(frames)} frames")

# Process all .h files in input_dir
for filename in os.listdir(input_dir):
    if filename.endswith('.h') and filename != 'animation.h':
        convert_gif_h_to_bitmap_h(os.path.join(input_dir, filename), filename)
