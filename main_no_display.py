#!/usr/bin/env python3.8


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
upperYLimit = .5*camY
maxDeltaY = 250
tapeBoxAreaL = 100
tapeBoxAreaH = 500
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

def removeND(L, arr):
    ind = 0
    size = len(L)
    while ind != size and not np.array_equal(L[ind], arr):
        ind += 1
    if ind != size:
        L.pop(ind)

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
        
        # Values for Sample Image (ECE 477 Lab Setting)
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
        #cv2.imshow("Calibration Output", output)  # removed for console mode
        
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
                # Added 3/22/22: Filtering based on Y pixel location
                yCheck = []
                for ychSample in sampleContour:
                    sampleY = ychSample[0][0][1]
                    if(sampleY < upperYLimit):
                        yCheck.append(ychSample)
                if(len(yCheck) < 4):
                    BleTransmitError(6)
                    print("***Error Occured*** tapeCalibration Tape Detection Error Type 2.1") # Landmarks not in expected y range
                else:
                    sampleAreas = []
                    # Filtering based on contour area
                    for sample in yCheck:
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
                                    removeND(sampleAreas, refined)
                                    removeND(sampleAreas, refined2)
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
                                     sample2Y = superRefined2[0][0][1]
                                     difY = abs(sampleY - sample2Y)
                                     if(difY < yTolerance and superRefined[0][0][0] != superRefined2[0][0][0]):
                                         sampleSimilarY.append(superRefined)
                                         sampleSimilarY.append(superRefined2)
                                         removeND(sampleSimilar, superRefined)
                                         removeND(sampleSimilar, superRefined2)
                                         break
                            if(len(sampleSimilarY) < 4):
                                BleTransmitError(4)
                                print("***Error Occured*** tapeCalibration Tape Detection Error Type 5") # Keyboard or camera lens needs straightening 
                            else:
                                # Final check, filtering based on maximum y seperation of landmarks
                                lowestY = camY + 1
                                highestY = -1
                                for refinedCubed in sampleSimilarY:
                                    sampleY = refined[0][0][1]
                                    if(sampleY < lowestY):
                                        lowestY = sampleY
                                    if(sampleY > highestY):
                                        highestY = sampleY
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
    success, img = cap.read()
    for _ in range(10):
        success, img = cap.read()
    #img = np.load("sampleImg.npy")
    #cv2.imshow("Calibration Image 2", img)  # removed for console mode
    
    # Tape Calibration
    tapeContours = tapeCalibration(img)
    if(len(tapeContours) != 4):
        while(len(tapeContours) != 4):
            BleTransmitError(7)
            print("***Error Occured*** expectedKeyGeneration Tape Detection Error Type 7") # Too much noise, remove other green objects
            success, img = cap.read()
            tapeContours = tapeCalibration(img)
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
        if(rcoordinate[1] < lowerLeftY):
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
        if(rcoordinate[1] < lowerRightY):
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
        
    print("KEYBOARD PERIMETER CORNER POINTS:")
    print("True upperLeftX: x: " + str(upperLeftX))
    print("True upperLeftY: y: " + str(upperLeftY))
    print("True lowerLeftX: x: " + str(lowerLeftX))
    print("True lowerLeftY: y: " + str(lowerLeftY))
    print("True lowerRightX: x: " + str(lowerRightX))
    print("True lowerRightY: y: " + str(lowerRightY))
    print("True upperRightX: x: " + str(upperRightX))
    print("True upperRightY: y: " + str(upperRightY))

    upperLeft = [upperLeftX, upperLeftY]
    upperRight = [upperRightX, upperRightY]
    lowerLeft = [lowerLeftX, lowerLeftY]
    lowerRight = [lowerRightX, lowerRightY]

    xLength = lowerLeftX - lowerRightX
    yLength = upperLeftY - lowerLeftY
    xDiff = xLength / 6
    xp1 = lowerRightX + xDiff # Roughly 22% y distortion
    # (289.5, 121)
    xp2 = lowerRightX + 2*xDiff # Roughly 33% y distortion
    # (472, 106)
    xp3 = lowerRightX + 3*xDiff # Roughly 37% y distortion 
    # (654.5, 101) 
    xp4 = lowerRightX + 4*xDiff # Roughtly 33% y distortion
    # (837, 108) 
    xp5 = lowerRightX + 5*xDiff # Roughly 22% y distortion
    # (1019.5, 122)

    p1 = [xp1, lowerLeftY - yLength*0.30]
    p2 = [xp2, lowerLeftY - yLength*0.40]
    p3 = [xp3, lowerLeftY - yLength*0.44]
    p4 = [xp4, lowerLeftY - yLength*0.40]
    p5 = [xp5, lowerLeftY - yLength*0.28]
    
    #p6 = [xp1, upperLeftY - yLength*0.17]
    #p7 = [xp2, upperLeftY - yLength*0.21]
    #p8 = [xp3, upperLeftY - yLength*0.26]
    #p9 = [xp4, upperLeftY - yLength*0.21]
    #p0 = [xp5, upperLeftY - yLength*0.17]

    perimeter = [np.array([[upperLeftX, upperLeftY + 5], [upperRightX, upperRightY + 5], lowerRight, p1, p2, p3, p4, p5, lowerLeft], dtype=np.int32)]
    #np.save("perimeter.npy", perimeter)

    perimOutput = cv2.drawContours(img, perimeter, -1, (0, 0, 255), 3)
    # Showing the output, debugging purposes
    #cv2.imshow("Perimeter Output", perimOutput)  # removed for console mode
      
    # Find Black Key Contours
    # https://techvidvan.com/tutorials/detect-objects-of-similar-color-using-opencv-in-python/
    # convert to hsv colorspace
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    
    # lower bound and upper bound for Black color, assuming white background
    lower_bound = np.array([0, 0, 0])   
    upper_bound = np.array([160, 160, 90])
    
    # find the colors within the boundaries
    mask = cv2.inRange(hsv, lower_bound, upper_bound)
    
    #define kernel size, 3x3 for low res black keys  
    kernel = np.ones((3,3),np.uint8)
    
    # Remove unnecessary noise from mask
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
    
    # Segment only the detected region
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
    print(len(blackKeys))
    if(len(blackKeys) != 36):
        output = cv2.drawContours(segmented_img, blackKeys, -1, (0, 0, 255), 3)
        #cv2.imshow("Black Key Output", output)  # removed for console mode
        BleTransmitError(7)
        print("***Error Occured*** Black Key Detection Error") # Black Key Detection Error
        return [0],[0],[0]
    else:
        # Normalizing array of black keys
        normalizedBlackKeys = sorted(blackKeys, key=lambda x: x[0][0][0], reverse=True)

        output = cv2.drawContours(segmented_img, normalizedBlackKeys, -1, (0, 0, 255), 3)
    
        # Showing the output, debugging purposes
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

        # Brightness Adjustment of Sample Image
        #for row in img:
        #    for col in row:
        #        col[1] = col[1] - 70

        whiteKeyOutput = cv2.drawContours(img, whiteKeys, -1, (0, 0, 255), 3)
        # Showing the output, debugging purposes
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

