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

  
  //"perfect" ball color
  /* pink = 210, 20, 50 ; maxDist = 65 */
  /* green = 20, 160, 80; maxDist = 45 */
  uchar pY = 100;
  uchar pCR = 100;
  uchar pCB = 120;

  //constants for image
  int maxDist = 50;
  int scale = 2;
  bool threshold = false;

  Mat ycrcb_frame(cols/scale, rows/scale, CV_8UC4);
  Mat gray(cols/scale, rows/scale, CV_8UC1);
  Mat frame(cols/scale, rows/scale, CV_8UC4);
  
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

  // Create a window in which the captured images will be presented
  namedWindow( "Camera", CV_WINDOW_AUTOSIZE );
  namedWindow( "YCrCb", CV_WINDOW_AUTOSIZE );
  namedWindow("After Color Filtering", CV_WINDOW_AUTOSIZE);
  moveWindow("Camera", 0, 0);
  moveWindow("YCrCb", rows/scale, 0);
  moveWindow("After Color Filtering", 2 * rows/scale, 0);
  
  while( 1 ){
    // Get one frame
    IplImage* iframe = cvQueryFrame(capture);
    if( !iframe ){
      fprintf( stderr, "ERROR: frame is null...\n" );
      getchar();
      break;
    }

    Mat dummyFrame(iframe);
    resize(dummyFrame, frame, Size(rows/scale, cols/scale));
    
    // hough detector works better with some smoothing of the image
    GaussianBlur(frame, frame, Size(9, 9), 1);
    
    // Covert color space to YCrCb
    cvtColor(frame, ycrcb_frame, CV_RGB2YCrCb);
    
    // Convert to custom grayscale
    for (int i = 0; i < ycrcb_frame.size().height; i++) {
      for (int j = 0; j < ycrcb_frame.size().width; j++) {
        Vec3b pixel = ycrcb_frame.at<Vec3b>(i, j);
        uchar y = pixel.val[0];
        uchar cr = pixel.val[1];
        uchar cb = pixel.val[2];
        
        int distance = (int)sqrt((pow((y-pY), 2) + pow((cr-pCR), 2) + pow((cb-pCB), 2))/3);
        if (distance > 255) {
          printf("distance was greater than 255...");
          gray.at<uchar>(i, j) = 255;
        }
        else {
          if (threshold && (distance > maxDist)) {
            distance = 255;
          }
          gray.at<uchar>(i, j) = distance;
        }
      }
    }
    
    
    // Draw the previous positions
    /*
    for (int i=0; i <previousC.size(); i++){
      circle( frame, previousC.at(i), 2, CV_RGB(255,127,0), -1, 8, 0);
    }
    */

    /*
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
    */


    // Calculate FPS
    time(&end);
    counter++;
    sec = difftime(end, start);
    fps = counter / sec;
    
    char buffer[50];
    sprintf(buffer, "FPS: %.2f", fps);

    putText(frame, buffer, cvPoint(10, 30), FONT_HERSHEY_SIMPLEX, 1, cvScalar(255,255,255));

    imshow( "Camera", frame ); // Original stream with detected ball overlay
    imshow( "YCrCb", ycrcb_frame); // Original stream in the YCrCb color space
    imshow( "After Color Filtering", gray ); // The stream after color filtering


    // Do not release the frame!
    if( (cvWaitKey(10) & 255) == 27 ) break;

  }

  // Release the capture device housekeeping
  cvReleaseCapture( &capture );
  cvDestroyWindow( "mywindow" );
  return 0;
}
