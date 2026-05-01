import cv2
import numpy as np
import os

# --- 1. GLOBAL SETTINGS (Your Tuned Values) ---
VIDEO_FILE = '0419(1)/0419(1).mp4'
OUTPUT_DIR = 'hinhdongesp32/DasaiBitmaps'

DEFAULT_BL = 60    
DEFAULT_CT = 1.6      
DEFAULT_GM = 0.75         

# --- 2. ANIMATION LIST ---
# Add your frame ranges here. 
# You can override global settings for specific animations!
ANIMATIONS = [
    # {"name": "happy", "start": 100, "end": 250}, 
    # {"name": "stubborn_one", "start": 500, "end": 600, "bl": 40, "ct": 2.0}, # Custom override
]

# --- 3. THE ENGINE ---
if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)

BAYER_MATRIX = np.array([[0,8,2,10],[12,4,14,6],[3,11,1,9],[15,7,13,5]]) * 16

def apply_bayer_dither(img):
    h, w = img.shape
    tile = np.tile(BAYER_MATRIX, (h // 4 + 1, w // 4 + 1))[:h, :w]
    return (img > tile).astype(np.uint8) * 255

def adjust_gamma(image, gamma=1.0):
    invGamma = 1.0 / gamma
    table = np.array([((i / 255.0) ** invGamma) * 255 for i in np.arange(0, 256)]).astype("uint8")
    return cv2.LUT(image, table)

def pack_to_u8g2(img):
    bitmap = bytearray(1024)
    for y in range(64):
        for x in range(128):
            if img[y, x] > 128:
                byte_idx = (y * 16) + (x // 8)
                bit_idx = 7 - (x % 8)
                bitmap[byte_idx] |= (1 << bit_idx)
    return bitmap

def export_animation(anim):
    name = anim["name"]
    start_f = anim["start"]
    end_f = anim["end"]
    bl = anim.get("bl", DEFAULT_BL)
    ct = anim.get("ct", DEFAULT_CT)
    gm = anim.get("gm", DEFAULT_GM)

    cap = cv2.VideoCapture(VIDEO_FILE)
    cap.set(cv2.CAP_PROP_POS_FRAMES, start_f)
    
    frame_buffer = []
    print(f"Exporting {name} (Frames {start_f}-{end_f}) [BL:{bl} CT:{ct} GM:{gm}]...")
    
    for _ in range(start_f, end_f):
        ret, frame = cap.read()
        if not ret: break
        
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        _, mask = cv2.threshold(gray, bl, 255, cv2.THRESH_TOZERO)
        smoothed = cv2.bilateralFilter(mask, 5, 50, 50)
        adjusted = adjust_gamma(smoothed, gm)
        final_gray = cv2.convertScaleAbs(adjusted, alpha=ct, beta=0)
        resized = cv2.resize(final_gray, (128, 64), interpolation=cv2.INTER_LANCZOS4)
        dithered = apply_bayer_dither(resized)
        frame_buffer.append(pack_to_u8g2(dithered))
    
    cap.release()
    
    if not frame_buffer: return

    out_path = os.path.join(output_dir if 'output_dir' in locals() else OUTPUT_DIR, f"{name}.h")
    with open(out_path, 'w') as f:
        f.write(f"#ifndef {name.upper()}_BITMAP_H\n#define {name.upper()}_BITMAP_H\n")
        f.write(f"#include <pgmspace.h>\n\n")
        f.write(f"const int {name}_frame_count = {len(frame_buffer)};\n")
        f.write(f"const uint8_t {name}_frames[{len(frame_buffer)}][1024] PROGMEM = {{\n")
        for i, fb in enumerate(frame_buffer):
            f.write("  {")
            f.write(", ".join([f"0x{b:02x}" for b in fb]))
            f.write("}" + ("," if i < len(frame_buffer)-1 else "") + "\n")
        f.write("};\n#endif\n")

# Run the export
if __name__ == "__main__":
    if not ANIMATIONS:
        print("Error: No animations listed in the ANIMATIONS array!")
    else:
        for anim in ANIMATIONS:
            export_animation(anim)
        print("\nFINISHED! Check hinhdongesp32/DasaiBitmaps/")
