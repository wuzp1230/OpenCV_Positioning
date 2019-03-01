#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>  
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <cstdio>
using namespace std;
using namespace cv;

double single_calib(InputArrayOfArrays _images, OutputArray _cameraMatrix, OutputArray _distCoeffs, vector<Mat> &rvecs, vector<Mat> &tvecs, bool show_corners = 1);
