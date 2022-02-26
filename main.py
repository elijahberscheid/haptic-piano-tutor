# Prerequisites: sudo apt update, sudo apt-get install python3-opencv
# References: https://docs.opencv.org/3.4/d2/de6/tutorial_py_setup_in_ubuntu.html
import cv2
# Prerequisites: sudo apt install -y python3-dev, sudo apt install -y cmake,
# sudo apt install -y protobuf-compiler, python3 -m pip install cython,
# python3 -m pip install numpy, python3 -m pip install pillow,
# python3 -m pip install mediapipe-python-aarch64/mediapipe-0.8.4-cp38-cp38-linux_aarch64.whl
# References: https://github.com/jiuqiant/mediapipe_python_aarch64
import mediapipe as mp
import time
import threading
import numpy as np
# Prerequisites: sudo apt-get install libglib2.0-dev; sudo pip install bluepy
# References: https://elinux.org/RPi_Bluetooth_LE, https://ianharvey.github.io/bluepy-doc/
from bluepy import btle
# Prerequisites: sudo pip install RPi.GPIO
# References: https://pypi.org/project/RPi.GPIO/ 
import RPi.GPIO as GPIO

# Assuming a height of 31" from piano keyboard to camera lens
# Establish resolution dimensions for the USB
camX = 1920
camY = 700

# Establish a BLE pairing with the MCU
def establishBLE():
    # Connection attempt loop
    loopVar = True
    atm = 1 # current connection attempt
    while loopVar:
        print("Attempt at Connection " + str(atm))
        try:
            # btle.Peripheral throws btle.BTLEDisconnectError if pairing unsuccessful
            dev = btle.Peripheral("62:00:A1:21:6E:67")
            passthroughUuid = btle.UUID("0000ffe0-0000-1000-8000-00805f9b34fb")
            passthroughService = dev.getServiceByUUID(passthroughUuid)

            writeUuid = btle.UUID("0000ffe9-0000-1000-8000-00805f9b34fb")
            writeCharacteristic = passthroughService.getCharacteristics(writeUuid)[0]

            GPIO.output(18, 1) # Pin 18 (Blue LED) on
            print("Pairing Successful")
            loopVar = False
        except btle.BTLEDisconnectError:
            print("Connection Attempt " + str(atm) + " Failed")
            GPIO.output(18, 0) # Pin 18 (Blue LED) off
        atm += 1

# The following function initializes the USB camera, aiming to optimize fps
# Code for this function was based off the code authored by AJR and posted on the
# OpenCV forum "OpenCV Camera Low FPS" on Dec 3, 2020.
# Link: forum.opencv.org/t/opencv-camera-low-fps/567/3
def initCamera():
    camera = cv2.VideoCapture(0)
    global camX
    camera.set(cv2.CAP_PROP_FRAME_WIDTH, camX) # x
    global camY
    camera.set(cv2.CAP_PROP_FRAME_HEIGHT, camY) # y
    # Note: camera display of piano keyboard intended to NOT be inverted
    camera.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'))
    camera.set(cv2.CAP_PROP_FPS, 60)
    print("Camera Initialized")
    return camera

# Given a picture from the USB camera, this function generates the expected piano key
# locations and returns them in the keyContours data structure. As an 88-key piano is
# expected, the keyContours structure will have 88 entries.
def expectedKeyGeneration(cap):
    keyContours = []
    success, img = cap.read()
    for _ in range(10):
        success, img = cap.read()
    if np.size(img) > 0:
        # First find green retroreflective tape coordinates
        # https://techvidvan.com/tutorials/detect-objects-of-similar-color-using-opencv-in-python/
        # convert to hsv colorspace
        hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
        # lower bound and upper bound for Green color
        lower_bound = np.array([60, 115, 60])   
        upper_bound = np.array([100, 255, 255])
        # Values for 477 Lab Testing
        #lower_bound = np.array([60, 115, 60])   
        #upper_bound = np.array([100, 255, 255])    
        # find the colors within the boundaries
        mask = cv2.inRange(hsv, lower_bound, upper_bound)
        #define kernel size  
        kernel = np.ones((8,8),np.uint8)
        # Remove unnecessary noise from mask
        mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)
        mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
        # Segment only the detected region
        segmented_img = cv2.bitwise_and(img, img, mask=mask)
        # Find contours from the mask
        contours, hierarchy = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        # Print contours, debugging purposes
        print(contours)
        output = cv2.drawContours(segmented_img, contours, -1, (0, 0, 255), 3)
        # Showing the output, debugging purposes
        cv2.imshow("Output", output)
        # Determining which contours correspond to the tape
        global camX
        xLimit = .5*camX
        xTolerance = .025*camX
        sampleContourX = []
        for contour in contours:
            sampleContourX.append(contour[0][0][0])
        matchingPairs = []
        optimizer = 1
        for sampledX1 in sampleContourX:
            for sampledX2 in sampleContourX[optimizer :]:
                if(sampledX1 < sampledX2):
                    if(sampledX1 + xTolerance >= sampledX2):
                        matchingPairs.append((sampledX1, sampledX2))    
                else:
                    if(sampledX1 - xTolerance <= sampledX2):
                        matchingPairs.append((sampledX1, sampledX2))
            optimizer += 1         
        print(sampleContourX)
        print(matchingPairs)
    else:
        BleTrasmitError(0)
        print("***Error Occured*** expectedKeyGeneration USB Camera Error")
    return keyContours

# Given the keyContours structure, detailing the expected piano key pixel locations, and
# the lmList structure, detailing the fingertip pixel locations of the user, this function
# will return the lmLocations integer array, matching each finger to a piano key (denoted
# 0 - 87) or 88 if the user's finger is not detected to be over a particular piano key.
def matchLocations(keyContours, lmList):
    lmLocations = [88, 88, 88, 88, 88, 88, 88, 88, 88, 88] # All landmark locations initalized to error code
    return lmLocations
 

