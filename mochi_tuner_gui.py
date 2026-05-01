import cv2
import numpy as np

# POINT TO YOUR NEW CROPPED VIDEO
VIDEO_FILE = '0419(1)/0419(1).mp4'

# Bayer Matrix
BAYER_MATRIX = np.array([[0,8,2,10],[12,4,14,6],[3,11,1,9],[15,7,13,5]]) * 16

def apply_bayer_dither(img):
    h, w = img.shape
    tile = np.tile(BAYER_MATRIX, (h // 4 + 1, w // 4 + 1))[:h, :w]
    return (img > tile).astype(np.uint8) * 255

def adjust_gamma(image, gamma=1.0):
    invGamma = 1.0 / (gamma / 100.0 + 0.1) 
    table = np.array([((i / 255.0) ** invGamma) * 255 for i in np.arange(0, 256)]).astype("uint8")
    return cv2.LUT(image, table)

cap = cv2.VideoCapture(VIDEO_FILE)
total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))

if total_frames == 0:
    print("Error: Video file not found or empty.")
    exit()

cv2.namedWindow("Mochi Final Tuner")
cv2.createTrackbar("BlackLevel", "Mochi Final Tuner", 30, 255, lambda x: None)
cv2.createTrackbar("Contrast", "Mochi Final Tuner", 15, 50, lambda x: None) 
cv2.createTrackbar("Gamma", "Mochi Final Tuner", 80, 200, lambda x: None)   
cv2.createTrackbar("Timeline", "Mochi Final Tuner", 0, total_frames-1, lambda x: None)

print("\n--- PRECISION TUNING MODE ---")
print("Use the SLIDERS to adjust the look.")
print("Use 'A' / 'D' keys to skip through the video.")
print("Use LEFT / RIGHT arrows for frame-by-frame precision.")
print("The FRAME NUMBER is shown in the top-left of the image.")
print("Press 'Q' to quit and save your settings.")

current_pos = 0

while True:
    # Get slider values
    bl = cv2.getTrackbarPos("BlackLevel", "Mochi Final Tuner")
    ct = cv2.getTrackbarPos("Contrast", "Mochi Final Tuner") / 10.0
    gm = cv2.getTrackbarPos("Gamma", "Mochi Final Tuner")
    
    # Read frame
    cap.set(cv2.CAP_PROP_POS_FRAMES, current_pos)
    ret, frame = cap.read()
    if not ret: break
    
    # Process
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    _, mask = cv2.threshold(gray, bl, 255, cv2.THRESH_TOZERO)
    smoothed = cv2.bilateralFilter(mask, 5, 50, 50)
    adjusted = adjust_gamma(smoothed, gm)
    final_gray = cv2.convertScaleAbs(adjusted, alpha=ct, beta=0)
    resized = cv2.resize(final_gray, (128, 64), interpolation=cv2.INTER_LANCZOS4)
    dithered = apply_bayer_dither(resized)
    
    # GUI Display
    display_zoom = cv2.resize(dithered, (512, 256), interpolation=cv2.INTER_NEAREST)
    display_final = cv2.cvtColor(display_zoom, cv2.COLOR_GRAY2BGR)
    
    # Draw frame number
    cv2.putText(display_final, f"FRAME: {current_pos}", (15, 40), 
                cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0, 255, 0), 2)
    
    cv2.imshow("Mochi Final Tuner", display_final)
    
    # Key handling
    key = cv2.waitKey(1) & 0xFF
    
    if key == ord('q'):
        print(f"\n--- FINAL SETTINGS ---")
        print(f"BLACK_LEVEL = {bl}")
        print(f"CONTRAST = {ct}")
        print(f"GAMMA = {gm/100.0}")
        break
    elif key == ord('d'): # Jump forward
        current_pos = min(total_frames - 1, current_pos + 30)
        cv2.setTrackbarPos("Timeline", "Mochi Final Tuner", current_pos)
    elif key == ord('a'): # Jump back
        current_pos = max(0, current_pos - 30)
        cv2.setTrackbarPos("Timeline", "Mochi Final Tuner", current_pos)
    
    # Also check trackbar for manual scrubbing
    trackbar_pos = cv2.getTrackbarPos("Timeline", "Mochi Final Tuner")
    if trackbar_pos != current_pos:
        current_pos = trackbar_pos

cap.release()
cv2.destroyAllWindows()
