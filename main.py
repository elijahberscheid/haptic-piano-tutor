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

# Assuming a height of 35" from piano keyboard to camera lens
# Camera is expected 
# Establish resolution dimensions for the USB
camX = 1920
camY = 700
# Calibration Constants (as intended for resolution and camera height)
lowerXLimit = 200
upperXLimitL = 1100
upperXLimitH = 1300
xTolerance = .025*camX
maxDeltaY = 250
tapeBoxAreaL = 120
tapeBoxAreaH = 325

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

# This function, given a still image from the USB camera, returns the four tape contours
def tapeCalibration(img):
    # Structure to return
    tapeContours = []
    if np.size(img) > 0:
        # To save an image for testing
        # np.save("sampleImg.npy", img)
        
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
        output = cv2.drawContours(segmented_img, contours, -1, (0, 0, 255), 3)
        
        # Showing the output, debugging purposes
        cv2.imshow("Calibration Output", output)
        
        # Determining which contours correspond to the tape
        # Algorithm for Tape Detection v2:
        if(len(contours) < 4):
            BleTransmitError(1)
            print("***Error Occured*** tapeCalibration Tape Detection Error Type 1") # Contours not detected, USB Camera or landmarks hidden
        else:
            sampleContour = []
            # Filtering based on X pixel location
            for contour in contours:
                sampleX = contour[0][0][0]
                if(sampleX < lowerXLimit):
                    sampleContour.append(contour)
                elif(sampleX > upperXLimitL and sampleX < upperXLimitH):
                    sampleContour.append(contour)
            if(len(sampleContour) < 4):
                BleTransmitError(2)
                print("***Error Occured*** tapeCalibration Tape Detection Error Type 2") # Landmarks not in expected x range
            else:
                sampleAreas = []
                # Filtering based on contour area
                for sample in sampleContour:
                    samArea = cv2.contourArea(sample)
                    if(samArea < tapeBoxAreaH and samArea > tapeBoxAreaL):
                        sampleAreas.append(sample)
                if(len(sampleAreas) < 4):
                    BleTransmitError(3)
                    print("***Error Occured*** tapeCalibration Tape Detection Error Type 3") # USB Camera too close or far
                else:
                    sampleSimilar = []
                    # Filtering based on similar x location (must have another contour present with similar location)
                    for refined in sampleAreas:
                        sampleX = refined[0][0][0]
                        for refined2 in sampleAreas:
                            sample2X = refined2[0][0][0]
                            difX = abs(sampleX - sample2X)
                            if(difX < xTolerance and refined[0][0][1] != refined2[0][0][1]):
                                sampleSimilar.append(refined)
                                sampleSimilar.append(refined2)
                                sampleAreas.remove(refined)
                                sampleAreas.remove(refined2)
                                break
                    if(len(sampleSimilar) < 4):
                        BleTransmitError(4)
                        print("***Error Occured*** tapeCalibration Tape Detection Error Type 4") # Keyboard or camera lense needs straightening 
                    else:
                        sampleSimilarY = []
                        # Filtering based on similar y location (must have another contour present with similar location)
                        yTolerance = xTolerance
                        for superRefined in sampleSimilar:
                             sampleY = superRefined[0][0][1]
                             for superRefined2 in sampleSimilar:
                                 sample2Y = superRefined[0][0][1]
                                 difY = abs(sampleY - sample2Y)
                                 if(difY < yTolerance and superRefined[0][0][0] != superRefined2[0][0][0]):
                                     sampleSimilarY.append(superRefined)
                                     sampleSimilarY.append(superRefined2)
                                     sampleSimilar.remove(superRefined)
                                     sampleSimilar.remove(superRefined2)
                                     break
                        if(len(sampleSimilarY) < 4):
                            BleTransmitError(4)
                            print("***Error Occured*** tapeCalibration Tape Detection Error Type 5") # Keyboard or camera lense needs straightening 
                        else:
                            # Final check, filtering based on maximum y seperation of landmarks
                            global camY
                            lowestY = camY + 1
                            highestY = -1
                            for refinedCubed in sampleSimilarY:
                                sampleY = refined[0][0][1]
                                if(sampleY < lowestY):
                                    lowestY = sampleY
                                if(sampleY > highestY):
                                    highestY = sampleY
                            print(str(highestY - lowestY))
                            if(highestY - lowestY > maxDeltaY):
                                BleTransmitError(3)
                                print("***Error Occured*** tapeCalibration Tape Detection Error Type 6") # Camera is either too close or too far
                            # If code reaches here, tape has been detected
                            tapeContours = sampleSimilarY
    else:
        BleTrasmitError(0)
        print("***Error Occured*** expectedKeyGeneration USB Camera Error")
    return tapeContours

