import cv2
import numpy as np

camX = 1920
camY = 700
blackKeyMinArea = 300

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


def blackKeyGeneration(cap):
    # https://techvidvan.com/tutorials/detect-objects-of-similar-color-using-opencv-in-python/
    success, img = cap.read()
    while(np.size(img) == 0):
        success, img = cap.read()
        
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
    
    # Segment only the detected region, for display
    segmented_img = cv2.bitwise_and(img, img, mask=mask)
    
    # Find contours from the mask
    contours, hierarchy = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    # Determining if contours are within the perimeter of the keyboard, have sufficient area
    blackKeys = []
    for blackKey in contours:
        sampPoint = (int(blackKey[0][0][0]), int(blackKey[0][0][1]))
        inContour = 1
        if(inContour >= 0):
            if(cv2.contourArea(blackKey) > blackKeyMinArea):
                blackKeys.append(blackKey)
    
    # Checking if correct number of key contours were found
    print("Black Keys Detected: " + str(len(blackKeys)))

    if(len(blackKeys) != 36):
        print("***Error Occured*** Black Key Detection Error") # Black Key Detection Error
    return blackKeys, segmented_img


cap = initCamera()
blackKeys, segmented_img = blackKeyGeneration(cap)
output = cv2.drawContours(segmented_img, blackKeys, -1, (0, 0, 255), 3) # removed for console mode
cv2.imshow("Failed Black Key Output", output)  # removed for console mode
cv2.waitKey(1)