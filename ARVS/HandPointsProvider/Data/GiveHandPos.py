import cv2
import mediapipe as mp
import os
from google.protobuf.json_format import MessageToDict

def ComputeHandPos():
    cap = cv2.VideoCapture(0)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)  #设置宽度
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)  #设置高度
    mpHands = mp.solutions.hands
    hands = mpHands.Hands()
    fileDir=os.path.dirname(os.path.abspath(__file__))
    while True:
        success, img = cap.read()
        imgRGB = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        results = hands.process(imgRGB)
        if results.multi_hand_landmarks:
            data = open(fileDir+"\HandPos.txt", 'w+')
            for handId , handLms in enumerate(results.multi_hand_landmarks):
                # 左右手
                handedness_dict = MessageToDict(results.multi_handedness[handId])
                res_handed = int(handedness_dict['classification'][0]['index'])

                for id, lm in enumerate(handLms.landmark):
                    h, w, c = img.shape
                    cx, cy, cz= int(lm.x * w), int(lm.y * h),int(lm.z * w)
                    print(id, cx, cy, cz , file=data)
                    print(res_handed,id, cx, cy, cz)
                    if os.path.exists(fileDir+"\ShowImg"):
                        radius=int(cz*0.1)+1
                        if radius<0:
                            radius=-radius
                        cv2.circle(img, (cx, cy), radius, (255, 0, 255), cv2.FILLED)  # 圆
            data.close()

        if os.path.exists(fileDir+"\ShowImg"):
            cv2.imshow("Image", img)

        if ord('q') == cv2.waitKey(1):
            break
        if 27 == cv2.waitKey(1):
            break
		
        if os.path.exists(fileDir+"\CloseHandPos"):
            break

ComputeHandPos()
