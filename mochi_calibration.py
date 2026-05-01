import cv2
import numpy as np
import os
from PIL import Image

# CALIBRATION CONFIG
VIDEO_FILE = 'Dasai Mochi 3 all animations 1080P.mp4'
BLACK_LEVEL = 60    # Increase this (0-255) to "kill" glare/noise in dark areas
CONTRAST = 1.8      # Eye brightness
GAMMA = 0.8         # Lower < 1.0 makes shadows darker
TEST_DURATION = 300 # Process 300 frames (~10 seconds) for the test

# Bayer 4x4 Dithering Matrix
BAYER_MATRIX = np.array([
    [ 0,  8,  2, 10],
    [12,  4, 14,  6],
    [ 3, 11,  1,  9],
    [15,  7, 13,  5]
]) * 16

def apply_bayer_dither(img):
    h, w = img.shape
    tile = np.tile(BAYER_MATRIX, (h // 4 + 1, w // 4 + 1))[:h, :w]
    return (img > tile).astype(np.uint8) * 255

def adjust_gamma(image, gamma=1.0):
    invGamma = 1.0 / gamma
    table = np.array([((i / 255.0) ** invGamma) * 255 for i in np.arange(0, 256)]).astype("uint8")
    return cv2.LUT(image, table)

# 1. Selection
cap = cv2.VideoCapture(VIDEO_FILE)
ret, first_frame = cap.read()
scale = 0.35
preview = cv2.resize(first_frame, (int(first_frame.shape[1]*scale), int(first_frame.shape[0]*scale)))

pts = []
def select_points(event, x, y, flags, param):
    global pts
    if event == cv2.EVENT_LBUTTONDOWN and len(pts) < 2: pts.append((x, y))

cv2.namedWindow("Calibration")
cv2.setMouseCallback("Calibration", select_points)
while True:
    temp = preview.copy()
    for p in pts: cv2.circle(temp, p, 5, (0, 0, 255), -1)
    if len(pts) == 2: cv2.rectangle(temp, pts[0], pts[1], (0, 255, 0), 2)
    cv2.imshow("Calibration", temp)
    k = cv2.waitKey(1) & 0xFF
    if k == 13 and len(pts) == 2: break
    if k == ord('r'): pts = []
cv2.destroyAllWindows()

x_start, y_start = [int(v / scale) for v in min(pts[0], pts[1])]
x_end, y_end = [int(v / scale) for v in max(pts[0], pts[1])]

# 2. Process Test Clip
print("Generating CALIBRATION_TEST.gif...")
frames_out = []
for i in range(TEST_DURATION):
    ret, frame = cap.read()
    if not ret: break
    
    # A. Crop
    crop = frame[y_start:y_end, x_start:x_end]
    
    # B. Grayscale & Denoise
    gray = cv2.cvtColor(crop, cv2.COLOR_BGR2GRAY)
    
    # C. ANTI-GLARE PIPELINE
    # Force low-light noise/glare to absolute zero
    _, mask = cv2.threshold(gray, BLACK_LEVEL, 255, cv2.THRESH_TOZERO)
    
    # Smooth banding with Bilateral Filter (preserves eye edges better than Gaussian)
    smoothed = cv2.bilateralFilter(mask, 9, 75, 75)
    
    # Boost Contrast & Gamma
    adjusted = adjust_gamma(smoothed, gamma=GAMMA)
    final_gray = cv2.convertScaleAbs(adjusted, alpha=CONTRAST, beta=0)
    
    # D. Dither
    resized = cv2.resize(final_gray, (128, 64), interpolation=cv2.INTER_LANCZOS4)
    dithered = apply_bayer_dither(resized)
    
    frames_out.append(Image.fromarray(dithered))

if frames_out:
    frames_out[0].save("CALIBRATION_TEST.gif", save_all=True, append_images=frames_out[1:], duration=33, loop=0)
    print("\nSUCCESS: 'CALIBRATION_TEST.gif' created. Please check it for glare/pixelation.")

cap.release()
