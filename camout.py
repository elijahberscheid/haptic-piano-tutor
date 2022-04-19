#!/usr/bin/env python3.8


import cv2
import numpy as np
#scale = 0.5
camera = cv2.VideoCapture(0)
camera.set(cv2.CAP_PROP_FRAME_WIDTH, (1920)) # x
camera.set(cv2.CAP_PROP_FRAME_HEIGHT, (700)) # y
# Note: camera display of piano keyboard intended to NOT be inverted
camera.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'))
camera.set(cv2.CAP_PROP_FPS, 10)
print("Camera Initialized")
perimeter = np.load("perimeter.npy")
#perimeter = perimeter * scale
windowStatus = 1
while windowStatus > 0:
    success, img = camera.read()
    perimOutput = cv2.drawContours(img, perimeter, -1, (0, 0, 255), 3)
    #perimOutput = cv2.drawContours(img, perimeter.astype(int), -1, (0, 0, 255), 3)
    cv2.imshow("Calibration Image", perimOutput)
    cv2.waitKey(1)
    windowStatus = cv2.getWindowProperty('Calibration Image', cv2.WND_PROP_VISIBLE)
