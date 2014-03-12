#include <opencv/cvaux.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>
#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>

using namespace std;
using namespace cv;

int main(int argc, char* argv[]){
  // Default capture size - 640x480
  int rows = 640;
  int cols = 480;

  // Open capture device. 0 is /dev/video0, 1 is /dev/video1, etc.
  CvCapture* capture = cvCaptureFromCAM( 0 );
  if( !capture ){
    fprintf( stderr, "ERROR: capture is NULL \n" );
    getchar();
    return -1;
  }

  // Create a window in which the captured images will be presented
  namedWindow( "Camera", CV_WINDOW_AUTOSIZE );
  //namedWindow( "YCrCb", CV_WINDOW_AUTOSIZE );
  namedWindow("After Color Filtering", CV_WINDOW_AUTOSIZE);
  moveWindow("Camera", 0, 0);
  moveWindow("After Color Filtering", 640, 0);
  
  //"perfect" ball color
  uchar pRED = 210;
  uchar pGREEN = 20;
  uchar pBLUE = 50;
  
  int maxDist = 65;

  Mat ycrcb_frame(rows, cols, CV_8UC4);
  Mat gray(cols, rows, CV_8UC1);

  vector<float> previousX;
  vector<float> previousY;
  vector<CvPoint> previousC;

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
    IplImage* iframe = cvQueryFrame(capture);
    if( !iframe ){
      fprintf( stderr, "ERROR: frame is null...\n" );
      getchar();
      break;
    }
    
    Mat frame(iframe);


    // Covert color space to YCrCb....later
    //cvCvtColor(frame, hsv_frame, CV_BGR2HSV);
    
    // hough detector works better with some smoothing of the image
    GaussianBlur(frame, frame, Size(9, 9), 1);
    
    // Convert to custom grayscale
    for (int i = 0; i < frame.size().height; i++) {
      for (int j = 0; j < frame.size().width; j++) {
        Vec3b pixel = frame.at<Vec3b>(i, j);
        uchar blue = pixel.val[0];
        uchar green = pixel.val[1];
        uchar red = pixel.val[2];
        
        int distance = (int)sqrt((pow((blue-pBLUE), 2) + pow((green-pGREEN), 2) + pow((red-pRED), 2))/3);
        if (distance > 255) {
          printf("distance was greater than 255...");
          gray.at<uchar>(i, j) = 255;
        }
        else {
          if (distance > maxDist) {
            distance = 255;
          }
          gray.at<uchar>(i, j) = distance;
        }
      }
    }
    
    
    // Draw the previous positions
    for (int i=0; i <previousC.size(); i++){
      circle( frame, previousC.at(i), 2, CV_RGB(255,127,0), -1, 8, 0);
    }
    
    // Hough Circles
    vector<Vec3f> circles;
    HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 4, 200, 20, 100, 0, 200);

    for (int i = 0; i < circles.size(); i++){
      Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
      int radius = cvRound(circles[i][2]);
      // draw the circle center
      circle( frame, center, 3, Scalar(0,255,0), -1, 8, 0 );
      // draw the circle outline
      circle( frame, center, radius, Scalar(0,0,255), 3, 8, 0 );
      previousC.push_back(cvPoint(cvRound(circles[i][0]), cvRound(circles[i][1])));
    }


    // Calculate FPS
    time(&end);
    counter++;
    sec = difftime(end, start);
    fps = counter / sec;
    
    char buffer[50];
    sprintf(buffer, "FPS: %.2f", fps);

    putText(frame, buffer, cvPoint(10, 30), FONT_HERSHEY_SIMPLEX, 1, cvScalar(255,255,255));

    imshow( "Camera", frame ); // Original stream with detected ball overlay
    //cvShowImage( "HSV", hsv_frame); // Original stream in the YCrCb color space
    imshow( "After Color Filtering", gray ); // The stream after color filtering


    // Do not release the frame!
    if( (cvWaitKey(10) & 255) == 27 ) break;

  }

  // Release the capture device housekeeping
  cvReleaseCapture( &capture );
  cvDestroyWindow( "mywindow" );
  return 0;
}
