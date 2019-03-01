#include <iostream>
#include <string>
#include <sstream>
#include <opencv2/opencv.hpp>  
#include <iomanip>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/stitching/stitcher.hpp"
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv2/highgui/highgui.hpp>  
using namespace cv;
using namespace std; 
void getNextImage(Mat src, int dx, int dy, Rect scope,Mat dst)
{
	/*
	�˳������ͼ�����βƴ�ӣ�srcΪԭʼͼ��dx��dyΪ�����ƶ��ľ��룬
	scopeΪҪ��ʾ�Ĵ��ھ���dstΪ���ͼ��
	*/
	//src.x is width of src image
	//scope includs(int x,int y,int width,int height)
	//scope.x is start cooedinate of display scope
	//dst is the rect shown
	Rect newscope(scope);  //converted new scope
	cout << "src.rows" << src.rows << "src.cols" << src.cols << endl; //rows 480,cols 640
	if ( (scope.y + dy) < 0)
	{
		newscope.y = 0; //�Ͻ�
	}
	else if ( (scope.y + scope.height + dy) > src.cols)
	{
		newscope.y = (src.rows - scope.height); //�½�
	}
	else
		newscope.y = (scope.y + dy);
	if ((scope.x + dx) < 0)
	{
		newscope.x =  src.cols + (scope.x + dx) % src.cols; //���
	}
	else if ((scope.x + dx) > src.cols)
	{
		newscope.x = (scope.x + dx) % src.cols; //�ҽ�
	}
	else
		newscope.x = (scope.x + dx);
	cout << "newscope.x:" << newscope.x << " newscope.y:" << newscope.y << endl;
	Mat lefttmp, left, righttmp, right;
	cout << "src.size" << src.size() << endl;
	cout << "dst.size" << dst.size() << endl;
	if (newscope.x > (src.cols - scope.width)) //start edge stitching
	{
		lefttmp = src.colRange(newscope.x, src.cols);
		left = lefttmp.rowRange(newscope.y, newscope.y + scope.height);
		Rect leftROI(0, 0, left.cols, left.rows);
		cout << "Size of left:" << left.size() << endl;
		righttmp = src.colRange(0, scope.width - (src.cols - newscope.x));
		right = righttmp.rowRange(newscope.y, newscope.y + scope.height);
		Rect rightROI(left.cols, 0, right.cols, right.rows);
		cout << "Size of right:" << right.size() << endl;
		cout << 111 << endl;
		left.copyTo(dst(leftROI));
		right.copyTo(dst(rightROI));

	}
	else
	{
		lefttmp = src.colRange(newscope.x, newscope.x + scope.width);
		left = lefttmp.rowRange(newscope.y, newscope.y + scope.height);
		left.copyTo(dst);
	}
	//cout << "dst=" << dst << endl;
	cout << "Size of dst:" << dst.size() << endl;
	//src.copyTo(dst);
}
void main()
{
	Mat src = imread("chessboard1.png", 1);
	Rect scope(10,10,400,300); //��������ʹ�С
	int dx = -50 - 200 * scope.width, dy = 5000; //�����ƶ���
	Mat dst(scope.height, scope.width, CV_8UC3);
	getNextImage(src, dx, dy, scope,dst);
	imshow("src", src);
	imshow("dst", dst);
	waitKey(0);
	return;
}