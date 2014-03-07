#!/usr/bin/env python
import freenect
import cv
import cv2
import frame_convert
import time
import numpy as np

#cv.NamedWindow('Depth')
#cv.NamedWindow('Video')
print('Press ESC in window to stop')


def get_depth():
    frame = frame_convert.pretty_depth_cv(freenect.sync_get_depth()[0])
    return frame


def get_video():
    frame = freenect.sync_get_video()[0]
    frame = cv2.medianBlur(frame, 5)
    frame = cv2.cvtColor(frame, cv2.COLOR_RGB2GRAY)
    return frame


print "Getting video from camera..."
video = get_video()

#video = cv2.threshold(video,127,255,cv2.THRESH_BINARY)
#video = cv2.adaptiveThreshold(video,255,cv2.ADAPTIVE_THRESH_GAUSSIAN_C,cv2.THRESH_BINARY,9,2)
#video = cv2.adaptiveThreshold(video,255,cv2.ADAPTIVE_THRESH_MEAN_C,cv2.THRESH_BINARY,11,2)
#video = cv2.threshold(video, 127, 255, cv2.THRESH_BINARY)[1]

print "Performing Hough Circle Transform..."

circles = cv2.HoughCircles(video,cv2.cv.CV_HOUGH_GRADIENT,1,20,param1=50,param2=80,minRadius=0,maxRadius=0)
#circles = cv2.HoughCircles(video, cv2.cv.CV_HOUGH_GRADIENT, 1, 10, np.array([]), 100, 30, 1, 30) 


print "Rounding circles..."
circles = np.uint16(np.around(circles))

print "Drawing circles..."
for i in circles[0,:]:
    # draw the outer circle
    cv2.circle(video,(i[0],i[1]),i[2],(0,255,0),2)
    # draw the center of the circle
    cv2.circle(video,(i[0],i[1]),2,(0,0,255),3)


#niceVideo = frame_convert.video_cv(video)

#cv.ShowImage("Video", niceVideo)
cv2.imshow("Circles", video)
cv2.waitKey(0)
cv2.destroyAllWindows()
