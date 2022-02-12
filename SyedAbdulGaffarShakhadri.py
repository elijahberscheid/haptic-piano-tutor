import cv2
import mediapipe as mp
import time

# The following function initializes the USB camera, aiming to optimize fps
# Code for this function was based off the code authored by AJR and posted on the
# OpenCV forum "OpenCV Camera Low FPS" on Dec 3, 2020.
# Link: forum.opencv.org/t/opencv-camera-low-fps/567/3
def initCamera():
    camera = cv2.VideoCapture(0, cv2.CAP_V4L2)
    camera.set(cv2.CAP_PROP_FRAME_WIDTH, 1600) # x
    camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 300) # y
    # Note: camera display of piano keyboard intended to NOT be inverted
    camera.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'))
    camera.set(cv2.CAP_PROP_FPS, 60)
    return camera

# The following funcion simply displays the USB camera feed if the camera is
# working properly. This function, the handDetector class, and the main function below,
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

# Class to track hand/finger position, has display functionalities
class handDetector():
    # Constructor for handDetector object
    # Detection and track confidence set to 50 percent, designed to operate with two hands
    def __init__(self, mode=False, maxHands=2, detectionCon=0.5, trackCon=0.5):
        self.mode = mode
        self.maxHands = maxHands
        self.detectionCon = detectionCon
        self.trackCon = trackCon
        
        self.mpHands = mp.solutions.hands
        self.hands = self.mpHands.Hands(self.mode, self.maxHands, self.detectionCon, self.trackCon)
        self.mpDraw = mp.solutions.drawing_utils
    
    # Function serves to identify the positions of the hands/fingers of the user, and draw those positons on the
    # mediapipe pipeline output if draw = True
    def findHands(self, img, draw=True):
        imgRGB = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        self.results = self.hands.process(imgRGB)
        if draw:
            if self.results.multi_hand_landmarks:
                for handLms in self.results.multi_hand_landmarks:
                    self.mpDraw.draw_landmarks(img, handLms, self.mpHands.HAND_CONNECTIONS)
        return img
    
    # Function serves to output the position of "landmarks of interest" (4, 8, 12, 16, 20: fingertips on each hand)
    def findPosition(self, img, draw=True):
        lmList = []
        # If a hand is detected in the frame of the USB camera
        if self.results.multi_hand_landmarks:
            # If only one hand is detected
            if len(self.results.multi_hand_landmarks) == 1:
                myHand = self.results.multi_hand_landmarks[0]
                for i in range(1, 6):
                    lm = myHand.landmark[i*4]
                    h, w, c = img.shape
                    cx, cy = int(lm.x * w), int(lm.y * h)
                    # 0 in index 0 indicates only one hand detected, hand not specified
                    lmList.append([0, i*4, cx, cy])
                    if draw:
                        cv2.circle(img, (cx, cy), 3, (255, 0, 255), cv2.FILLED)
            # If two hands are detected
            else:
                myHand1 = self.results.multi_hand_landmarks[0]
                for i in range(1, 6):
                    lm = myHand1.landmark[i*4]
                    h, w, c = img.shape
                    cx, cy = int(lm.x * w), int(lm.y * h)
                    # 1 in index 0 indicates left hand
                    lmList.append([1, i*4, cx, cy])
                    if draw:
                        cv2.circle(img, (cx, cy), 3, (255, 0, 255), cv2.FILLED)
                myHand2 = self.results.multi_hand_landmarks[1]
                for i in range(1, 6):
                    lm = myHand2.landmark[i*4]
                    h, w, c = img.shape
                    cx, cy = int(lm.x * w), int(lm.y * h)
                    # 2 in index 0 indicates right hand
                    lmList.append([2, i*4, cx, cy])
                    if draw:
                        cv2.circle(img, (cx, cy), 3, (255, 0, 255), cv2.FILLED)
        return lmList 

# Main function
def main():
    cap = initCamera()
    print("Camera Initialized")
    
    # Debug USB camera logic, does not return to main
    #cameraCheck(cap)
    
    # Initializations
    pTime = 0
    cTime = 0
    sumFPS = 0
    countFPS = 0
    prevFPS = 0
    detector = handDetector()
    
    # Main processing loop
    while True:
        # Reading of camera, finding position of hands/fingers
        success, img = cap.read()
        img = detector.findHands(img)
        lmList = detector.findPosition(img)
        
        # Processing time display logic, mainly for debugging purposes
        cTime = time.time()
        fps = 1 / (cTime - pTime)
        pTime = cTime
        sumFPS += fps
        countFPS += 1
        avgFPS = round(sumFPS/countFPS)
        if(avgFPS != prevFPS and countFPS > 5):
            print("Average Processing Time: " + str(avgFPS) + " Hz")
        prevFPS = avgFPS
        cv2.putText(img, (str(int(avgFPS)) + " Hz"), (10, 70), cv2.FONT_HERSHEY_PLAIN, 3, (255, 0, 255), 3)
        
        # Landmark display logic, mainly for debugging purposes
        for landmark in lmList:
           print(landmark)
        
        # Computer vision pipeline output, mainly for debugging purposes
        cv2.imshow("Image", img)
        cv2.waitKey(1)
    
if __name__ == '__main__':
    main()