# Interrupt function, transmits the data in currentlmLocations to the MCU at ~10 Hz
def BleTransmit(currentlmLocations, btcnt):
    toTransmit = ''
    print(currentlmLocations)
    #"""
    for lm in currentlmLocations:
        toTransmit += chr(lm)
    toTransmit = bytes(toTransmit, "utf-8")
    if(True or btcnt % 3 != 0): # TODO: can delete this delay
        try:
            BLEVar.write(toTransmit)
        except (btle.BTLEDisconnectError, btle.BTLEInternalError):
            GPIO.output(18, 0)
            establishBLE()
    #"""

# Called upon to transmit an error packet in the event of a calibration error
def BleTransmitError(code):
    # USB Camera Error (0)
    if(code == 0):
        print("error 0")
        try:
            BLEVar.write(bytes("error code 0", "utf-8"))
        except (btle.BTLEDisconnectError, btle.BTLEInternalError):
            GPIO.output(18, 0)
            establishBLE()
    # Retroreflective Tape Detection Error - Tape likely blocked (1)
    elif(code == 1):
        print("error 1")
        try:
            BLEVar.write(bytes("error code 1", "utf-8"))
        except (btle.BTLEDisconnectError, btle.BTLEInternalError):
            GPIO.output(18, 0)
            establishBLE()
    # Retroreflective Tape Detection Error - Tape landmarks not in expected x range, keyboard needs horizontal adjustment, camera may be too close/far
    elif(code == 2):
        print("error 2")
        try:
            BLEVar.write(bytes("error code 2", "utf-8"))
        except (btle.BTLEDisconnectError, btle.BTLEInternalError):
            GPIO.output(18, 0)
            establishBLE()
    # Retroreflective Tape Detection Error - USB Camera too close or far
    elif(code == 3):
        print("error 3")
        try:
            BLEVar.write(bytes("error code 3", "utf-8"))
        except (btle.BTLEDisconnectError, btle.BTLEInternalError):
            GPIO.output(18, 0)
            establishBLE()
    # Retroreflective Tape Detection Error - Keyboard or camera lense needs straightening 
    elif(code == 4):
        print("error 4")
        try:
            BLEVar.write(bytes("error code 4", "utf-8"))
        except (btle.BTLEDisconnectError, btle.BTLEInternalError):
            GPIO.output(18, 0)
            establishBLE()
    # Retroreflective Tape Detection Error - Too much noise present, remove other green objects from USB camera frame 
    elif(code == 5):
        print("error 5")
        try:
            BLEVar.write(bytes("error code 5", "utf-8"))
        except (btle.BTLEDisconnectError, btle.BTLEInternalError):
            GPIO.output(18, 0)
            establishBLE()
    # Retroreflective Tape Detection Error - Tape landmarks not in expected y range, camera lens may be inverted, keyboard may need vertical adjustment
    elif(code == 6):
        print("error 6")
        try:
            BLEVar.write(bytes("error code 6", "utf-8"))
        except (btle.BTLEDisconnectError, btle.BTLEInternalError):
            GPIO.output(18, 0)
            establishBLE()
    # Black key detection error
    elif(code == 7):
        print("error 7")
        try:
            BLEVar.write(bytes("error code 7", "utf-8"))
        except (btle.BTLEDisconnectError, btle.BTLEInternalError):
            GPIO.output(18, 0)
            establishBLE()
        
# Main function
def main():
    # Bluetooth Initialization 
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(18, GPIO.OUT)
    GPIO.output(18, 0) # Ensure blue LED is off
    establishBLE()
    
    # Camera Initialization
    cap = initCamera()

    # Expected Key Location Generation
    white, black, whiteKeyBoundaries = expectedKeyGeneration(cap)
    while len(white) == 1:
        white, black, whiteKeyBoundaries = expectedKeyGeneration(cap)

    print("Expected Keyboard Generation Complete")

    # Variable Initializations
    pTime = 0
    cTime = 0
    sumFPS = 0
    countFPS = 0
    prevFPS = 0
    detector = handDetector()
    
    # Main processing loop
    btcnt = 0
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
            threading.Timer(0, lambda: BleTransmit(currentlmLocations, btcnt)).start()
            btcnt = btcnt + 1
            
            # Computer vision pipeline output, mainly for debugging purposes
            #cv2.imshow("Image", img)  # removed for console mode
            cv2.waitKey(1)

if __name__ == '__main__':
    main()

