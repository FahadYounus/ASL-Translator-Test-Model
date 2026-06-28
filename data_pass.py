import cv2
import csv
import os
import mediapipe as mp

# MediaPipe setup
mp_hands = mp.solutions.hands
mp_draw  = mp.solutions.drawing_utils

# Hand detector
hand = mp_hands.Hands(
    static_image_mode=False,
    max_num_hands=1,
    min_detection_confidence=0.7,
    min_tracking_confidence=0.7
)

# Open camera
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("Error: Could not open webcam. Is another program (like your Qt GUI) using it?")
    exit()
print("Hand Tracker running... Press ESC to quit")

# CSV path — C++ reads this
CSV_PATH = 'C:\\ASL_PROJECT\\landmarks.csv'

# Smoothing — average last 5 frames
coord_history = []
SMOOTH_FRAMES = 7

no_hand_count = 0

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame     = cv2.flip(frame, 1)                     # Mirror
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB) # BGR to RGB
    result    = hand.process(rgb_frame)                # Detect hand

    if result.multi_hand_landmarks:
        no_hand_count = 0 #reset
        for hand_landmarks in result.multi_hand_landmarks:

            # Draw hand skeleton
            mp_draw.draw_landmarks(
                frame, hand_landmarks,
                mp_hands.HAND_CONNECTIONS)

            # Get 21 landmarks x,y,z = 63 values
            coord = []
            for lm in hand_landmarks.landmark:
                coord.extend([lm.x, lm.y, lm.z])

            # Add to history
            coord_history.append(coord)
            if len(coord_history) > SMOOTH_FRAMES:
                coord_history.pop(0)

            # Average frames for smooth output
            avg_coord = [
                sum(x) / len(x)
                for x in zip(*coord_history)
            ]

            # Write to CSV
            with open(CSV_PATH, 'w', newline='') as f:
                csv.writer(f).writerow(avg_coord)
                f.flush()
                os.fsync(f.fileno())
    else:
        # No hand — clear CSV
        no_hand_count += 1
        if no_hand_count > 10:
            coord_history.clear()
            open(CSV_PATH, 'w').close()

    cv2.imshow("Hand Tracker", frame)

   
    key = cv2.waitKey(1) & 0xFF
    if key == 27 or key == ord('q'):
        break
    # if cv2.waitKey(1) & 0xFF == 27:
    #     break

# Cleanup
cap.release()
cv2.destroyAllWindows()
open(CSV_PATH, 'w').close()
print("Done.")