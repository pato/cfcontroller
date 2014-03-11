/*
 *  balltracker.cpp
 *  Ball Tracking program
 *
 *  Created by Adam Faeth
 *     modified from the camshiftdemo.c file included with opencv
 *  February 24, 2007
 */
#ifdef _CH_
#pragma package <opencv>
#endif
 
#ifndef _EiC
#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <ctype.h>
#endif
 
#include<iostream>
 
#define BALL_MIN cvScalar(0, 80, 237)
#define BALL_MAX cvScalar(28, 163, 255)
#define RED_CUP_MIN cvScalar(168, 115, 76)
#define RED_CUP_MAX cvScalar(180, 255, 225)
#define GREEN_CUP_MIN cvScalar(62, 69, 0)
#define GREEN_CUP_MAX cvScalar(80, 255, 255) 
#define BLUE_CUP_MIN cvScalar(105, 86, 127) 
#define BLUE_CUP_MAX cvScalar(124, 255, 255)
 
//Ball:  Hmin: 0  Hmax: 28  Smin: 80  Smax: 163  Vmin: 237  VMax: 256
//Red cup:  Hmin: 168  Hmax: 180  Smin: 115  Smax: 256  Vmin: 76  VMax: 225
//Green cup:  Hmin: 62  Hmax: 80  Smin: 69  Smax: 256  Vmin: 0  VMax: 256
//Blue cup:  Hmin: 105  Hmax: 124  Smin: 86  Smax: 256  Vmin: 127  VMax: 256
 
IplImage *image = 0, *hsv = 0, *histimg = 0;
int backproject_mode = 0;
int select_object = 0;
int track_object = 0;
int show_hist = 1;
bool showSliders = 0;
CvPoint origin;
CvRect selection;
int selObjects = 0;
CvRect track_window;
CvBox2D track_box;
CvConnectedComp track_comp;
int hdims = 16;
float hranges_arr[] = {0,180};
float* hranges = hranges_arr;
int vmin = 10,  smin = 30, hmin = 30;
int vmax = 256, smax = 30, hmax = 30;
 
class ColorObject 
{
public:
	CvScalar cMin, cMax;
	CvHistogram * hist;
	IplImage * mask;
	const char * windowName;
	CvBox2D position;
	ColorObject(CvScalar min, CvScalar max, IplImage * hsvImage, const char window[]=NULL) 
	{ 
		cMin = min; cMax = max; windowName = window;
		mask = cvCreateImage( cvGetSize(image), 8, 1 );
		IplImage * hue = cvCreateImage( cvGetSize(image), 8, 1 );
		
		if(windowName != NULL)
			cvNamedWindow(windowName);
		
		/* create a mask of the values that lie within the object threshold */
		cvInRangeS( hsvImage, cMin, cMax, mask );
		cvSplit( hsvImage, hue, 0, 0, 0 );
 
		/* things that need to be done once to create the histogram that this object stores (I think) */
		hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );		
		float max_val = 0;
		cvCalcHist( &hue, hist, 0, mask );
		cvGetMinMaxHistValue( hist, 0, &max_val, 0, 0 );
		cvConvertScale( hist->bins, hist->bins, max_val ? 255. / max_val : 0., 0 );
		cvReleaseImage(&hue);
	}
	~ColorObject()
	{
		if(windowName != NULL)
			cvDestroyWindow(windowName);
		cvReleaseImage(&mask);
		cvReleaseHist(&hist);
	}
	
	
	void getPosition(IplImage * imHSV)
	{
		IplImage * backproject = cvCreateImage( cvGetSize(imHSV), 8, 1 );
		IplImage * hue = cvCreateImage( cvGetSize(imHSV), 8, 1 );
		
		CvConnectedComp trackComp;
		
		/* create a mask of the values that lie within the object threshold */
		cvInRangeS( imHSV, cMin, cMax, mask );
		cvSplit( imHSV, hue, 0, 0, 0 );
		
		/* find the backprojection for the object within the mask, then use camshift to find the object */
		cvCalcBackProject( &hue, backproject, hist );
		cvAnd( backproject, mask, backproject, 0 );
		cvCamShift( backproject, track_window,
				cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
				&trackComp, &position );
		
		/* display the back projection */
		if(windowName != NULL)
			cvShowImage(windowName, backproject);
		
		if( !image->origin )
                position.angle = -position.angle;
		//if(position.size.width > 0) 
			//cvEllipseBox( image, position, cMin, 3, CV_AA, 0 );
		
		cvReleaseImage(&backproject);
	}
	CvBox2D getPosition(void) { return position; }
};
 
