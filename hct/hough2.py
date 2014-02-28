import sys, string, os
import cv2
import cv2.cv
import numpy as np
import time

if (len(sys.argv) == 3):
    imageFile = sys.argv[1]
    parameter = float(sys.argv[2])
else:
    imageFile = "balls.jpg"
    parameter = 90 # 72 works for all but one ball

visual = True
stressNum = 10


img = cv2.imread(imageFile,0)

start = time.time()
for i in range(0, stressNum):
    cimg = img
    cimg = cv2.blur(cimg, (3, 3))
    cimg = cv2.cvtColor(cimg,cv2.COLOR_GRAY2BGR)
    circles = None
    
    # Params for HC:
    # image, method, dp, minDist,
    #   upper threshold of edge detector, accumulator threshold for circle centers, min/max radius

    circles = cv2.HoughCircles(img,cv2.cv.CV_HOUGH_GRADIENT,1,60,
                            param1=40,param2=parameter,minRadius=80,maxRadius=0)
    
print("It took " + str((time.time()-start)/stressNum) + " on average.")

if visual:
    circles = np.uint16(np.around(circles))
    for i in circles[0,:]:
        # draw the outer circle
        cv2.circle(cimg,(i[0],i[1]),i[2],(0,255,0),2)
        # draw the center of the circle
        cv2.circle(cimg,(i[0],i[1]),2,(0,0,255),3)

    cv2.imshow('detected circles',cimg)
    cv2.imwrite("output.jpg", cimg)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
