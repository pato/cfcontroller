#!/usr/bin/python
from opencv.cv import *
from opencv.highgui import *
from threading import Thread
#import serial

class RobotVision:
	cvSize size
	cvImage hsv_frame, thresholded, thresholded2
	cvScalar hsv_min, hsv_max, hsv_min2, hsv_max2
	cvCapture capture;

	def InitBallTracking():
		globals size, hsv_frame, thresholded, thresholded2, hsv_min, hsv_max, hsv_min2, hsv_max2, capture
		print "Initializing ball Tracking"
		size = cvSize(640, 480)
		hsv_frame = cvCreateImage(size, IPL_DEPTH_8U, 3)
		thresholded = cvCreateImage(size, IPL_DEPTH_8U, 1)
		thresholded2 = cvCreateImage(size, IPL_DEPTH_8U, 1)

		hsv_min = cvScalar(0, 50, 170, 0)
		hsv_max = cvScalar(10, 180, 256, 0)
		hsv_min2 = cvScalar(170, 50, 170, 0)
		hsv_max2 = cvScalar(256, 180, 256, 0)

		storage = cvCreateMemStorage(0)

		# start capturing form webcam
		capture = cvCreateCameraCapture(-1)

		if not capture:
		print "Could not open webcam"
		sys.exit(1)

		#CV windows
		cvNamedWindow( "Camera", CV_WINDOW_AUTOSIZE );

	def TrackBall(i):
		t = Thread(target=TrackBallThread, args=(i,))
		t.start()

	def TrackBallThread(num_of_balls):
		globals size, hsv_frame, thresholded, thresholded2, hsv_min, hsv_max, hsv_min2, hsv_max2, capture
		while 1:
			# get a frame from the webcam
			frame = cvQueryFrame(capture)

			if frame is not None:

				# convert to HSV for color matching
				# as hue wraps around, we need to match it in 2 parts and OR together
				cvCvtColor(frame, hsv_frame, CV_BGR2HSV)
				cvInRangeS(hsv_frame, hsv_min, hsv_max, thresholded)
				cvInRangeS(hsv_frame, hsv_min2, hsv_max2, thresholded2)
				cvOr(thresholded, thresholded2, thresholded)

				# pre-smoothing improves Hough detector
				cvSmooth(thresholded, thresholded, CV_GAUSSIAN, 9, 9)
				circles = cvHoughCircles(thresholded, storage, CV_HOUGH_GRADIENT, 2, thresholded.height/4, 100, 40, 20, 200)

				# find largest circle
				maxRadius = 0
				x = 0
				y = 0
				found = False
				for i in range(circles.total):
					circle = circles[i]
					if circle[2] > maxRadius:
					found = True
						maxRadius = circle[2]
						x = circle[0]
						y = circle[1]

				cvShowImage( "Camera", frame );

				if found:
					print "ball detected at position:",x, ",", y, " with radius:", maxRadius

					if x > 420:
						# need to pan right
						servoPos += 5
						servoPos = min(140, servoPos)
						servo(2, servoPos)
					elif x < 220:
						servoPos -= 5
						servoPos = max(40, servoPos)
						servo(2, servoPos)
					print "servo position:", servoPos
			else:
				print "no ball"
