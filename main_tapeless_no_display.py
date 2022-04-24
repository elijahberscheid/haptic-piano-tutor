#!/usr/bin/env python3.8
# Last Edit: 4/23/22, Phillip Archuleta
# Edit Purpose: Incorporate touch calibration to replace tape calibration, general optimization

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
import bisect
import threading
import numpy as np
# Prerequisites: sudo apt-get install libglib2.0-dev; sudo pip install bluepy
# References: https://elinux.org/RPi_Bluetooth_LE, https://ianharvey.github.io/bluepy-doc/
from bluepy import btle

# Assuming a height of 35" from piano keyboard to camera lens
# Camera is expected 
# Establish resolution dimensions for the USB
camX = 1920
camY = 700
# Calibration Constants (as intended for resolution and camera height)
blackKeyMinArea = 300


# Establish a BLE pairing with the MCU
BLEVar = 0
def establishBLE():
    # Connection attempt loop
    loopVar = True
    atm = 1 # current connection attempt
    while loopVar:
        print("Attempt at Connection " + str(atm))
        try:
            # btle.Peripheral throws btle.BTLEDisconnectError if pairing unsuccessful
            dev = btle.Peripheral("62:00:A1:21:64:BB")
            passthroughUuid = btle.UUID("0000ffe0-0000-1000-8000-00805f9b34fb")
            passthroughService = dev.getServiceByUUID(passthroughUuid)
            writeUuid = btle.UUID("0000ffe9-0000-1000-8000-00805f9b34fb")
            writeCharacteristic = passthroughService.getCharacteristics(writeUuid)[0]
            global BLEVar
            BLEVar = writeCharacteristic
            print("Pairing Successful")
            loopVar = False
        except btle.BTLEDisconnectError:
            print("Connection Attempt " + str(atm) + " Failed")
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
    return camera


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
                leftHand = False
                pinkyKnuckle = myHand.landmark[17]
                pointerKnuckle = myHand.landmark[5]
                if(pinkyKnuckle.x > pointerKnuckle.x):
                    leftHand = True
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
                leftHand1 = False
                leftHand2 = False
                pinkyKnuckle1 = myHand1.landmark[17]
                pointerKnuckle1 = myHand1.landmark[5]
                pinkyKnuckle2 = myHand2.landmark[17]
                pointerKnuckle2 = myHand2.landmark[5]
                if(pinkyKnuckle1.x > pointerKnuckle1.x):
                    leftHand1 = True
                if(pinkyKnuckle2.x > pointerKnuckle2.x):
                    leftHand2 = True
                for i in range(1, 6):
                    lm = myHand1.landmark[i*4]
                    h, w, c = img.shape
                    cx, cy = int(lm.x * w), int(lm.y * h)
                    # 1 in index 0 indicates left hand
                    if leftHand1:
                        lmList.append([1, i*4, cx, cy])
                    else:
                        lmList.append([2, i*4, cx, cy])
                    if draw:
                        cv2.circle(img, (cx, cy), 3, (255, 0, 255), cv2.FILLED)
                for i in range(1, 6):
                    lm = myHand2.landmark[i*4]
                    h, w, c = img.shape
                    cx, cy = int(lm.x * w), int(lm.y * h)
                    # 2 in index 0 indicates right hand
                    if leftHand2:
                        lmList.append([1, i*4, cx, cy])
                    else:
                        lmList.append([2, i*4, cx, cy])
                    if draw:
                        cv2.circle(img, (cx, cy), 3, (255, 0, 255), cv2.FILLED)
        return lmList 