# Given a picture from the USB camera, this function generates the expected piano key
# locations and returns them in the keyContours data structure. As an 88-key piano is
# expected, the keyContours structure will have 88 entries.
def expectedKeyGeneration(cap):
    keyContours = []
    #success, img = cap.read()
    #for _ in range(10):
    #    success, img = cap.read()
    img = np.load("sampleImg.npy")
    cv2.imshow("Sample Image", img)
    tapeContours = tapeCalibration(img)
    
    if(len(tapeContours) != 4):
        BleTransmitError(5)
        print("***Error Occured*** expectedKeyGeneration Tape Detection Error Type 7") # Too much noise, remove other green objects
    
    # Extract the perimeter corners of the keyboard from the tape contours
    # NOTE: keyboard is expected to be upside down in the USB camera feed
    upperLeftTape = 0
    upperRightTape = 0
    lowerLeftTape = 0
    lowerRightTape = 0
    
    xMidway = 0
    yMidway = 0
    for contour in tapeContours:
        xMidway += contour[0][0][0]
        yMidway += contour[0][0][1]
    xMidway = xMidway/4
    yMidway = yMidway/4
    
    for contour in tapeContours:
        if(contour[0][0][0] < xMidway and contour[0][0][1] < yMidway):
            lowerRightTape = contour
        elif(contour[0][0][0] < xMidway and contour[0][0][1] > yMidway):
            upperRightTape = contour
        elif(contour[0][0][0] > xMidway and contour[0][0][1] < yMidway):
            lowerLeftTape = contour
        elif(contour[0][0][0] > xMidway and contour[0][0][1] > yMidway):
            upperLeftTape = contour
        else:
            BleTransmitError(5)
            print("***Error Occured*** expectedKeyGeneration Tape Detection Error Type 8") # Too much noise, remove other green objects
    
    # Define perimeter corner coordinates:
    lowerLeftX = 0
    lowerLeftY = 0
    upperLeftX = 0
    upperLeftY = 0
    lowerRightX = 0
    lowerRightY = 0
    upperRightX = 0
    upperRightY = 0
    
    for coordinate in lowerLeftTape:
        rcoordinate = coordinate[0]
        if(lowerLeftX == 0):
            lowerLeftX = rcoordinate[0]
        if(lowerLeftY == 0):
            lowerLeftY = rcoordinate[1]
        if(rcoordinate[0] < lowerLeftX):
            lowerLeftX = rcoordinate[0]
        if(rcoordinate[1] > lowerLeftY):
            lowerLeftY = rcoordinate[1]
    
    for coordinate in upperLeftTape:
        rcoordinate = coordinate[0]
        if(upperLeftX == 0):
            upperLeftX = rcoordinate[0]
        if(upperLeftY == 0):
            upperLeftY = rcoordinate[1]
        if(rcoordinate[0] < upperLeftX):
            upperLeftX = rcoordinate[0]
        if(rcoordinate[1] < upperLeftY):
            upperLeftY = rcoordinate[1]
            
    for coordinate in lowerRightTape:
        rcoordinate = coordinate[0]
        if(lowerRightX == 0):
            lowerRightX = rcoordinate[0]
        if(lowerRightY == 0):
            lowerRightY = rcoordinate[1]
        if(rcoordinate[0] > lowerRightX):
            lowerRightX = rcoordinate[0]
        if(rcoordinate[1] > lowerRightY):
            lowerRightY = rcoordinate[1]

    for coordinate in upperRightTape:
        rcoordinate = coordinate[0]
        if(upperRightX == 0):
            upperRightX = rcoordinate[0]
        if(upperRightY == 0):
            upperRightY = rcoordinate[1]
        if(rcoordinate[0] > upperRightX):
            upperRightX = rcoordinate[0]
        if(rcoordinate[1] < upperRightY):
            upperRightY = rcoordinate[1]
            
    print("KEYBOARD CORNER POINTS")
    print("x: " + str(upperLeftX))
    print("y: " + str(upperLeftY))
    print("x: " + str(lowerLeftX))
    print("y: " + str(lowerLeftY))
    print("x: " + str(lowerRightX))
    print("y: " + str(lowerRightY))
    print("x: " + str(upperRightX))
    print("y: " + str(upperRightY))

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
    # USB Camera Error (0)
    if(code == 0):
        print("error 0")
        #try:
        #    writeCharacteristic.write(bytes("error code 0"))
        #except (btle.BTLEDisconnectError, btle.BTLEInternalError):
        #    GPIO.output(18, 0)
        #    establishBLE()
    # Retroreflective Tape Detection Error - Tape likely blocked (1)
    elif(code == 1):
        print("error 1")
        #try:
        #    writeCharacteristic.write(bytes("error code 1"))
        #except (btle.BTLEDisconnectError, btle.BTLEInternalError):
        #    GPIO.output(18, 0)
        #    establishBLE()
    # Retroreflective Tape Detection Error - Tape landmarks not in expected x range, keyboard needs horizontal adjustment, camera may be too close/far
    elif(code == 2):
        print("error 2")
        #try:
        #    writeCharacteristic.write(bytes("error code 2"))
        #except (btle.BTLEDisconnectError, btle.BTLEInternalError):
        #    GPIO.output(18, 0)
        #    establishBLE()
    # Retroreflective Tape Detection Error - USB Camera too close or far
    elif(code == 3):
        print("error 3")
        #try:
        #    writeCharacteristic.write(bytes("error code 3"))
        #except (btle.BTLEDisconnectError, btle.BTLEInternalError):
        #    GPIO.output(18, 0)
        #    establishBLE()
    # Retroreflective Tape Detection Error - Keyboard or camera lense needs straightening 
    elif(code == 4):
        print("error 4")
        #try:
        #    writeCharacteristic.write(bytes("error code 4"))
        #except (btle.BTLEDisconnectError, btle.BTLEInternalError):
        #    GPIO.output(18, 0)
        #    establishBLE()
    # Retroreflective Tape Detection Error - Too much noise present, remove other green objects from USB camera frame 
    elif(code == 5):
        print("error 5")
        #try:
        #    writeCharacteristic.write(bytes("error code 4"))
        #except (btle.BTLEDisconnectError, btle.BTLEInternalError):
        #    GPIO.output(18, 0)
        #    establishBLE()
        
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