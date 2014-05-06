#include "ros/ros.h"
#include "std_msgs/String.h"
#include "std_msgs/Empty.h"
#include <geometry_msgs/Twist.h>

#include <opencv/cvaux.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>
#include "opencv2/opencv.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>

//ball - red (hsv)
#define BALL_MIN (Scalar(150, 84, 130))
#define BALL_MAX (Scalar(358, 256, 255))

//quad - green (ycrcb)
#define QUAD_MIN (Scalar(0, 0, 0))
#define QUAD_MAX (Scalar(255, 120, 130))

using namespace cv;

int min(int a, int b) {
  return (a < b) ? a : b;
}

int max(int a, int b) {
  return (a > b) ? a : b;
}

cv::Mat getImage(CvCapture* capture) {
  
  IplImage* frame = cvQueryFrame(capture);
  if (!frame) {
    std::cerr << "ERROR: frame is null\n";
    return Mat(0, 0, CV_8UC4);
  }
  Mat result(frame, true);
  //cvtColor(result, result, CV_RGB2BGR);
  return result;
  
}

std::vector<Vec3f> findBall(Mat frame) {
  
  Mat hsvFrame(frame.size().height, frame.size().width, CV_8UC4);
  Mat thresholded(frame.size().height, frame.size().width, CV_8UC1);
  
  GaussianBlur(frame, frame, Size(9, 9), 1);
  cvtColor(frame, hsvFrame, CV_RGB2HSV); //might have to be bgr
  inRange(hsvFrame, BALL_MIN, BALL_MAX, thresholded);
  std::vector<Vec3f> circles;
  HoughCircles(thresholded, circles, CV_HOUGH_GRADIENT, 2, thresholded.size().height/4, 100, 50, 10, 400);

  return circles;
  
}

std::vector<Vec3f> findQuad(Mat frame) {
  //return findQuadAlt(frame);
  
  //use ycrcb since I already have good ranges for green
  Mat ycrcbFrame(frame.size().height, frame.size().width, CV_8UC4);
  Mat thresholded(frame.size().height, frame.size().width, CV_8UC1);
  
  GaussianBlur(frame, frame, Size(9, 9), 1);
  cvtColor(frame, ycrcbFrame, CV_RGB2YCrCb); //might have to be bgr
  inRange(ycrcbFrame, QUAD_MIN, QUAD_MAX, thresholded);
  std::vector<Vec3f> circles;
  HoughCircles(thresholded, circles, CV_HOUGH_GRADIENT, 2, thresholded.size().height/4, 100, 50, 10, 400);
  
  return circles;
  
}

std::vector<Vec3f> findQuadAlt(Mat frame) {
  
  int pY = 100;
  int pCR = 100;
  int pCB = 100;
  
  Mat ycrcbFrame(frame.size().height, frame.size().width, CV_8UC4);
  Mat gray(frame.size().height, frame.size().width, CV_8UC1);
  
  GaussianBlur(frame, frame, Size(9, 9), 1);
  cvtColor(frame, ycrcbFrame, CV_RGB2YCrCb); //might have to be bgr

  int max_int = (int) (((unsigned)(~0)) >> 1);
  int min = max_int;
  int min_x = max_int;
  int min_y = max_int;
  
  for (int i = 0; i < ycrcbFrame.size().height; i++) {
    for (int j = 0; j < ycrcbFrame.size().width; j++) {
      Vec3b pixel = ycrcbFrame.at<Vec3b>(i, j);
      uchar y = pixel.val[0];
      uchar cr = pixel.val[1];
      uchar cb = pixel.val[2];
      
      int distance = (int)sqrt(0.5 * (pow((y-pY), 2) + pow((cr-pCR), 2) + pow((cb-pCB), 2))/3);

      if (distance < min) {
        min = distance;
        min_x = j;
        min_y = i;
      }
      
    }
  }
  std::vector<Vec3f> result;
  if (min_x != max_int && min_y != max_int) {
    result.push_back(Vec3f((float) min_x, (float) min_y, 0));
  }
  
  return result;
  
}