def perimeterHelper(cap, detector, rightHand):
    coords = []
    timerCount = 5
    failCount = 0
    xSum = 0
    ySum = 0
    for _ in range(10):
       success, img = cap.read()
       if(np.size(img) > 0):
           img = detector.findHands(img)
           lmList = detector.findPosition(img)   
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
    print('%%              HOLD FINGER UNTIL TIMER IS COMPLETE            %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')  
    while timerCount > 0:
        success, img = cap.read()
        lmFound = False
        if np.size(img) > 0:
            img = detector.findHands(img)
            lmList = detector.findPosition(img)
            if(rightHand):
                for lmCoord in lmList:
                    # right pointer fingertip
                    if(lmCoord[1] == 8 and lmCoord[0] == 2 and lmCoord[2] < 200):
                        xSum = xSum + lmCoord[2]
                        print(lmCoord[3])
                        ySum = ySum + lmCoord[3]
                        lmFound = True
                        failCount = 0
            else:
                for lmCoord in lmList:
                    # left pointer fingertip
                    if(lmCoord[1] == 8 and lmCoord[0] == 1 and lmCoord[2] > 1100 and lmCoord[2] < 1300):
                        xSum = xSum + lmCoord[2]
                        print(lmCoord[3])
                        ySum = ySum + lmCoord[3]
                        lmFound = True
                        failCount = 0
            if(lmFound):
                if(timerCount >= 10):
                    print('%%                               ' + str(timerCount) + '                            %%')
                else:
                    print('%%                                ' + str(timerCount) + '                            %%')    
                print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
                timerCount = timerCount - 1
                time.sleep(1)
            else:
                failCount = failCount + 1
            if(failCount > 50):
                print('%%          ISSUES DETECTING POINTER FINGER FINGERTIP          %%')
                print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
                print('%%                       PLEASE TRY AGAIN                      %%')
                print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')                
    print('%%                                0                            %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    time.sleep(0.5)                                
    coords.append(xSum/5)
    coords.append(ySum/5)
    print('\n' * 500)
    return coords


# cap: camera object
# detector: mediapipe hands object
# return: 8 integer array in the following format:
# [upperLeftx, upperLefty, lowerLeftx, lowerLefty, lowerRightx, lowerRighty, upperRightx, upperRighty]
def touchPerimeter(cap, detector):
    perimeterArray = [0, 0, 0, 0, 0, 0, 0, 0]
    rightHand = True
    print('\n' * 500)
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
    print('%%                      CALIBRATION SEQUENCE                   %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    print('%%  PLEASE USE RIGHT HAND POINTER FINGER FOR FIRST TWO POINTS  %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
    print('%%                PRESS ENTER WHEN READY TO BEGIN              %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    input()
    print('\n' * 500)
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
    print('%%   PLACE R POINTER FINGER ON UPPER RIGHT CORNER OF KEYBOARD  %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
    print('%%               HOLD FINGER THERE UNTIL INSTRUCTED            %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
    print('%%                PRESS ENTER WHEN READY TO BEGIN              %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    input()
    print('\n' * 500)
    upperRightCoords = perimeterHelper(cap, detector, rightHand)
    perimeterArray[6] = upperRightCoords[0]
    perimeterArray[7] = upperRightCoords[1]
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
    print('%%   PLACE R POINTER FINGER ON LOWER RIGHT CORNER OF KEYBOARD  %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
    print('%%               HOLD FINGER THERE UNTIL INSTRUCTED            %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
    print('%%                PRESS ENTER WHEN READY TO BEGIN              %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    input()
    print('\n' * 500)
    lowerRightCoords = perimeterHelper(cap, detector, rightHand)
    perimeterArray[4] = lowerRightCoords[0]
    perimeterArray[5] = lowerRightCoords[1]
    rightHand = False
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    print('%% PLEASE USE LEFT HAND POINTER FINGER FOR THE NEXT TWO POINTS %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
    print('%%   PLACE L POINTER FINGER ON UPPER LEFT CORNER OF KEYBOARD   %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
    print('%%               HOLD FINGER THERE UNTIL INSTRUCTED            %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
    print('%%                PRESS ENTER WHEN READY TO BEGIN              %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    input()
    print('\n' * 500)
    upperLeftCoords = perimeterHelper(cap, detector, rightHand)
    perimeterArray[0] = upperLeftCoords[0]
    perimeterArray[1] = upperLeftCoords[1]
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
    print('%%    PLACE L POINTER FINGER ON LOWER LEFT CORNER OF KEYBOARD  %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
    print('%%               HOLD FINGER THERE UNTIL INSTRUCTED            %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
    print('%%                PRESS ENTER WHEN READY TO BEGIN              %%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
    input()
    print('\n' * 500)
    lowerLeftCoords = perimeterHelper(cap, detector, rightHand)
    perimeterArray[2] = lowerLeftCoords[0]
    perimeterArray[3] = lowerLeftCoords[1]
    return perimeterArray


def blackKeyGeneration(cap, perimeter):
    # https://techvidvan.com/tutorials/detect-objects-of-similar-color-using-opencv-in-python/
    success, img = cap.read()
    while(np.size(img) == 0):
        success, img = cap.read()
        
    # convert to hsv colorspace
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    
    # lower bound and upper bound for Black color, assuming white background
    lower_bound = np.array([0, 0, 0])   
    upper_bound = np.array([160, 160, 130])
    
    # find the colors within the boundaries
    mask = cv2.inRange(hsv, lower_bound, upper_bound)
    
    #define kernel size, 3x3 for low res black keys  
    kernel = np.ones((3,3),np.uint8)
    
    # Remove unnecessary noise from mask
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
    
    # Segment only the detected region, for display
    segmented_img = cv2.bitwise_and(img, img, mask=mask)
    
    # Find contours from the mask
    contours, hierarchy = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    # Determining if contours are within the perimeter of the keyboard, have sufficient area
    blackKeys = []
    for blackKey in contours:
        sampPoint = (int(blackKey[0][0][0]), int(blackKey[0][0][1]))
        inContour = cv2.pointPolygonTest(perimeter[0], sampPoint, False)
        if(inContour >= 0):
            if(cv2.contourArea(blackKey) > blackKeyMinArea):
                blackKeys.append(blackKey)
    
    # Checking if correct number of key contours were found
    print("Black Keys Detected: " + str(len(blackKeys)))

    if(len(blackKeys) != 36):
        output = cv2.drawContours(segmented_img, blackKeys, -1, (0, 0, 255), 3) # removed for console mode
        cv2.imshow("Failed Black Key Output", output)  # removed for console mode
        cv2.waitKey(1)
        print("***Error Occured*** Black Key Detection Error") # Black Key Detection Error
    return blackKeys, segmented_img


# Given a picture from the USB camera, this function generates the expected piano key
# locations and returns them in the keyContours data structure. As an 88-key piano is
# expected, the keyContours structure will have 88 entries.
def expectedKeyGeneration(cap, detector):    
    # Perimeter Calibration
    perimeterArray = [0, 0, 0, 0, 0, 0, 0, 0]
    perimeterResponse = "no"
    lowerLeftX = 0
    lowerLeftY = 0
    upperLeftX = 0
    upperLeftY = 0
    lowerRightX = 0
    lowerRightY = 0
    upperRightX = 0
    upperRightY = 0

    while(perimeterResponse != "yes" and perimeterResponse != "YES"):
        perimeterArray = touchPerimeter(cap, detector)
        for i in range(len(perimeterArray)):
            perimeterArray[i] = int(perimeterArray[i])
        
        success, img = cap.read()
        while(np.size(img) == 0):
            success, img = cap.read()
    
        print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
        print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
        print('%%         DO THE PERIMETER CORNER POINTS APPEAR CORRECT?      %%')
        print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
        print('%%                 TYPE yes IF POINTS ARE CORRECT              %%')
        print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')    
        print('%%                        TYPE no OTHERWISE                    %%')
        print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
        print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
        
        lowerLeftX = perimeterArray[2]
        lowerLeftY = perimeterArray[3]
        upperLeftX = perimeterArray[0]
        upperLeftY = perimeterArray[1]
        lowerRightX = perimeterArray[4]
        lowerRightY = perimeterArray[5]
        upperRightX = perimeterArray[6]
        upperRightY = perimeterArray[7]
    
        print("Upper Left X: " + str(upperLeftX))
        print("Upper Left Y: " + str(upperLeftY))
        print("Lower Left X: " + str(lowerLeftX))
        print("Lower Left Y: " + str(lowerLeftY))
        print("Lower Right X: " + str(lowerRightX))
        print("Lower Right Y: " + str(lowerRightY))
        print("Upper Right X: " + str(upperRightX))
        print("Upper Right Y: " + str(upperRightY))

        perimeterResponse = input()
    print('\n' * 500)
      
    # perimeterArray: [upperLeftx, upperLefty, lowerLeftx, lowerLefty, lowerRightx, lowerRighty, upperRightx, upperRighty]
    # Define perimeter corner coordinates:
    lowerLeftX = perimeterArray[2]
    lowerLeftY = perimeterArray[3]
    upperLeftX = perimeterArray[0]
    upperLeftY = perimeterArray[1]
    lowerRightX = perimeterArray[4]
    lowerRightY = perimeterArray[5]
    upperRightX = perimeterArray[6]
    upperRightY = perimeterArray[7]

    # Defined for construction of the perimeter contour
    lowerLeft = [lowerLeftX, lowerLeftY]
    lowerRight = [lowerRightX, lowerRightY]

    # Calculating radial distortion for the perimeter contour
    xLength = lowerLeftX - lowerRightX
    yLength = upperLeftY - lowerLeftY
    xDiff = xLength / 6
    xp1 = lowerRightX + xDiff # Roughly 22% y distortion based on expected alignment
    xp2 = lowerRightX + 2*xDiff # Roughly 33% y distortion based on expected alignment
    xp3 = lowerRightX + 3*xDiff # Roughly 37% y distortion based on expected alignment
    xp4 = lowerRightX + 4*xDiff # Roughtly 33% y distortion based on expected alignment
    xp5 = lowerRightX + 5*xDiff # Roughly 22% y distortion based on expected alignment

    p1 = [xp1, lowerLeftY - yLength*0.30]
    p2 = [xp2, lowerLeftY - yLength*0.40]
    p3 = [xp3, lowerLeftY - yLength*0.44]
    p4 = [xp4, lowerLeftY - yLength*0.40]
    p5 = [xp5, lowerLeftY - yLength*0.28]
    
    perimeter = [np.array([[upperLeftX, upperLeftY + 5], [upperRightX, upperRightY + 5], lowerRight, p1, p2, p3, p4, p5, lowerLeft], dtype=np.int32)]

    perimOutput = cv2.drawContours(img, perimeter, -1, (0, 0, 255), 3) 
    cv2.imshow("Perimeter Output", perimOutput)  
    cv2.waitKey(1)      
      
    # Finding the Black Key Contours
    blackKeys, segmented_img = blackKeyGeneration(cap, perimeter)
    while(len(blackKeys) != 36):
        blackKeys, segmented_img = blackKeyGeneration(cap, perimeter)
        
    # Normalizing array of black keys
    normalizedBlackKeys = sorted(blackKeys, key=lambda x: x[0][0][0], reverse=True)
   
    # Showing the output, debugging purposes
    #output = cv2.drawContours(segmented_img, normalizedBlackKeys, -1, (0, 0, 255), 3) # removed for console mode
    #cv2.imshow("Black Key Output", output)  # removed for console mode
        
    whiteKeyBoundaries = [0] * 51
    blackKeyGivens = [0, 2, 3, 5, 6, 7, 9, 10, 12, 13, 14, 16, 17, 19, 20, 21, 23, 24, 26, 27, 28, 30, 31, 33, 34, 35, 37, 38, 40, 41, 42, 44, 45, 47, 48, 49]
        
    # Determining white key pixel boundaries based on black key contours
    iterator = 0
    for key in normalizedBlackKeys:
        maxX = 0
        minX = 0
        for coord in key:
            if(maxX == 0):
                maxX = coord[0][0]
                minX = coord[0][0]
            if(coord[0][0] > maxX):
                maxX = coord[0][0]
            elif(coord[0][0] < minX):
                minX = coord[0][0]
        xp = (maxX + minX)/2
        whiteKeyIndex = blackKeyGivens[iterator]
        whiteKeyBoundaries[whiteKeyIndex] = xp
        iterator += 1
        
    # Determining remaining white key pixel boundaries by averaging known locations
    iterator = 0
    for bound in whiteKeyBoundaries:
        if(iterator != 50 and bound == 0):
            whiteKeyBoundaries[iterator] = (whiteKeyBoundaries[iterator - 1] + whiteKeyBoundaries[iterator + 1])/2
        iterator += 1
    whiteKeyBoundaries[50] = (whiteKeyBoundaries[49] + lowerRightX)/2
        
    # Determining bottom points of white key contours
    bottomWhiteKeyCoordinates = []
    for i in range(0, 53):
        if(i == 0):
            bottomWhiteKeyCoordinates.append([upperLeftX, upperRightY])
        elif(i == 52):
            bottomWhiteKeyCoordinates.append([upperRightX, upperRightY])
        else:
            bottomWhiteKeyCoordinates.append([whiteKeyBoundaries[i - 1], upperRightY])
        
    # Determining top points of white key contours
    topWhiteKeyCoordinates = []
    for j in range(0, 53):
        if(j == 0):
            topWhiteKeyCoordinates.append([lowerLeftX, lowerRightY])
        elif(j == 52):
            topWhiteKeyCoordinates.append([lowerRightX, lowerRightY])
        else:
            baseline = int(lowerRightY) + 5
            while True:
                sampPoint = (int(whiteKeyBoundaries[j - 1]), baseline)
                inContour = cv2.pointPolygonTest(perimeter[0], sampPoint, False)
                if(inContour == 1):
                    baseline = baseline - 1
                else:
                    break
            topWhiteKeyCoordinates.append([whiteKeyBoundaries[j - 1], baseline])
        
    # Drawing white key contours
    whiteKeys = []
    for k in range(0, 52):
        whiteKeys.append(np.array([topWhiteKeyCoordinates[k], topWhiteKeyCoordinates[k + 1], bottomWhiteKeyCoordinates[k + 1], bottomWhiteKeyCoordinates[k]], dtype=np.int32))

    # Showing the output, debugging purposes
    #whiteKeyOutput = cv2.drawContours(img, whiteKeys, -1, (0, 0, 255), 3) # removed for console mode
    #cv2.imshow("White Key Output", whiteKeyOutput)  # removed for console mode
        
    return whiteKeys, normalizedBlackKeys, whiteKeyBoundaries
            

# lmDict structure, for converting hand and lm number to lmLocations entry
lmDict = {
    (1, 20): 0,
    (1, 16): 1,
    (1, 12): 2,
    (1, 8): 3,
    (1, 4): 4,
    (2, 4): 5,
    (2, 8): 6,
    (2, 12): 7,
    (2, 16): 8,
    (2, 20): 9
}

# applicableBlack structure, converts pos in whiteKeyBoundaries to applicable black keys to be checked
applicableBlack = {
    51 : [0],
    50 : [0],
    49 : [1],
    48 : [1, 2],
    47 : [2],
    46 : [3],
    45 : [3, 4],
    44 : [4, 5],
    43 : [5],
    42 : [6],
    41 : [6, 7],
    40 : [7],
    39 : [8],
    38 : [8, 9],
    37 : [9, 10],
    36 : [10],
    35 : [11],
    34 : [11, 12],
    33 : [12],
    32 : [13],
    31 : [13, 14],
    30 : [14, 15],
    29 : [15],
    28 : [16],
    27 : [16, 17],
    26 : [17],
    25 : [18],
    24 : [18, 19],
    23 : [19, 20],
    22 : [20],
    21 : [21],
    20 : [21, 22],
    19 : [22],
    18 : [23],
    17 : [23, 24],
    16 : [24, 25],
    15 : [25],
    14 : [26],
    13 : [26, 27],
    12 : [27],
    11 : [28],
    10 : [28, 29],
    9 : [29, 30],
    8 : [30],
    7 : [31],
    6 : [31, 32],
    5 : [32],
    4 : [33],
    3 : [33, 34],
    2 : [34, 35],
    1 : [35]
}

# masterKey structure, converts a given white or black key to its absolute key number
masterKey = {
    (0, -1) : 0,
    (1, -1) : 2,
    (2, -1) : 3,
    (3, -1) : 5,
    (4, -1) : 7,
    (5, -1) : 8,
    (6, -1) : 10,
    (7, -1) : 12,
    (8, -1) : 14,
    (9, -1) : 15,
    (10, -1) : 17,
    (11, -1) : 19,
    (12, -1) : 20,
    (13, -1) : 22,
    (14, -1) : 24,
    (15, -1) : 26,
    (16, -1) : 27,
    (17, -1) : 29,
    (18, -1) : 31,
    (19, -1) : 32,
    (20, -1) : 34,
    (21, -1) : 36,
    (22, -1) : 38,
    (23, -1) : 39,
    (24, -1) : 41,
    (25, -1) : 43,
    (26, -1) : 44,
    (27, -1) : 46,
    (28, -1) : 48,
    (29, -1) : 50,
    (30, -1) : 51,
    (31, -1) : 53,
    (32, -1) : 55,
    (33, -1) : 56,
    (34, -1) : 58,
    (35, -1) : 60,
    (36, -1) : 62,
    (37, -1) : 63,
    (38, -1) : 65,
    (39, -1) : 67,
    (40, -1) : 68,
    (41, -1) : 70,
    (42, -1) : 72,
    (43, -1) : 74,
    (44, -1) : 75,
    (45, -1) : 77,
    (46, -1) : 79,
    (47, -1) : 80,
    (48, -1) : 82,
    (49, -1) : 84,
    (50, -1) : 86,
    (51, -1) : 87,
    (-1, 0) : 1,
    (-1, 1) : 4,
    (-1, 2) : 6,
    (-1, 3) : 9,
    (-1, 4) : 11,
    (-1, 5) : 13,
    (-1, 6) : 16,
    (-1, 7) : 18,
    (-1, 8) : 21,
    (-1, 9) : 23,
    (-1, 10) : 25,
    (-1, 11) : 28,
    (-1, 12) : 30,
    (-1, 13) : 33,
    (-1, 14) : 35,
    (-1, 15) : 37,
    (-1, 16) : 40,
    (-1, 17) : 42,
    (-1, 18) : 45,
    (-1, 19) : 47,
    (-1, 20) : 49,
    (-1, 21) : 52,
    (-1, 22) : 54,
    (-1, 23) : 57,
    (-1, 24) : 59,
    (-1, 25) : 61,
    (-1, 26) : 64,
    (-1, 27) : 66,
    (-1, 28) : 69,
    (-1, 29) : 71,
    (-1, 30) : 73,
    (-1, 31) : 76,
    (-1, 32) : 78,
    (-1, 33) : 81,
    (-1, 34) : 83,
    (-1, 35) : 85
}


# Given the white and black structures, detailing the expected piano key pixel locations, and
# the lmList structure, detailing the fingertip pixel locations of the user, this function
# will return the lmLocations integer array, matching each finger to a piano key (denoted
# 0 - 87) or 88 if the user's finger is not detected to be over a particular piano key.
def matchLocations(white, black, whiteKeyBoundaries, lmList):
    lmLocations = [88, 88, 88, 88, 88, 88, 88, 88, 88, 88] # All landmark locations initalized to error code
    if(len(lmList) > 0):
        whiteKeyBoundaries = sorted(whiteKeyBoundaries, reverse=False)
        for lm in lmList:
            pos = bisect.bisect_left(whiteKeyBoundaries, lm[2])
            sampPoint = (lm[2], lm[3])
            if(pos == 0):
                inContour = cv2.pointPolygonTest(white[51], sampPoint, False)
                if(inContour >= 0):
                    dictConv = (lm[0], lm[1])
                    locationIndex = lmDict[dictConv]
                    lmLocations[locationIndex] = 87
            else:
                blackToCheck = applicableBlack[pos]
                isntBlack = True
                for key in blackToCheck:
                    assert isinstance(key, int)
                    if key >= len(black):
                        key = len(black)-1
                    inContour = cv2.pointPolygonTest(black[key], sampPoint, False)
                    if(inContour >= 0):
                        dictConv = (lm[0], lm[1])
                        locationIndex = lmDict[dictConv]
                        keyConv = (-1, key)
                        keyNum = masterKey[keyConv]
                        lmLocations[locationIndex] = keyNum
                        isntBlack = False
                        break
                if(isntBlack):
                    whiteIndex = 51 - pos
                    inContour = cv2.pointPolygonTest(white[whiteIndex], sampPoint, False)
                    if(inContour >= 0):
                        dictConv = (lm[0], lm[1])
                        locationIndex = lmDict[dictConv]
                        keyConv = (whiteIndex, -1)
                        keyNum = masterKey[keyConv]
                        lmLocations[locationIndex] = keyNum
    return lmLocations
 

# Interrupt function, transmits the data in currentlmLocations to the MCU at ~10 Hz
def BleTransmit(currentlmLocations):
    toTransmit = ''
    print(currentlmLocations)
    #"""
    for lm in currentlmLocations:
        toTransmit += chr(lm)
    toTransmit = bytes(toTransmit, "utf-8")
    try:
        BLEVar.write(toTransmit)
    except (btle.BTLEDisconnectError, btle.BTLEInternalError):
        establishBLE()
    #"""
        
    
# Main function
def main():
    # Bluetooth Initialization 
    establishBLE()
    
    # Camera Initialization
    cap = initCamera()
    print("Camera Initialization Complete")
    time.sleep(2)

    # Mediapipe Initialization
    detector = handDetector()
    print("Mediapipe Initialization Complete")
    time.sleep(2)

    # Expected Key Location Generation
    white, black, whiteKeyBoundaries = expectedKeyGeneration(cap, detector)
    print("Expected Keyboard Generation Complete")

    # Variable Initializations
    pTime = 0
    cTime = 0
    sumFPS = 0
    countFPS = 0
    prevFPS = 0
    
    # Main processing loop
    while True:
        # Reading of camera, finding position of hands/fingers
        success, img = cap.read()
        if np.size(img) > 0:
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
        
            # Match fingertip lanmark locations to expected key locations
            currentlmLocations = matchLocations(white, black, whiteKeyBoundaries, lmList)
            
            # BLE Transmission Logic
            threading.Timer(0, lambda: BleTransmit(currentlmLocations)).start()
            
            # Computer vision pipeline output, mainly for debugging purposes
            #cv2.imshow("Live Image", img)  # removed for console mode
            cv2.waitKey(1)

if __name__ == '__main__':
    main()