# Class to track hand/finger position, has display functionalities
# This handDetector class is based off the code authored by Syed Abdul Gaffar Shakhadri
# and posted on the forum Analytics Vidhya on July 8, 2021. The forum post is titled
# "Building a Hand Tracking System using OpenCV".
# Link: analyticsvidhya.com/blog/2021/07/building-a-hand-tracking-system-using-opencv/
class handDetector():
    # Constructor for handDetector object
    # Detection and track confidence set to 50 percent, designed to operate with up to two hands
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
    def findHands(self, img, draw=False):
        imgRGB = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        self.results = self.hands.process(imgRGB)
        if draw:
            if self.results.multi_hand_landmarks:
                for handLms in self.results.multi_hand_landmarks:
                    self.mpDraw.draw_landmarks(img, handLms, self.mpHands.HAND_CONNECTIONS)
        return img
    
    # Function serves to output the position of "landmarks of interest" (4, 8, 12, 16, 20: fingertips on each hand)
    def findPosition(self, img, draw=False):
        lmList = []
        # If a hand is detected in the frame of the USB camera
        if self.results.multi_hand_landmarks:
            # If only one hand is detected
            if len(self.results.multi_hand_landmarks) == 1:
                myHand = self.results.multi_hand_landmarks[0]
                leftHand = True
                pinkyKnuckle = myHand.landmark[17]
                pointerKnuckle = myHand.landmark[5]
                if(pinkyKnuckle.x > pointerKnuckle.x):
                    leftHand = False
                for i in range(1, 6):
                    lm = myHand.landmark[i*4]
                    h, w, c = img.shape
                    cx, cy = int(lm.x * w), int(lm.y * h)
                    # 1 in index 0 indicates left hand
                    # 2 in index 0 indicates right hand
                    if leftHand:
                        lmList.append([1, i*4, cx, cy])
                    else:
                        lmList.append([2, i*4, cx, cy])
                    if draw:
                        cv2.circle(img, (cx, cy), 3, (255, 0, 255), cv2.FILLED)
            # If two hands are detected
            else:
                myHand1 = self.results.multi_hand_landmarks[0]
                myHand2 = self.results.multi_hand_landmarks[1]
                for i in range(1, 6):
                    lm = myHand1.landmark[i*4]
                    h, w, c = img.shape
                    cx, cy = int(lm.x * w), int(lm.y * h)
                    # 1 in index 0 indicates left hand
                    lmList.append([1, i*4, cx, cy])
                    if draw:
                        cv2.circle(img, (cx, cy), 3, (255, 0, 255), cv2.FILLED)
                for i in range(1, 6):
                    lm = myHand2.landmark[i*4]
                    h, w, c = img.shape
                    cx, cy = int(lm.x * w), int(lm.y * h)
                    # 2 in index 0 indicates right hand
                    lmList.append([2, i*4, cx, cy])
                    if draw:
                        cv2.circle(img, (cx, cy), 3, (255, 0, 255), cv2.FILLED)
        return lmList 

currentlmLocations = [88, 88, 88, 88, 88, 88, 88, 88, 88, 88] # All landmark locations set to error code

# Interrupt function, transmits the data in currentlmLocations to the MCU at 10 Hz
def BleTransmit():
    a = 0
    return a

# Called upon to transmit an error packet in the event of a calibration error
def BleTransmitError(code):
    # USB Camera Error
    if(code == 0):
        try:
            writeCharacteristic.write(bytes("error code 0"))
        except (btle.BTLEDisconnectError, btle.BTLEInternalError):
            GPIO.output(18, 0)
            establishBLE()
    # Retroreflective Tape Detection Error
    elif(code == 1):
        try:
            writeCharacteristic.write(bytes("error code 1"))
        except (btle.BTLEDisconnectError, btle.BTLEInternalError):
            GPIO.output(18, 0)
            establishBLE()
    
# Main function
def main():
    # Bluetooth Initialization 
    #GPIO.setmode(GPIO.BOARD)
    #GPIO.setup(18, GPIO.OUT)
    #GPIO.output(18, 0) # Ensure blue LED is off
    #establishBLE()
    
    # Bluetooth Error Code Testing
    #BleTransmitError(0)
    #BleTransmitError(1)
    
    # Camera Initialization
    cap = initCamera()

    # Expected Key Location Generation
    keyContours = expectedKeyGeneration(cap)
    print("Done")

    # Variable Initializations
    pTime = 0
    cTime = 0
    sumFPS = 0
    countFPS = 0
    prevFPS = 0
    detector = handDetector()
    
    # BLE Transmission Initialization
    threading.Timer(0.1, lambda: BleTransmit()).start() # start a timer on a new thread for BLE transmission at 10 Hz 
    
    # Main processing loop
    while True:
        # Reading of camera, finding position of hands/fingers
        success, img = cap.read()
        if np.size(img) > 0:
            img = detector.findHands(img)
            lmList = detector.findPosition(img)
        
            # BLE Transmission Logic
            threading.Timer(0, lambda: BleTransmit()).start()

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
            #cv2.putText(img, (str(int(avgFPS)) + " Hz"), (10, 70), cv2.FONT_HERSHEY_PLAIN, 3, (255, 0, 255), 3)
        
            # Match fingertip lanmark locations to expected key locations
            #global currentlmLocations
            #currentlmLocations = matchLocations(keyContours, lmList)
        
            # Computer vision pipeline output, mainly for debugging purposes
            cv2.imshow("Image", img)
            cv2.waitKey(1)

if __name__ == '__main__':
    main()