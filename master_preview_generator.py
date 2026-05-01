import cv2
import numpy as np
import os
from PIL import Image

# CONFIG
VIDEO_FILE = 'Dasai Mochi 3 all animations 1080P.mp4'
OUTPUT_DIR = 'MASTER_PREVIEWS'
BLACK_LEVEL = 65
CONTRAST = 1.9
GAMMA = 0.75
SLICE_DURATION = 900 

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

# 1. Scaled Selection Logic
cap = cv2.VideoCapture(VIDEO_FILE)
ret, first_frame = cap.read()
if not ret:
    print("Error: Could not read video.")
    exit()

# Scale to 35% for UI selection only
scale = 0.35
preview_w = int(first_frame.shape[1] * scale)
preview_h = int(first_frame.shape[0] * scale)
resized_preview = cv2.resize(first_frame, (preview_w, preview_h))

pts = []
def select_points(event, x, y, flags, param):
    global pts
    if event == cv2.EVENT_LBUTTONDOWN and len(pts) < 2:
        pts.append((x, y))

print("\n[UI SCALED TO 35%]")
print("1. Click Top-Left of Mochi screen.")
print("2. Click Bottom-Right of Mochi screen.")
print("3. Press ENTER to begin processing.")

cv2.namedWindow("Select Mochi Screen")
cv2.setMouseCallback("Select Mochi Screen", select_points)

while True:
    temp = resized_preview.copy()
    for p in pts: cv2.circle(temp, p, 5, (0, 0, 255), -1)
    if len(pts) == 2: cv2.rectangle(temp, pts[0], pts[1], (0, 255, 0), 2)
    cv2.imshow("Select Mochi Screen", temp)
    key = cv2.waitKey(1) & 0xFF
    if key == 13 and len(pts) == 2: break
    if key == ord('r'): pts = []

cv2.destroyAllWindows()

# Map coordinates back to 1080p
x1, y1 = [int(v / scale) for v in pts[0]]
x2, y2 = [int(v / scale) for v in pts[1]]
x_start, x_end = min(x1, x2), max(x1, x2)
y_start, y_end = min(y1, y2), max(y1, y2)

# 2. Process Video
part_idx = 0
frames_out = []
frame_total = 0
print(f"Processing... Using crop area {x_end-x_start}x{y_end-y_start} at full resolution.")

while True:
    ret, frame = cap.read()
    if not ret: break
    
    crop = frame[y_start:y_end, x_start:x_end]
    if crop.size == 0: continue
    
    gray = cv2.cvtColor(crop, cv2.COLOR_BGR2GRAY)
    _, mask = cv2.threshold(gray, BLACK_LEVEL, 255, cv2.THRESH_TOZERO)
    smoothed = cv2.bilateralFilter(mask, 9, 75, 75)
    adjusted = adjust_gamma(smoothed, gamma=GAMMA)
    final_gray = cv2.convertScaleAbs(adjusted, alpha=CONTRAST, beta=0)
    resized = cv2.resize(final_gray, (128, 64), interpolation=cv2.INTER_LANCZOS4)
    dithered = apply_bayer_dither(resized)
    
    frames_out.append(Image.fromarray(dithered))
    frame_total += 1
    
    if frame_total % 100 == 0:
        print(f"Frame {frame_total} processed...")
    
    if len(frames_out) >= SLICE_DURATION:
        filename = os.path.join(OUTPUT_DIR, f"MASTER_PART_{part_idx}.gif")
        frames_out[0].save(filename, save_all=True, append_images=frames_out[1:], duration=33, loop=0)
        print(f"--- PART {part_idx} SAVED ---")
        part_idx += 1
        frames_out = []

if frames_out:
    filename = os.path.join(OUTPUT_DIR, f"MASTER_PART_{part_idx}.gif")
    frames_out[0].save(filename, save_all=True, append_images=frames_out[1:], duration=33, loop=0)

cap.release()
print("\nSUCCESS! Check the 'MASTER_PREVIEWS' folder.")
