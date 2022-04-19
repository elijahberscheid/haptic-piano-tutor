#!/usr/bin/env python3.8


import cv2
import numpy as np
camera = cv2.VideoCapture(0)
camera.set(cv2.CAP_PROP_FRAME_WIDTH, (1920)) # x
camera.set(cv2.CAP_PROP_FRAME_HEIGHT, (700)) # y
# Note: camera display of piano keyboard intended to NOT be inverted
camera.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'))
camera.set(cv2.CAP_PROP_FPS, 10)
print("Camera Initialized")
#perimeter = np.load("perimeter.npy")
#windowStatus = 1
#while windowStatus > 0:
#    success, img = camera.read()
#    perimOutput = cv2.drawContours(img, perimeter, -1, (0, 0, 255), 3)
#    cv2.imshow("Calibration Image", perimOutput)
#    cv2.waitKey(1)
#    windowStatus = cv2.getWindowProperty('Calibration Image', cv2.WND_PROP_VISIBLE)


while 1:
    success, img = camera.read()

    if np.size(img) > 0:
        # To save an image for testing
        # np.save("sampleImg.npy", img)
        
        # First find green retroreflective tape coordinates
        # https://techvidvan.com/tutorials/detect-objects-of-similar-color-using-opencv-in-python/
        # convert to hsv colorspace
        hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
        
        # lower bound and upper bound for Green color
        lower_bound = np.array([30, 88, 15])   
        upper_bound = np.array([120, 255, 255])
        
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
        print("contours:", len(contours))
        
        # Print contours, debugging purposes
        output = cv2.drawContours(segmented_img, contours, -1, (0, 0, 255), 3)
        
        # Showing the output, debugging purposes
        cv2.imshow("Calibration Output", output)
        cv2.waitKey(1)

