import cv2
import mediapipe as mp
import time

# The following function initializes the USB camera, aiming to optimize fps
# Code for this function was based off the code authored by AJR and posted on the
# OpenCV forum "OpenCV Camera Low FPS" on Dec 3, 2020.
# Link: forum.opencv.org/t/opencv-camera-low-fps/567/3
def initCamera():
    camera = cv2.VideoCapture(0, cv2.CAP_V4L2)
    camera.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
    camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)
    camera.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'))
    camera.set(cv2.CAP_PROP_FPS, 60)
    return camera

# The following funcion simply displays the USB camera feed if the camera is
# working properly. This function, and following functions where indicated,
# are based off the code authored by Syed Abdul Gaffar Shakhadri and posted on
# the forum Analytics Vidhya on July 8, 2021. The forum post is titled "Building
# a Hand Tracking System using OpenCV".
# Link: analyticsvidhya.com/blog/2021/07/building-a-hand-tracking-system-using-opencv/
def cameraCheck(cap):
    pTime = 0
    while True:
        success, img = cap.read()
        imgRGB = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        cTime = time.time()
        fps = 1 / (cTime - pTime)
        pTime = cTime
        cv2.putText(img, f'FPS:{int(fps)}', (20, 70), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        cv2.imshow("Test", img)
        cv2.waitKey(1)
    
class handDetector():
    def __init__(self, mode=False, maxHands=2, detectionCon=0.5, trackCon=0.5):
        self.mode = mode
        self.maxHands = maxHands
        self.detectionCon = detectionCon
        self.trackCon = trackCon
        
        self.mpHands = mp.solutions.hands
        self.hands = self.mpHands.Hands(self.mode, self.maxHands, self.detectionCon, self.trackCon)
        self.mpDraw = mp.solutions.drawing_utils
        
    def findHands(self, img, draw=True):
        imgRGB = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        self.results = self.hands.process(imgRGB)
        if draw:
            if self.results.multi_hand_landmarks:
                for handLms in self.results.multi_hand_landmarks:
                    self.mpDraw.draw_landmarks(img, handLms, self.mpHands.HAND_CONNECTIONS)
        return img
    
    def findPosition(self, img, handNo=0, draw=True):
        lmList = []
        if self.results.multi_hand_landmarks:
            myHand = self.results.multi_hand_landmarks[handNo]
            for i in range(1, 6):
                lm = myHand.landmark[i*4]
                h, w, c = img.shape
                cx, cy = int(lm.x * w), int(lm.y * h)
                lmList.append([i*4, cx, cy])
                if draw:
                    cv2.circle(img, (cx, cy), 3, (255, 0, 255), cv2.FILLED)
        return lmList 
# Main function
def main():
    cap = initCamera()
    print("Camera Initialized")
    #cameraCheck(cap)
    pTime = 0
    cTime = 0
    detector = handDetector()
    
    while True:
        success, img = cap.read()
        img = detector.findHands(img)
        lmList = detector.findPosition(img)
        
        cTime = time.time()
        fps = 1 / (cTime - pTime)
        pTime = cTime
        print(str(fps))
        
        cv2.putText(img, str(int(fps)), (10, 70), cv2.FONT_HERSHEY_PLAIN, 3, (255, 0, 255), 3)
        
        cv2.imshow("Image", img)
        cv2.waitKey(1)
    
if __name__ == '__main__':
    main()