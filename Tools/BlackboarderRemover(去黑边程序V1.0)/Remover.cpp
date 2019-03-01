#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>  
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <cstdio>
using namespace cv;
using namespace std;
void logInfo(String s)
{
	cout << s << endl;
	return;
}
String showRGB(Mat img1,Point point1) //Show RGB o a point
{
	Vec3b RGB_read = img1.at<Vec3b>(point1);
	cout << RGB_read << endl;
	int B = RGB_read.val[0]; //B蓝
	int G = RGB_read.val[1]; //G绿
	int R = RGB_read.val[2]; //R红
	stringstream ss;

	ss << "(";
	ss << point1.x;
	ss << ",";
	ss << point1.y;
	ss << "|||";
	ss << R;
	ss << ",";
	ss << G;
	ss << ",";
	ss << B;
	ss << ")";
	return ss.str();
}
bool isBlackpoint(Mat img1, Point point1, int threshold) //Judge whether point P is a black point
{
	Vec3b RGB_read = img1.at<Vec3b>(point1);
	int B = RGB_read.val[0]; //B蓝
	int G = RGB_read.val[1]; //G绿
	int R = RGB_read.val[2]; //R红
	if (B <= threshold && G <= threshold && R<= threshold)
		return 1;
	else
		return 0;
}
void initboundaryPoint(Mat src, Vector<Point> &boundary, int threshold)
{//从四角快速搜索初始图片边缘
	Mat dst;
	Point left_up, left_down, right_up, rightdown; // 四角
	Vector<Point> boundaries;
	Point pointer; // 探针
	int cols = src.cols - 1; //width
	int rows = src.rows - 1; //height
	//cout << "cols:" << cols << "  rows:" << rows <<endl;
	int min_col_rows = min(cols, rows);
	int max_cols_rows = max(cols, rows);
	logInfo("initial Boundaries:");
	bool found_left_up = false;					//左上，从左下到右上
	for (int i = 0; i < min_col_rows && !found_left_up; i++)
	{
		for (int j = 0; j <= i && !found_left_up; j++)
		{
			pointer.x = j; pointer.y = i - j;
			found_left_up = !isBlackpoint(src, pointer,threshold);
		}
	}
	boundary.push_back(pointer);
	logInfo("found_left_up:");
	bool found_right_up = false;			//右上,从左上到右下
	for (int i = 0; i < min_col_rows && !found_right_up; i++)
	{
		for (int j = 0; j <= i && !found_right_up; j++)
		{
			pointer.x = cols  - i + j; pointer.y = j;
			found_right_up = !isBlackpoint(src, pointer, threshold);
		}
	}
	boundary.push_back(pointer);
	logInfo("found_right_up:");
	bool found_left_down = false;				//左下，从左上到右下
	for (int i = 0; i < min_col_rows && !found_left_down; i++)
	{
		for (int j = 0; j <= i && !found_left_down; j++)
		{
			pointer.x = j; pointer.y = rows - i + j;
			found_left_down = !isBlackpoint(src, pointer, threshold);
		}
	}
	boundary.push_back(pointer);
	logInfo("found_left_down:");
	bool found_right_down = false;				//右下，从左下到右上
	for (int i = 0; i < min_col_rows && !found_right_down; i++)
	{
		for (int j = 0; j <= i && !found_right_down; j++)
		{
			pointer.x = cols  - i + j; pointer.y =  rows - j;
			found_right_down = !isBlackpoint(src, pointer, threshold);
		}
	}
	boundary.push_back(pointer);
	logInfo("found_right_down:");
	//cout << "InitBoundary : " << endl;
	return;
}
Mat boundaryRemover(Mat &src, int threshold)
{
	Point left_up, left_down, right_up, rightdown; // 四角
	Vector<Point> boundary; //左上，右上，左下，右下
	double ROI = 0.2;
	Point pointer, next_1_pointer, next_2_pointer, next_3_pointer; // 四个探针，相当于去噪
	int cols = src.cols; //width宽度
	int rows = src.rows; //height高度
	initboundaryPoint(src, boundary,threshold);
	//开始取共同宽度
	int upBoundary = max(boundary[0].y, boundary[1].y);
	int dowmBoundary = min(boundary[2].y, boundary[3].y);
	int leftBoundary = max(boundary[0].x, boundary[2].x);
	int rightBoundary = min(boundary[1].x, boundary[3].x);
	logInfo("Boundaries:");
	//cout << upBoundary << endl << dowmBoundary << endl << leftBoundary << endl << rightBoundary << endl;
	logInfo("boundaries calculated ");
	int flag = 0;
	Rect finalRect;

	finalRect.y = upBoundary;
	finalRect.height = dowmBoundary - upBoundary;
	finalRect.x = leftBoundary;
	finalRect.width = rightBoundary - leftBoundary;
	/*  进一步处理的程序，，暂时不需要
	bool foundUpperBoundary = false;
	for (int j = upBoundary; j <= dowmBoundary && !foundUpperBoundary; j++) //从上到下 逐行扫描
	{
		flag = 0;
		for (int i = leftBoundary; i <= (rightBoundary - leftBoundary) * ROI - 3 && !foundUpperBoundary; i++)	//从左到右
		{
			pointer.x = i; pointer.y = j;
			next_1_pointer.x = i + 1; next_1_pointer.y = j;
			next_2_pointer.x = i + 2; next_2_pointer.y = j;
			next_3_pointer.x = i + 3; next_3_pointer.y = j;
			if ((isBlackpoint(src, pointer) == isBlackpoint(src, next_1_pointer)) && (isBlackpoint(src, next_1_pointer) == !isBlackpoint(src, next_2_pointer)) && (isBlackpoint(src, next_2_pointer) == isBlackpoint(src, next_3_pointer)))
				flag++;
		}
		if (flag >= 2)
		{
			foundUpperBoundary = true;
		}
	}
	cout << pointer << endl;
	finalRect.y = pointer.y;
	logInfo("foundUpperBoundary!");
	bool foundDownBoundary = false;
	for (int j = dowmBoundary; j >= upBoundary&& !foundDownBoundary; j--) //从下到上 逐行扫描
	{
		flag = 0;
		for (int i = leftBoundary; i <= (rightBoundary - leftBoundary) * ROI - 3 && !foundDownBoundary; i++)	//从左到右
		{
			pointer.x = i; pointer.y = j;
			next_1_pointer.x = i + 1; next_1_pointer.y = j;
			next_2_pointer.x = i + 2; next_2_pointer.y = j;
			next_3_pointer.x = i + 3; next_3_pointer.y = j;
			if ((isBlackpoint(src, pointer) == isBlackpoint(src, next_1_pointer)) && (isBlackpoint(src, next_1_pointer) == !isBlackpoint(src, next_2_pointer)) && (isBlackpoint(src, next_2_pointer) == isBlackpoint(src, next_3_pointer)))
				flag++;
		}
		if (flag >= 2)
		{
			foundDownBoundary = true;
		}
	}
	cout << pointer << endl;
	finalRect.height = pointer.y - finalRect.y;
	logInfo("foundDownBoundary!");
	bool foundLeftBoundary = false;
	for (int i = leftBoundary; i <= rightBoundary && !foundLeftBoundary; i++) //从左到右 逐列扫描
	{
		flag = 0;
		for (int j = upBoundary; j <= (dowmBoundary - upBoundary) * ROI - 3 && !foundLeftBoundary; j++)	//从上到下
		{
			pointer.x = i; pointer.y = j;
			next_1_pointer.x = i; next_1_pointer.y = j + 1;
			next_2_pointer.x = i; next_2_pointer.y = j + 2;
			next_3_pointer.x = i; next_3_pointer.y = j + 3;
			if ((isBlackpoint(src, pointer) == isBlackpoint(src, next_1_pointer)) && (isBlackpoint(src, next_1_pointer) == !isBlackpoint(src, next_2_pointer)) && (isBlackpoint(src, next_2_pointer) == isBlackpoint(src, next_3_pointer)))
				flag++;
		}
		if (flag >= 2)
		{
			foundLeftBoundary = true;
		}
	}
	cout << pointer << endl;
	finalRect.x = pointer.x;
	logInfo("foundLeftBoundary!");
	bool foundRightBoundary = false;
	for (int i = rightBoundary; i <= leftBoundary && !foundRightBoundary; i++) //从右到左 逐列扫描
	{
		flag = 0;
		for (int j = upBoundary; j <= (dowmBoundary - upBoundary) * ROI - 3 && !foundRightBoundary; j++)	//从上到下
		{
			pointer.x = i; pointer.y = j;
			next_1_pointer.x = i; next_1_pointer.y = j + 1;
			next_2_pointer.x = i; next_2_pointer.y = j + 2;
			next_3_pointer.x = i; next_3_pointer.y = j + 3;
			if ((isBlackpoint(src, pointer) == isBlackpoint(src, next_1_pointer)) && (isBlackpoint(src, next_1_pointer) == !isBlackpoint(src, next_2_pointer)) && (isBlackpoint(src, next_2_pointer) == isBlackpoint(src, next_3_pointer)))
				flag++;
		}
		if (flag >= 2)
		{
			foundRightBoundary = true;
		}
	}
	cout << pointer << endl;
	finalRect.width = pointer.x - finalRect.x;
	logInfo("foundRightBoundary!");
	*/
	Mat dst(finalRect.height, finalRect.width, CV_8UC3);//返回图片
	//cout << "Final Rect :" << finalRect << endl;
	//cout << "finalRect.height :" << finalRect.height << " finalRect.width :" << finalRect.width  << endl;
	//cout << "created" << endl;
	//cout << dst.size() << endl;
	dst = src(finalRect);
	//imshow("dst", dst);
	//waitKey(0);
	return dst;
}

int main(int argc, char* argv[])
{
	//const string path = "camera1_01.png";
	const string path = "result_stiches_final.png";
	Mat src = imread(path, 1);
	logInfo("Removing entry");
	int threshold = 9;
	Mat dst = boundaryRemover(src, threshold);
	logInfo("Removed");
	imwrite("final.png", dst);
	imshow("src", src);
	imshow("dst", dst);
	waitKey(0);
	return 0;
}