class Ball : public ColorObject
{
public:
	ColorObject * obscuredBy;
	CvBox2D lastLocat;
	IplImage * ballImage;
	IplImage * ballMask;
	Ball(CvScalar min, CvScalar max, IplImage * hsvFirstFrame, const char window[]) : ColorObject(min, max, hsvFirstFrame, window) 
	{ 
		ballImage = NULL; 
		ballMask = NULL; 
	}
	~Ball() { if(ballImage != NULL) cvReleaseImage(&ballImage); if(ballMask != NULL) cvReleaseImage(&ballMask); }
	void getPosition(IplImage * hsvImage, ColorObject * redCup, ColorObject * greenCup, ColorObject * blueCup=NULL)
	{ 
		ColorObject::getPosition(hsvImage);
		
		/* if we can't find the ball, then it is under the ojbect that is closest to its last position */
		float trackBoxSize = position.size.width * position.size.height;
		
		/* visible */
		if( trackBoxSize > 10) {
			obscuredBy = NULL;
			lastLocat = position;
		}
		/* obscured for the first time */
		else if(obscuredBy == NULL)
		{
			obscuredBy = closestObject(redCup, greenCup, blueCup);
			position = obscuredBy->getPosition();
		}
		else {
			position = obscuredBy->getPosition();
//			std::cout << "Obscured by: " << obscuredBy->windowName << " " << position.center.x << ":" << position.center.y << std::endl;
		}
		if(position.size.width > 0)  {
			if( !image->origin )
                position.angle = -position.angle;
			cvEllipseBox( image, position, cMin, 3, CV_AA, 0 );
		}
	}
	ColorObject * closestObject(ColorObject * cand1, ColorObject * cand2, ColorObject * cand3=NULL) 
	{
		float dist1 = 100000.f, dist2 = 100000.f, dist3 = 100000.f;
		
		/* find the closest object to the lastLocat (last location) */
		dist1 = sqrt(pow(lastLocat.center.x - cand1->getPosition().center.x, 2) + pow(lastLocat.center.y - cand1->getPosition().center.y, 2));
		dist2 = sqrt(pow(lastLocat.center.x - cand2->getPosition().center.x, 2) + pow(lastLocat.center.y - cand2->getPosition().center.y, 2));
		if(cand3 != NULL)
			dist3 = sqrt(pow(lastLocat.center.x - cand3->getPosition().center.x, 2) + pow(lastLocat.center.y - cand3->getPosition().center.y, 2));
 
		/* return the pointer to the closest object */
		if(dist1 < dist2 && dist1 < dist3) {
			std::cout << "Red closest\n";
			return cand1;
		}
		else if(dist2 < dist3) {
			std::cout << "Green closest\n";
			return cand2;
		}
		else {
			std::cout << "Blue closest\n";
			return cand3;
		}
	}
};
 
 
CvScalar hsv2rgb( float hue )
{
    int rgb[3], p, sector;
    static const int sector_data[][3]=
        {{0,2,1}, {1,2,0}, {1,0,2}, {2,0,1}, {2,1,0}, {0,1,2}};
    hue *= 0.033333333333333333333333333333333f;
    sector = cvFloor(hue);
    p = cvRound(255*(hue - sector));
    p ^= sector & 1 ? 255 : 0;
 
    rgb[sector_data[sector][0]] = 255;
    rgb[sector_data[sector][1]] = 0;
    rgb[sector_data[sector][2]] = p;
 
    return cvScalar(rgb[2], rgb[1], rgb[0],0);
}
 
