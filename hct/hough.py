import sys, string, os
import cv2
import cv2.cv
import numpy as np

if (len(sys.argv) == 3):
    imageFile = sys.argv[1]
    parameter = float(sys.argv[2])
else:
    imageFile = "balls.jpg"
    parameter = 72

visual = True


img = cv2.imread(imageFile,0)
#img = cv2.medianBlur(img,3) #medBlur(5) was probably too strong
img = cv2.blur(img, (3, 3))  #meanBlur is closer to what we want
cimg = cv2.cvtColor(img,cv2.COLOR_GRAY2BGR)

circles = cv2.HoughCircles(img,cv2.cv.CV_HOUGH_GRADIENT,1,60,
                            param1=40,param2=parameter,minRadius=80,maxRadius=0)

if visual:
    circles = np.uint16(np.around(circles))
    for i in circles[0,:]:
        # draw the outer circle
        cv2.circle(cimg,(i[0],i[1]),i[2],(0,255,0),2)
        # draw the center of the circle
        cv2.circle(cimg,(i[0],i[1]),2,(0,0,255),3)

    cv2.imshow('detected circles',cimg)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
