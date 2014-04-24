#include <opencv/cvaux.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>
#include "opencv2/opencv.hpp"
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

vector<double> polyReg(vector<double> &xcoords, vector<double> &ycoords, int m);
vector<CvPoint> getPoints(vector<double> coefficients, double count, double step);
vector<CvPoint> getSpecificPoints(vector<CvPoint> points, vector<double> coefficients);
