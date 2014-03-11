'''
Shrink the image to speed up processing while preserving accuracy
Impelement a non-standard way of converting to grayscale based off of the target color of the ball
Perform HCT on the new grayscale image
'''

import sys, string, os
import cv2
import cv2.cv
import numpy as np
import math
import time

if (len(sys.argv) == 2):
    imageFile = "images/magenta" + str(sys.argv[1]) + ".png"
else:
    imageFile = "images/magenta5.png"



visual = True
start = time.time()

src = cv2.imread(imageFile, cv2.CV_LOAD_IMAGE_COLOR)

#constants for the 'perfect' ball color
pRED = 200
pGREEN = 1
pBLUE = 120

scale = 2
img2 = cv2.resize(src, (640/scale, 480/scale))
img2 = cv2.GaussianBlur(img2, (9, 9), 1)

#convert to gray
imgGray = cv2.cvtColor(img2, cv2.COLOR_BGR2GRAY) #REALLY bad, but idk how to create a 2-d array with just a width and height

height, width = imgGray.shape

for i in range(height):
    for j in range(width):
        b = img2.item(i, j, 0)
        g = img2.item(i, j, 1)
        r = img2.item(i, j, 2)
        
        distance = ((b-pBLUE)**2 + (g-pGREEN)**2 + (r-pRED)**2)/3
        distance = int(math.sqrt(distance)) #can shave off 0.03 seconds on 320x240 image without using sqrt
        if (distance > 70):
            distance = 255
        imgGray.itemset((i, j), distance)


print("It took " + str(time.time()-start) + " seconds.")
circles = cv2.HoughCircles(imgGray, cv2.cv.CV_HOUGH_GRADIENT, 4*scale, 200/scale,
                           param1=30,param2=200,minRadius=0/scale,maxRadius=150/scale)


#display
if visual:
    displayImage = src
    if (circles is None):
        print("circles was null or something...")
    else:
        circles = np.uint16(np.around(circles))
        height, width, depth = displayImage.shape
        
        for i in circles[0,:]:
            # draw the outer circle
            cv2.circle(displayImage,(scale * i[0], scale * i[1]), scale * i[2], (0, 255, 0), 2)
            # draw the center of the circle
            cv2.circle(displayImage,(scale * i[0], scale * i[1]), 2, (0, 0, 255), 3)
            #print(str(i[2]))

    cv2.imshow('source image', displayImage)
    cv2.imshow('gray image', imgGray)
    cv2.moveWindow('source image', 0, 25)
    cv2.moveWindow('gray image', width + 10, 25)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