int main(int argc, char **argv) {
  //ros initialization
  ros::init(argc, argv, "pilot");
  ros::NodeHandle n;
  
  ros::Rate rate(30); //max 30fps from camera
  
  ros::Publisher pub_move = n.advertise<geometry_msgs::Twist>("cmd_vel", 30);
  ros::Publisher pub_takeoff = n.advertise<std_msgs::Empty>("/ardrone/takeoff", 30, true);
  ros::Publisher pub_land = n.advertise<std_msgs::Empty>("/ardrone/land", 30, true);
  
  std_msgs::Empty empty;

  //OpenCv initialization
  CvCapture* capture = cvCaptureFromCAM(0);
  if (!capture) {
    std::cerr << "ERROR: Capture was null\n";
    return -1;
  }
  
  namedWindow("Camera", CV_WINDOW_AUTOSIZE);
  moveWindow("Camera", 0, 0);
  
  //takeoff
  pub_takeoff.publish(empty); //??? - dunno if it'll work
  
  //keep on moving up until in view
  Mat frame = getImage(capture);
  imshow("Camera", frame);
  
  while(findQuad(frame).size() == 0) {
    geometry_msgs::Twist vel_msg;
    vel_msg.angular.z = 0.5;
    
    pub_move.publish(vel_msg);
    
    frame = getImage(capture);
    imshow("Camera", frame);

    rate.sleep();
  }
  
  //move
  std::vector<Vec3f> ball_previous = findBall(frame);
  std::vector<Vec3f> ball_current = ball_previous;
  std::vector<Vec3f> quad = findQuad(frame);
  
  while(n.ok()) {
    //find the target + quad
    frame = getImage(capture);
    ball_current = findBall(frame);
    quad = findQuad(frame);
    
    //backup in case the target can't be found
    if (ball_current.size() == 0) {
      ball_current = ball_previous;
    }
    
    //set velocity
    int ball_x = cvRound(ball_current[0][0]);
    int ball_y = cvRound(ball_current[0][1]);
    int radius = cvRound(ball_current[0][2]);
    
    int quad_x, quad_y;
    
    if (quad.size() == 0) {
      quad_x = ball_x;
      quad_y = ball_y;
    }
    else {
      quad_x = cvRound(quad[0][0]);
      quad_y = cvRound(quad[0][1]);
    }
    
    
    signed char isLeft = (ball_x - quad_x > 0) ? 1 : -1; //1 if quad is left of ball
    signed char isDown = (quad_y - ball_y > 0) ? 1 : -1; //(0, 0) is top left corner
    
    
    int dist_x = max(abs(ball_x - quad_x), radius);
    int dist_y = max(abs(ball_y - quad_y), radius);
    
    geometry_msgs::Twist vel_msg;
    
    //0.1 for speed constant, 1/400 for 400 pixel distance -> 0.1 speed
    //linear.y and linear.z of ARDrone because we assume x forward, y left, z up
    //linear.y > 0 => move left ; linear.z > 0 => move up
    vel_msg.linear.y = min(((0.1 * 1/400) * (dist_x - radius)), 1) * isLeft * (-1);
    vel_msg.linear.z = min(((0.1 * 1/400) * (dist_y - radius)), 1) * isDown;

    pub_move.publish(vel_msg);
      
    //draw image + circles
    Point ball_center(ball_x, ball_y);
    circle(frame, ball_center, 3, Scalar(0, 255, 0), -1, 8, 0);
    circle(frame, ball_center, radius, Scalar(0, 0, 255), 3, 8, 0);

    Point quad_center(quad_x, quad_y);
    circle(frame, quad_center, 3, CV_RGB(255, 127, 0), -1, 8, 0);

    if ((waitKey(10) & 255) == 27) {
      pub_land.publish(empty); //take note if takeoff msg changes
      ros::shutdown();
      break;
    }
    
    imshow("Camera", frame);
    
    
    rate.sleep();
  }
  
  cvReleaseCapture(&capture);
  destroyAllWindows();

  
  return 0;
}
