#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>  
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <cstdio>
using namespace cv;

double stereo_calib(const vector<Mat> images[2], vector<Mat> &_cameraMatrix, vector<Mat> &_distCoeffs, Mat &_R, Mat &_T, Mat &_R1, Mat &_R2, Mat &_P1, Mat &_P2, Mat &_Q, vector<Rect> &ROI, bool showProcess);
void saveStereoResults(const string fname, const double err, const vector<Mat> cameraMatrix, const vector<Mat> distCoeffs, const Mat R, const Mat T, const Mat R1, const Mat R2, const Mat P1, const Mat P2, const Mat Q, const vector<Rect> ROI);
void loadStereoResults(const string fname, const int cnum, vector<Mat> &cameraMatrix, vector<Mat> &distCoeffs, Mat &R, Mat &T, Mat &R1, Mat &R2, Mat &P1, Mat &P2, Mat &Q, vector<Rect> &ROI);
void lengthErrorMeasure(vector<Mat> images, const Mat Q,int num);
extern void bug_info(string s);