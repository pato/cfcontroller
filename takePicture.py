#!/usr/bin/env python
import freenect
import cv
import cv2
import frame_convert
import time
import numpy as np
import random

#cv.NamedWindow('Depth')
#cv.NamedWindow('Video')
print('Press ESC in window to stop')



image = freenect.sync_get_video()[0]
num = random.randint(1, 20)
cv2.imwrite("image"+str(num)+".png", image)

print "Saved image "+str(num)
