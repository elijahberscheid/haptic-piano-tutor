# Patch created on 4/23/22 to remove the need for green retroreflective tape
# 
# Finger coordinates will now be used as the user is prompted to point to the 
# corners of the keyboard in a specified sequence

import cv2
import mediapipe as mp
import numpy as np
import time

def perimeterHelper(cap, detector, rightHand):
    coords = []
    timerCount = 10
    failCount = 0
    xSum = 0
    ySum = 0
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
                    if(lmCoord[1] == 8 and lmCoord[0] == 2):
                        xSum = xSum + lmCoord[2]
                        ySum = ySum + lmCoord[3]
                        lmFound = True
                        failCount = 0
            else:
                for lmCoord in lmList:
                    # left pointer fingertip
                    if(lmCoord[1] == 8 and lmCoord[0] == 1):
                        xSum = xSum + lmCoord[2]
                        ySum = ySum + lmCoord[3]
                        lmFound = True
                        failCount = 0
            if(lmFound):
                if(timerCount == 10):
                    print('%%                                ' + str(timerCount) + '                            %%')
                else:
                    print('%%                                 ' + str(timerCount) + '                            %%')    
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
    coords.append(xSum/10)
    coords.append(ySum/10)
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