int main( int argc, char** argv )
{
    CvCapture* capture = 0;
	bool firstFrame = true;
	ColorObject * redCup = NULL;
	ColorObject * blueCup = NULL;
	ColorObject * greenCup = NULL;
	Ball * orangeBall = NULL;
 
		
	capture = cvCaptureFromFile("/Users/adamfaeth/Documents/coursework/hci 575/homework 3/2 cups/2-3.avi");
	CvVideoWriter * outVideo = cvCreateVideoWriter("/Users/adamfaeth/Documents/coursework/hci 575/homework 3/2 cups/2-3-tracked.avi", 
			CV_FOURCC('D', 'I', 'V', 'X'), 29.97, cvSize(320, 240));
 
    if( !capture )
    {
        fprintf(stderr,"Could not initialize capturing...\n");
        return -1;
    }
 
    cvNamedWindow( "Histogram", 1 );
    cvNamedWindow( "BallColorTracker", 1 );
	cvNamedWindow( "Mask", 1);
    //cvSetMouseCallback( "BallColorTracker", on_mouse, 0 );
	
    cvCreateTrackbar( "Hmin", "BallColorTracker", &hmin, 256, 0 );
    cvCreateTrackbar( "Hmax", "BallColorTracker", &hmax, 180, 0 );
    cvCreateTrackbar( "Vmin", "BallColorTracker", &vmin, 256, 0 );
    cvCreateTrackbar( "Vmax", "BallColorTracker", &vmax, 256, 0 );
    cvCreateTrackbar( "Smin", "BallColorTracker", &smin, 256, 0 );
    cvCreateTrackbar( "Smax", "BallColorTracker", &smax, 256, 0 );
	
	
    for(;;)
    {
        IplImage* frame = 0;
        int c;
 
        frame = cvQueryFrame( capture );
        if( !frame )
            break;
 
        if( !image )
        {
            /* allocate all the buffers */
            image = cvCreateImage( cvGetSize(frame), 8, 3 );
            image->origin = frame->origin;
            hsv = cvCreateImage( cvGetSize(frame), 8, 3 );
            histimg = cvCreateImage( cvSize(320,200), 8, 3 );
            cvZero( histimg );
			track_window.x = 0;
			track_window.y = 0;
			track_window.width = frame->width;
			track_window.height = frame->height;
		}
 
		cvCopy( frame, image, 0 );
		cvCvtColor( image, hsv, CV_BGR2HSV );
		
		if(firstFrame) {
			/* initialize the tracked objects */
			orangeBall = new Ball(BALL_MIN, BALL_MAX, hsv, "Ball");
			redCup = new ColorObject(RED_CUP_MIN, RED_CUP_MAX, hsv, "Red Cup");
			blueCup = new ColorObject(BLUE_CUP_MIN, BLUE_CUP_MAX, hsv, "Blue Cup");
			greenCup = new ColorObject(GREEN_CUP_MIN, GREEN_CUP_MAX, hsv, "Green Cup");
			firstFrame = false;
		}
		
		if(showSliders) {
			int lvMin = 0, lvMax = 0, lhMin = 0, lhMax = 0, lsMin = 0, lsMax = 0;
			int _vmin, _vmax, _hmin, _hmax, _smin, _smax;
			IplImage * mask = cvCreateImage( cvGetSize(frame), 8, 1);
			
			while( (char)cvWaitKey(10) != ' ') {
				_vmin = vmin, _vmax = vmax, _hmin = hmin, _hmax = hmax, _smin = smin, _smax = smax;
				
				/* create a mask of the values that lie within the threshold sliders */
				cvInRangeS( hsv, cvScalar(_hmin,_smin,MIN(_vmin,_vmax),0),
							cvScalar(_hmax,_smax,MAX(_vmin,_vmax),0), mask );
 
				if(_vmin != lvMin || _vmax != lvMin || _smin != lsMin || _smin != lsMin || smin != lsMin) {
					std::cout << "Hmin: " << _hmin << "  Hmax: " << _hmax << "  Smin: " << _smin << "  Smax: " << _smax << "  Vmin: " << _vmin << "  VMax: " << _vmax << std::endl;
					lvMin = _vmin, lvMax = _vmax, lhMin = _hmin, lhMax = _hmax, lsMin = _smin, lsMax = _smax;
				}
				
				cvShowImage("BallColorTracker", image);
				cvShowImage("Mask", mask);
			}
			showSliders=false;
			
			cvReleaseImage(&mask);
		}
		
		/* Find the objects in the frame */
		redCup->getPosition(hsv);
		blueCup->getPosition(hsv);
		greenCup->getPosition(hsv);		
		orangeBall->getPosition(hsv, redCup, greenCup, blueCup);
		
		
        cvWriteFrame(outVideo, image);
        cvShowImage( "BallColorTracker", image );
        cvShowImage( "Histogram", histimg );
		
		c = cvWaitKey(10);
			
        if( (char) c == 27 )
            break;
        switch( (char) c )
        {
        case 'b':
            backproject_mode ^= 1;
            break;
        case 'c':
            track_object = 0;
            cvZero( histimg );
            break;
        case 'h':
            show_hist ^= 1;
            if( !show_hist )
                cvDestroyWindow( "Histogram" );
            else
                cvNamedWindow( "Histogram", 1 );
            break;
		case 's':
			showSliders ^= 1;
			break;
        default:
            ;
        }
    }
 
    cvReleaseCapture( &capture );
    cvDestroyWindow("BallColorTracker");
	cvDestroyWindow( "Histogram" );
	cvDestroyWindow( "Mask" );
	cvReleaseVideoWriter( &outVideo);
	delete redCup;
	delete greenCup;
	delete blueCup;
	delete orangeBall;
    return 0;
}
