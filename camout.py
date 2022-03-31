import cv2
import numpy as np
camera = cv2.VideoCapture(0)
camera.set(cv2.CAP_PROP_FRAME_WIDTH, 1920) # x
camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 700) # y
# Note: camera display of piano keyboard intended to NOT be inverted
camera.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'))
camera.set(cv2.CAP_PROP_FPS, 60)
print("Camera Initialized")
perimeter = np.load("perimeter.npy")
while True:
    success, img = camera.read()
    perimOutput = cv2.drawContours(img, perimeter, -1, (0, 0, 255), 3)
    cv2.imshow("Calibration Image", perimOutput)
    cv2.waitKey(1)
