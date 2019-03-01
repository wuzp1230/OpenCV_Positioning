#include "opencv2/imgproc.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>  
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <cstdio>
#include "opencv2/highgui.hpp"

using namespace cv;
using namespace std;

// callback function  
void cvMouseCallback(int mouseEvent, int x, int y, int flags, void* param); //回调函数
struct drawbox
{ //Struct is convenient to pass argument
	Point point1;
	Point point2;
	Mat image;
	Mat temp;
	bool isDraw;
};
String showdist(Point point1, Point point2)
{ //To calculate distance between two points and return string.
	int dstx = point1.x - point2.x;
	int dsty = point1.y - point2.y;
	int distance = sqrt(dstx * dstx + dsty * dsty);
	//int to string
	std::stringstream sstr2;
	sstr2 << distance;
	string str2;
	sstr2 >> str2;
	String text = "Distance is" + str2 + " px";
	return text;
}
int main(int argc, char* argv[])
{
	const string path = "fruits.jpg";
	// declare and initialize a struct drawbox variable  
	drawbox box;
	box.point1 = Point(0, 0);
	box.point2 = Point(0, 0);
	box.image = imread(path, 1);
	box.temp = box.image.clone();
	box.isDraw = false;
	// register a mouse callback function  
	namedWindow("exam", 1);
	//第二个参数指定窗口里每次鼠标事件发生的时候，被调用的函数指针
	cvSetMouseCallback("exam", cvMouseCallback, &box);
	while (1)
	{
		//box.temp = box.image;
		box.temp = box.image.clone(); //A temp image
		if (box.isDraw)
		{
			CvFont* cf = new CvFont();
			cvInitFont(cf, CV_FONT_HERSHEY_SIMPLEX, 1.0, 0.5); //Initialize the font
			String text = showdist(box.point1,  box.point2);
			putText(box.temp, text, box.point1, FONT_HERSHEY_DUPLEX, 0.6, CV_RGB(255, 0, 0), 1, 8, false); //Start text
			putText(box.temp, text, box.point2, FONT_HERSHEY_DUPLEX, 0.6, CV_RGB(0, 255, 0), 1, 8, false); //End text
			line(box.temp, box.point1, box.point2, Scalar(255, 255, 255), 2, 8, 0);//Draw the white dynamic line
			//rectangle(box.temp, box.point1, box.point2, Scalar(255, 255, 255));
		}
		imshow("exam", box.temp);
		if (cvWaitKey(20) == 27)  break;
	}
	//ReleaseImage(&box.image); 
	//cvReleaseImage(&box.temp); 
	cvDestroyWindow("exam");
	return 0;
}
void cvMouseCallback(int mouseEvent, int x, int y, int flags, void* param)
{ //Mouse event callback
	drawbox* box = (drawbox*)param;
	//To declare a box header for &box
	switch (mouseEvent)
	{
	case CV_EVENT_LBUTTONDOWN: //LBotton Down
		box->point1 = Point(x, y);
		box->point2 = Point(x, y);
		box->isDraw = true;
		break;
	case CV_EVENT_MOUSEMOVE: //Mouse Move
		box->point2 = Point(x, y);
		break;
	case CV_EVENT_LBUTTONUP: //LBotton Up
		box->point2 = Point(x, y);
		//rectangle(box->image, box->point1, box->point2, Scalar(0, 255, 0)); //Final rectangle
		line(box->image, box->point1, box->point2, Scalar(0, 255, 0), 2, 8, 0);//To draw the green final line
		box->isDraw = false;
		break;
	}
	return;
}