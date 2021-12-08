from utils import cal_len_2d, finger_status_loader_5way

import mediapipe as mp
import numpy as np
import cv2
import os

cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH,  640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)


## mediapipe setting
mp_drawing = mp.solutions.drawing_utils
mp_drawing_styles = mp.solutions.drawing_styles
mp_hands = mp.solutions.hands

hands = mp_hands.Hands(max_num_hands = 1,              # 인식할 손 갯수
                       min_detection_confidence=0.7,   # 손바닥 찾기 최소 신뢰도
                       min_tracking_confidence=0.7)    # 관절 찾기 최소 신뢰도

pre_status  = [0x00,0x00,0x00]
status      = [0x00,0x00,0x00]
handedness  = None
switch      = False
drawing     = True
same_status = 0
send_data = False

while True:
    ret, image = cap.read()

    image.flags.writeable = False
    image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
    results = hands.process(image)

    image.flags.writeable = True
    image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)
    if results.multi_hand_landmarks and flag:
        for hand_landmarks in results.multi_hand_landmarks:

            xyz = np.empty((13,3), dtype=np.float16)
            handedness = results.multi_handedness[0].classification[0].label.lower()    # Left, Right
            handedness = 'left' if handedness == 'right' else 'right'

            for i in range(0, 13):
                xyz[i][0] = hand_landmarks.landmark[i].x * 640
                xyz[i][1] = hand_landmarks.landmark[i].y * 480
                xyz[i][2] = hand_landmarks.landmark[i].z * 640

            # 0 - wrist | 1~4 - thumb | 5~8 - index | 9~12 - middle
            # xyz - xyz값의 픽셀값
            # handedness - 손방향
            status = finger_status_loader_5way(xyz, handedness)  

            if drawing: # press 'd' for switching
                mp_drawing.draw_landmarks(
                    image,
                    hand_landmarks,
                    mp_hands.HAND_CONNECTIONS,
                    mp_drawing_styles.get_default_hand_landmarks_style(),
                    mp_drawing_styles.get_default_hand_connections_style())
            


    if pre_status == status: 
        same_status += 1
    else: 
        same_status = 0

    if same_status == 3: 
        print(status)
    else: 
        pass

    if switch: 
        # print(status, pre_status, pre_status==status)
        pre_status=status


    cv2.imshow('MediaPipe Hands', cv2.flip(image, 1))

    key = cv2.waitKey(5) & 0xFF

    if key == ord('q'):
        break

    elif drawing == True and key == ord('d'):   
        print("DRAWING OFF") 
        drawing = False
    elif drawing == False and key == ord('d'):    
        print("DRAWING ON") 
        drawing = True

    elif switch == True and key == ord('s'):
        print("SWITCH OFF") 
        switch = False    
    elif switch == False and key == ord('s'):
        print("SWITCH ON") 
        switch = True


cap.release()
