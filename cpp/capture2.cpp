#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp> 
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char* argv[]){
    VideoCapture cap(0); // open the video camera no. 0

    if (!cap.isOpened()){  // if not success, exit program{
        cout << "Cannot open the video cam" << endl;
        return -1;
    }

   double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
   double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
   double fps = cap.get(CV_CAP_PROP_FPS);

    cout << "Frame size : " << dWidth << " x " << dHeight << endl;

    cout << "Framerate : " << fps << endl;

    namedWindow("MyVideo",CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"

    while (1){
        Mat frame;

        bool bSuccess = cap.read(frame); // read a new frame from video

         if (!bSuccess){
             cout << "Cannot read a frame from video stream" << endl;
             break;
        }

        imshow("MyVideo", frame); //show the frame in "MyVideo" window

        if (waitKey(30) == 27){
            cout << "esc key is pressed by user" << endl;
            break; 
       }
    }
    return 0;

}
