#include "ballTracking.h"


int main(int argc, char* argv[]){
// Default capture size - 640x480
CvSize size = cvSize(640,480);

// Open capture device. 0 is /dev/video0, 1 is /dev/video1, etc.
CvCapture* capture = cvCaptureFromCAM( 0 );
if( !capture ){
    fprintf( stderr, "ERROR: capture is NULL \n" );
    getchar();
    return -1;
}

// Create a window in which the captured images will be presented
cvNamedWindow( "Camera", CV_WINDOW_AUTOSIZE );
cvNamedWindow( "HSV", CV_WINDOW_AUTOSIZE );
cvNamedWindow( "EdgeDetection", CV_WINDOW_AUTOSIZE );

// Detect a red ball
CvScalar hsv_min = cvScalar(150, 84, 130, 0);
CvScalar hsv_max = cvScalar(358, 256, 255, 0);

// Detect yellow
//hsv_min = cvScalar(20, 41, 133);
//hsv_max = cvScalar(40, 150, 255);

// Detect blue
//hsv_min = cvScalar(100, 150, 0);
//hsv_max = cvScalar(150, 255, 255);

// Detect Pink
//hsv_min = cvScalar(100, 90, 100);
//hsv_max = cvScalar(172, 255, 255);

IplImage * hsv_frame = cvCreateImage(size, IPL_DEPTH_8U, 3);
IplImage* thresholded = cvCreateImage(size, IPL_DEPTH_8U, 1);

vector<double> previousX;
vector<double> previousY;
vector<CvPoint> previousC;
std::vector<cv::Point2f> previousPoints;
cv::Vec4f line;

//Used for calculating FPS
time_t start, end;
double fps;
double sec;
int counter = 0;
time(&start);


CvFont font;
cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5);

while( 1 ){
    // Get one frame
    IplImage* frame = cvQueryFrame( capture );
    if( !frame ){
        fprintf( stderr, "ERROR: frame is null...\n" );
        getchar();
        break;
    }

    // Covert color space to HSV as it is much easier to filter colors in the HSV color-space.
    cvCvtColor(frame, hsv_frame, CV_BGR2HSV);
    // Filter out colors which are out of range.
    cvInRangeS(hsv_frame, hsv_min, hsv_max, thresholded);

    // Draw the previous positions
    bool drawPoints = true;
    bool connectPoints = false;

    if (drawPoints){
        for (int i=0; i <previousC.size(); i++){
            cvCircle( frame, previousC.at(i), 2, CV_RGB(255,127,0), -1, 8, 0);
            if (connectPoints && i>0){
                cvLine( frame, previousC.at(i-1), previousC.at(i), cvScalar(255,0,0), 2, 8);
            }
        }
    }
   

    if (previousC.size() > 1){
        //cv::fitLine(previousPoints, line, CV_DIST_L2, 0, 0.01, 0.01); 
        //cv::line(frame, cv::Point(line[2],line[3]), cv::Point(line[2]+line[0]*5,line[3]+line[1]*5));
        //cvLine(frame, cv::Point(line[2],line[3]), cv::Point(line[2]+line[0]*2000,line[3]+line[1]*2000), cvScalar(255, 0,0), 2, 8);
        
        vector<double> coefficients = polyReg(previousX, previousY, 2);
        //vector<CvPoint> points = getPoints(coefficients, 600, 1); 
        vector<CvPoint> points = getSpecificPoints(previousC, coefficients); 

        for (int i=0; i < points.size() - 1; i++){
            cvLine(frame, points.at(i), points.at(i+1), cvScalar(255,0,0), 2, 8);
        }

        if (false){
            for (int i=0;i<coefficients.size();i++){
                cout<< coefficients[i] << "x^" << (i) << " + ";
            }
            cout<< "0" << endl;
        }
    }



    // Memory for hough circles
    CvMemStorage* storage = cvCreateMemStorage(0);
    // hough detector works better with some smoothing of the image
    cvSmooth( thresholded, thresholded, CV_GAUSSIAN, 9, 9 );
    CvSeq* circles = cvHoughCircles(thresholded, storage, CV_HOUGH_GRADIENT, 2,
    thresholded->height/4, 100, 50, 10, 400);

    for (int i = 0; i < circles->total; i++){
        float* p = (float*)cvGetSeqElem( circles, i );
        //printf("Ball! x=%f y=%f r=%f\n\r",p[0],p[1],p[2] );
        cvCircle( frame, cvPoint(cvRound(p[0]),cvRound(p[1])),3, CV_RGB(0,255,0), -1, 8, 0 );
        cvCircle( frame, cvPoint(cvRound(p[0]),cvRound(p[1])),cvRound(p[2]), CV_RGB(255,0,0), 3, 8, 0 );
        //cvLine( frame, previousC.at(circleIndex), cvPoint(p[0],p[1]), cvScalar(255,0,0), 2, 8);
        
        //previousPoints.push_back(cv::Point2f(p[0], p[1]));
        previousC.push_back(cvPoint(p[0], p[1]));        
        previousX.push_back(p[0]);
        previousY.push_back(p[1]);
    }


    // Calculate FPS
    time(&end);
    counter++;
    sec = difftime(end, start);
    fps = counter / sec;
    
    char buffer[10];
    sprintf(buffer, "FPS: %.2f", fps);

    cvPutText(frame, buffer, cvPoint(10, 30), &font , cvScalar(255,255,255));

    cvShowImage( "Camera", frame ); // Original stream with detected ball overlay
    cvShowImage( "HSV", hsv_frame); // Original stream in the HSV color space
    cvShowImage( "After Color Filtering", thresholded ); // The stream after color filtering

    cvReleaseMemStorage(&storage);

    // Do not release the frame!
    if( (cvWaitKey(10) & 255) == 27 ) break;

}

// Release the capture device housekeeping
cvReleaseCapture( &capture );
cvDestroyWindow( "mywindow" );
return 0;
}
