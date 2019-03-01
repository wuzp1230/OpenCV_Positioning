#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
# include <io.h>
# include <fcntl.h>
#include <windows.h>
#endif
#include <assert.h>
#include "zbar.h"    
#include <sstream>
#include <windows.h> 
#include <iomanip>
#include <opencv2/opencv.hpp>  
#include <vector>
#include "opencv2/highgui/highgui.hpp"   
#include "PNPSolver.h"

using namespace zbar;  //添加zbar名称空间    
using namespace cv;

#define IMAGE_WIDTH 1280
#define IMAGE_HEIGHT 720											// 标定板的长宽比为7:5	
#define CLEFT 0
#define Pi 3.1415926

#define pxLen 5.2*1e-3//毫米
#define focLen 508 * pxLen
#define QRLen 200//二维码长度毫米
#define QRheight 1500//二维码顶端与地面距离，毫米
#define wallZ 5810	//墙的Z坐标

#define ROI_HEIGHT IMAGE_HEIGHT
#define ROI_WIDTH (ROI_HEIGHT / 3 * 4)													// 标定板的长宽比为7:5	

#define CAP_WIDTH 1280
#define CAP_HEIGHT 720
#define ReadPosFromQR 1//是否从二维码中读取坐标
#define SHOW_2D 1
#define SHOW_3D 1
#define IS_REMAP 1
#define IS_ROI 1
double SCREEN_WIDTH = 1600;
double SCREEN_HEIGHT = 900;

int CALIB = 0;				//是否读取标定信息
int CONTRAST_ENHANCE = 1;	//是否增强对比度
int CENTER_LINE = 0;		//是否画中心基准线
const int startID = 0;		//第一个相机的标识号

/*检测可用相机数量
cnum：输入相机数量
sucCamNum：输出检测成功的相机数量
MaxDetectNum：输入最大检测设备号
sucVCs：输出成功的相机捕捉器队列（需要在函数外另外进行一次Open操作）
sucCamSeq：输出成功的初始相机顺序队列
*/
void detectCamnum(int cnum, int &sucCamNum, int MaxDetectNum, vector<VideoCapture> &sucVCs, vector<int> &sucCamSeq)
{
	vector<Mat> images;												//用于初始检测相机时采集图片队列
	vector<VideoCapture> VCs;										//用于初始检测的相机捕捉器队列
	for (int i = 0; sucCamNum < cnum && i < MaxDetectNum; i++){		//设置相机参数并检测是否可用
		cout << "正在检测第" << i << "个视频捕捉器！" << endl;
		VCs.push_back(VideoCapture(startID + i));					//初始队列放入捕捉器
		VCs[i].open(startID + i);
		VCs[i].set(CV_CAP_PROP_FRAME_WIDTH, CAP_WIDTH);
		VCs[i].set(CV_CAP_PROP_FRAME_HEIGHT, CAP_HEIGHT);
		if (CONTRAST_ENHANCE)
		{
			VCs[i].set(CV_CAP_PROP_CONTRAST, 200);					//增强对比度
		}
		//images.push_back(Mat(CAP_HEIGHT, CAP_WIDTH, CV_8UC3));
		images.push_back(Mat());									//初始化采集图片队列
		VCs[i] >> images[i];										//先检测一个图像看是否有图片数据	
		if (!images[i].data){										//无数据
			cout << "第" << i << "个视频捕捉器不能取得图像！" << endl;
		}
		else{														//有数据
			sucVCs.push_back(VideoCapture(startID + i));			//加入成功队列中
			sucCamSeq.push_back(sucCamNum);							//初始化相机序号队列
			sucCamNum++;											//成功计数器+1
			cout << "已成功将第" << sucCamNum << "个视频捕捉器加入队列！" << endl;
		}
	}
	if (sucCamNum == 0)	//未检测到，退出
	{
		cout << "未检测到视频捕捉器,不能取得图像！" << endl;
		system("PAUSE");
		return;
	}
}
/*手动调整相机顺序
sucCamNum：要调整的相机数量
sucVCs：输入、输出相机队列
sucCamSeq：输入、输出相机序号队列
原理：使相机当前所在的窗口号和应当在的窗口号对调
*/
void changeCamSeq(int sucCamNum, vector<VideoCapture> &sucVCs, vector<int> &sucCamSeq)
{
	bool showSwapInfo = true;
	int swappingCamNum = 0;	//要对调的相机的设备号（大）
	int swappingWinNum = 0;	//要对调的相机的目标窗口号（小）
	bool isConfirmed = false;			//再次确认的开关
	double scaleView = 0.7;
	int press;
	stringstream camNum;	//保存输入的相机编号
	VideoCapture tempVC;
	vector<Mat> sucImages;		//相机调整顺序时用到的图片序列
	for (int i = 0; i < sucCamNum; i++)			//为相机调整顺序做准备
	{
		sucImages.push_back(Mat());
	}
	while (1)
	{
		if (showSwapInfo)
		{
			cout << "1.请调整窗口位置，保持活跃窗口在相机画面窗口上，并保持命令行窗口可见" << endl;
			cout << "2.输入第 " << swappingWinNum << " 个相机当前所在的窗口编号，按回车键确定：" << endl;
			showSwapInfo = false;
		}
		for (int i = 0; i < sucCamNum; i++)	//读取图像
		{
			sucVCs[i] >> sucImages[i];
			//cout << "成功获取第" << i << "个图像" << endl;
			//resize(sucImages[i], sucImages[i], Size(), scaleView, scaleView, 1);
			imshow(to_string(i), sucImages[i]);
			//cout << sucImages[i].size() << endl;
		}
		press = waitKey(10);

		if (press >= 48 && press <= 57)	//0  -  9  48   -  57
		{
			//cout << press - 48 << endl;
			camNum << press - 48;
			cout << camNum.str();
		}
		else if (press == 13)			//回车13	空格32  
		{
			camNum >> swappingCamNum;
			camNum.clear();
			if (!isConfirmed)				//未确认
			{
				cout << endl;
				if (swappingCamNum >= sucCamNum)	//相机号超界，不合法
				{
					cout << "相机号超界，请重新输入" << endl;
					camNum.str("");
					isConfirmed = false;
				}
				else
				{
					cout << "确认将第 " << swappingCamNum << " 个相机的画面和第" << swappingWinNum << "个窗口的画面交换吗？" << endl;
					cout << "再次按回车键确认，按Esc键取消" << endl;
					isConfirmed = true;
				}
			}
			else //isConfirmed
			{
				//对调
				tempVC = sucVCs[swappingCamNum];
				sucVCs[swappingCamNum] = sucVCs[swappingWinNum];
				sucVCs[swappingWinNum] = tempVC;
				//序号对调
				int tempSeq = sucCamSeq[swappingCamNum];
				sucCamSeq[swappingCamNum] = sucCamSeq[swappingWinNum];
				sucCamSeq[swappingWinNum] = tempSeq;

				swappingWinNum++;
				isConfirmed = false;
				showSwapInfo = true;	//对调完毕，显示下一个相机的信息
				camNum.str("");
				if (swappingWinNum == sucCamNum - 1)	//全部对调完毕，开始保存文件
				{
					char filename[] = "camera_seq.xml";
					FileStorage fs(filename, FileStorage::WRITE);
					if (!fs.isOpened())
					{
						cout << "打开失败！" << endl;
					}
					else
					{
						fs << "seq" << sucCamSeq;
					}
					cout << "相机顺序已保存至" << filename << endl;
					fs.release();

					break;//对调过程完毕，退出
				}
			}

		}
		else if (press == 27)		//Esc
		{
			if (!isConfirmed)
			{		//退出
				cout << "已退出调整" << endl;
				break;
				//isConfirmed = true;
			}
			else
			{
				cout << "已取消此次调整" << endl;
				isConfirmed = false;//取消确认状态
			}
		}
	}
}
/*从配置文件中读取相机序号
filename：文件名
sucCamNum：相机数量
sucVCs：相机队列
sucCamSeq：相机序号队列
原理：读取相机序号，通过序号将相机调整至适当的顺序
*/
void readCamSeq(char *filename, int sucCamNum, vector<VideoCapture> &sucVCs, vector<int> &sucCamSeq)
{
	FileStorage fs(filename, FileStorage::READ);
	if (!fs.isOpened())
	{
		cout << "打开失败！" << endl;
	}
	else
	{
		fs["seq"] >> sucCamSeq;
	}
	cout << "相机顺序已读取" << filename << endl;
	fs.release();
	Vector<VideoCapture> tmpsucCamSeq;
	for (int i = 0; i < sucCamNum; i++)	//初始化temp队列
	{
		tmpsucCamSeq.push_back(sucVCs[i]);
	}
	for (int i = 0; i < sucCamNum; i++)
	{
		sucVCs[i] = tmpsucCamSeq[sucCamSeq[i]];
	}
	cout << "相机顺序已调整" << filename << endl;
	//对调过程完毕，退出
}
/*
根据threshold阈值寻找src图像中的所有边缘，返回Mat图像
*/
Mat findMoreContours(Mat src, int threshold)
{
	Mat dst = Mat::zeros(src.rows, src.cols, CV_8UC3);
	Mat imageGrayMoreThres = src > threshold;
	cvtColor(imageGrayMoreThres, imageGrayMoreThres, CV_RGB2GRAY);
	vector<vector<Point>> contours;
	vector<Vec4i>hierarchy; //hierarchy[ i ][ 0 ] ~hierarchy[ i ][ 3 ]，分别表示后一个轮廓、前一个轮廓、父轮廓、内嵌轮廓的索引编号
	findContours(imageGrayMoreThres, contours, hierarchy, CV_RETR_CCOMP, CV_RETR_TREE);
	if (!contours.empty() && !hierarchy.empty())
	{
		int idx = 0;
		for (; idx >= 0; idx = hierarchy[idx][0])
		{
			Scalar color(255, 255, 255);
			drawContours(dst, contours, idx, color, 1, 8, hierarchy);	//第idx个轮廓
		}
	}
	waitKey(1);
	return dst;
}
/*
计算P1和P2两点的距离，返回double值
*/
double calTwoPointDist(Point P1, Point P2)
{
	return(sqrt((P2.x - P1.x)*(P2.x - P1.x) + (P2.y - P1.y)*(P2.y - P1.y)));
}

void readPointsFromText(String pos_str, vector<Point3f> &points, int edgeLen, int upHeight, int Z)
{
	char pos_char[100];
	strcpy(pos_char, pos_str.c_str());
	int LeftX = ((int)pos_char[0] - 48) * 1000 + ((int)pos_char[1] - 48) * 100;
	int RightX = pos_char[2] == '+' ? LeftX + edgeLen : LeftX - edgeLen;
	Z = pos_char[2] == '+' ? 0 : Z;
	int downHeight = upHeight - edgeLen;
	points.push_back(Point3f(RightX, upHeight, Z));
	points.push_back(Point3f(RightX, downHeight, Z));
	points.push_back(Point3f(LeftX, downHeight, Z));
	points.push_back(Point3f(LeftX, upHeight, Z));
}
/*
计算Z轴在图像中的点
输入二维码的位姿和二维码四个点  输出Z点应在的位置
*/
void calcZImagePoint(Point3f angles, Point3f coords, Vector<Point> points, Point &Z)
{
	double alpha = -(angles.x + 5)*CV_PI / 180.f;//x轴俯仰  俯负仰正
	alpha = alpha > 0 ? alpha - CV_PI : alpha + CV_PI;
	//alpha = -alpha;
	double beta = -(angles.y - 10)*CV_PI / 180.f;//y轴左右 左大为正
	double gamma = angles.z*CV_PI / 180.f;//z轴左旋右旋  逆时针为正
	gamma = gamma > 0 ? gamma - CV_PI : gamma + CV_PI;
	//gamma = -gamma;
	//cout << alpha << endl << beta << endl << gamma << endl;

	double L = coords.z;
	double x0 = points[3].x, y0 = points[3].y, z0 = abs(QRLen * 5 * focLen / L / pxLen);
	double x2, y2, z2;
	x2 = z0*cos(alpha)*sin(beta); y2 = -z0*sin(alpha);
	//x2左大为正 y2仰正  P从x轴正方向开始逆时针转动
	//cout << "x2 :" << x2 << " y2: " << y2 << endl;
	double oz = sqrt(x2*x2 + y2*y2);
	double alpha0 = atan(y2 / x2);
	if (beta < 0) alpha0 += CV_PI;
	alpha0 -= gamma;
	double xz, yz;
	yz = oz*sin(alpha0);
	xz = oz*cos(alpha0);
	double new_xz, new_yz;
	new_xz = x0 + xz; new_yz = y0 + yz;
}
/*
计算绝对角度
输入相对位置 相对坐标队列 地图 二维码四角世界坐标 输出二维码绝对坐标 绝对角度队列
*/
void calcAbsAngleCoord(vector<Point3f> pos_a, vector<Point3f> coord_a, vector<Point3f> points, vector<Point3f> &abs_pos_a, vector<Point3f> &abs_coord_a)
{
	Point3f tmp_pos, tmp_coord;//临时存放姿态、坐标
	if (points[3].x < points[0].x)
	{//0墙正方向，[3]>[0]
		//cout << "0墙正方向" << endl;
		for (int i = 0; i < pos_a.size(); i++)
		{
			abs_coord_a.push_back(coord_a[i]);
			abs_pos_a.push_back(pos_a[i]);
		}
	}
	else
	{//581墙反方向，[3]<[0]
		for (int i = 0; i < pos_a.size(); i++)
		{
			abs_coord_a.push_back(coord_a[i]);
			tmp_pos.x = pos_a[i].x + 180;
			tmp_pos.y = -pos_a[i].y + 180;
			tmp_pos.z = pos_a[i].z + 180;
			abs_pos_a.push_back(tmp_pos);
		}
	}
}

/*
计算相机位姿平均值
输入地图中所有绝对位姿队列  输出一个平均的位姿点
*/
void calcAvgPos(vector<Point3f> abs_pos_a, vector<Point3f> abs_coord_a, Point3f &finalPos_a, Point3f &finalCoord_a)
{
	double posX = 0, posY = 0, posZ = 0;
	double coordX = 0, coordY = 0, coordZ = 0;
	/*cout << "adding" << endl;
	cout << "abs_pos_a.size()" << abs_pos_a.size() << endl;
	cout << "abs_pos_a" << abs_pos_a << endl;
	cout << "abs_coord_a" << abs_coord_a << endl;*/
	for (int i = 0; i < abs_pos_a.size(); i++)
	{
		posX += abs_pos_a[i].x;
		posY += abs_pos_a[i].y;
		posZ += abs_pos_a[i].z;
		coordX += abs_coord_a[i].x;
		coordY += abs_coord_a[i].y;
		coordZ += abs_coord_a[i].z;
	}
	posX /= abs_pos_a.size();
	posY /= abs_pos_a.size();
	posZ /= abs_pos_a.size();
	coordX /= abs_pos_a.size();
	coordY /= abs_pos_a.size();
	coordZ /= abs_pos_a.size();
	finalPos_a.x = posX;
	finalPos_a.y = posY;
	finalPos_a.z = posZ;
	finalCoord_a.x = coordX;
	finalCoord_a.y = coordY;
	finalCoord_a.z = coordZ;
}
/*
通过相机旋转平移矩阵计算另一个相机的旋转平移
*/
void calcRightPos(Point3f finalPos_a, Point3f finalCoord_a, Mat R, Mat T, Point3f &rightPos_a, Point3f &rightCoord_a)
{//A = R12*B + T12
	Mat rightCoordMat, rightPosMat;
	Mat tmpfinalPos_a = (Mat_<double>(3, 1) << finalPos_a.x, finalPos_a.y, finalPos_a.z); //R2
	Mat tmpfinalCoord_a = (Mat_<double>(3, 1) << finalCoord_a.x, finalCoord_a.y, finalCoord_a.z); //T2
	Mat RodR, RodR2;
	//cout << "R " << R << endl;
	Rodrigues(R, RodR);
	Rodrigues(RodR, RodR);
	//cout << "RodR " << RodR << endl;
	//cout << "T " << T << endl;
	//cout << "tmpfinalPos_a" << tmpfinalPos_a << endl;
	rightPosMat = R * tmpfinalPos_a; //R1 = R * R2
	//cout << "rightPosMat" << rightPosMat << endl;
	Rodrigues(rightPosMat, RodR2);
	//cout << "RodR2" << RodR2 << endl;
	rightCoordMat = RodR2 * tmpfinalCoord_a + T; //T1 = R * T2 + T
	//rightCoordMat = R.inv() * (tmpfinalCoord_a - T);//T1 = R-1 * (T2 - T)
	//cout << "rightCoordMat" << rightCoordMat << endl;
	//cout << "rightPosMat" << rightPosMat << endl;
	rightPos_a.x = rightPosMat.at<double>(0, 0);
	rightPos_a.y = rightPosMat.at<double>(1, 0);
	rightPos_a.z = rightPosMat.at<double>(2, 0);

	rightCoord_a.x = finalCoord_a.x + T.at<double>(0, 0) * cos((finalPos_a.y + 180) / 180 * CV_PI);
	rightCoord_a.z = finalCoord_a.z + T.at<double>(0, 0) * sin((finalPos_a.y + 180) / 180 * CV_PI);
	//rightCoord_a.x = rightCoordMat.at<double>(0, 0);
	rightCoord_a.y = finalCoord_a.y + T.at<double>(0, 0) * sin((finalPos_a.z + 180) / 180 * CV_PI);
	//rightCoord_a.z = rightCoordMat.at<double>(2, 0);
}

/*
在地图中绘制相机点
输入相机绝对世界坐标  以毫米为单位 相机颜色BGR 姿态轴颜色 输入输出绘制过的相机图
*/
void drawCamOnMap(Point3f finalPos_a, Point3f finalCoord_a, Point textShift, Scalar scCam, Scalar scPos, Scalar scText, Mat &mapImage)
{//2D相机半对角线长19pxBGR(255,255,0)  相机姿态轴长63px(0,130,255)	3D相机半对角线长19px(255,0,255)
	Point2f camCoordPoint;		//记录相机坐标所在点
	Point2f camPosPoint;		//相机姿态的连线
	camCoordPoint.x = finalCoord_a.x / 10;
	camCoordPoint.y = finalCoord_a.z / 10;
	vector<Point> camBorder4Points;
	//计算相机轮廓四个点
	double finalPosy = finalPos_a.y + 180;
	double camLen = 5;
	camBorder4Points.push_back(Point(camCoordPoint.x + camLen * cos((finalPosy + 45) / 180 * CV_PI), camCoordPoint.y + camLen * sin((finalPosy + 45) / 180 * CV_PI)));
	camBorder4Points.push_back(Point(camCoordPoint.x + camLen * cos((finalPosy + 135) / 180 * CV_PI), camCoordPoint.y + camLen * sin((finalPosy + 135) / 180 * CV_PI)));
	camBorder4Points.push_back(Point(camCoordPoint.x + camLen * cos((finalPosy + 225) / 180 * CV_PI), camCoordPoint.y + camLen * sin((finalPosy + 225) / 180 * CV_PI)));
	camBorder4Points.push_back(Point(camCoordPoint.x + camLen * cos((finalPosy + 315) / 180 * CV_PI), camCoordPoint.y + camLen * sin((finalPosy + 315) / 180 * CV_PI)));
	//cout << "camBorder4Points" <<camBorder4Points << endl;
	//计算姿态轴连线
	camPosPoint.x = camCoordPoint.x + 63 * cos((finalPosy + 90) / 180 * CV_PI);
	camPosPoint.y = camCoordPoint.y + 63 * sin((finalPosy + 90) / 180 * CV_PI);

	if (scPos != Scalar(255, 255, 255))
	{
		//画相机姿态轴
		line(mapImage, camCoordPoint, camPosPoint, scPos, 2, 8, 0);
	}
	if (scCam != Scalar(255, 255, 255))
	{	//画相机轮廓线
		//line(mapImage, camBorder4Points[0], camBorder4Points[1], scCam, 1, 8, 0);
		line(mapImage, camBorder4Points[0], camBorder4Points[1], Scalar(0, 0, 0), 2, 8, 0);
		line(mapImage, camBorder4Points[2], camBorder4Points[1], scCam, 2, 8, 0);
		line(mapImage, camBorder4Points[2], camBorder4Points[3], scCam, 2, 8, 0);
		line(mapImage, camBorder4Points[0], camBorder4Points[3], scCam, 2, 8, 0);
	}
	if (scText != Scalar(255, 255, 255))
	{
		//cout << "line" << endl;
		stringstream coordStream;
		coordStream << "X:" << (int)camCoordPoint.x << ",Z:" << (int)camCoordPoint.y << ")";
		putText(mapImage, coordStream.str(), Point(camCoordPoint.x + textShift.x - 25, camCoordPoint.y + textShift.y), FONT_HERSHEY_DUPLEX, 0.6, scText, 1, 8, false);
		stringstream coordStream2;
		coordStream2 << "Height;" << (int)finalCoord_a.y / 10;
		putText(mapImage, coordStream2.str(), Point(camCoordPoint.x + textShift.x - 25, camCoordPoint.y + textShift.y + 20), FONT_HERSHEY_DUPLEX, 0.6, scText, 1, 8, false);
		//cout << "putText" << endl;
	}
}

/*
识别QR二维码，返回多个二维码四角的二维坐标
输入图片pic, 右图rightPic, 输出二维码文本text, 输出左目二维码四个点Points2D
*/
bool recognitionQR(Mat& pic, String &text, vector<Point2f>& Points2D)
{
	Mat imgGray;
	bool isSuccess = false;
	cvtColor(pic, imgGray, CV_RGB2GRAY);
	ImageScanner scanner;
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
	int width = pic.cols;
	int height = pic.rows;
	uchar *raw = (uchar *)imgGray.data;
	Image imageZbar(width, height, "Y800", raw, width * height);
	scanner.scan(imageZbar); //扫描条码      
	Image::SymbolIterator symbol = imageZbar.symbol_begin();
	if (imageZbar.symbol_begin() == imageZbar.symbol_end())
	{
		cout << "查询条码失败，请检查图片！" << endl;
		return false;
	}
	for (; symbol != imageZbar.symbol_end(); ++symbol)
	{
		if (symbol->get_location_size() == 4)
		{//二维码直径2.4cm 0 - 1 - 2- 3  右上  右下  左下 左上
			isSuccess = true;
			text = symbol->get_data();
			Points2D.push_back(Point(symbol->get_location_x(0), symbol->get_location_y(0)));
			Points2D.push_back(Point(symbol->get_location_x(1), symbol->get_location_y(1)));
			Points2D.push_back(Point(symbol->get_location_x(2), symbol->get_location_y(2)));
			Points2D.push_back(Point(symbol->get_location_x(3), symbol->get_location_y(3)));
		}
	}
	return isSuccess;
}
/*
根据左右二维码图和Q计算深度，返回视差d和Z物理坐标
输入左图leftPic, 右图rightPic, 输出视差d, 输出左目二维码四个点leftPoints2D, 输出右目二维码四个点rightPoints2D, 输出双目形心doubleCenter, 输出二维码文本QRText, 输入双目Q矩阵leftQ
*/
bool cadlculateQR(Mat& leftPic, Mat& rightPic, int &d, vector<Point2f> &leftPoints2D, vector<Point2f> &rightPoints2D, vector<Point> &doubleCenter, String &QRText, Mat leftQ)
{
	String textL, textR;
	bool isRecognizedL = recognitionQR(leftPic, textL, leftPoints2D);
	bool isRecognizedR = recognitionQR(rightPic, textR, rightPoints2D);
	if (isRecognizedL && isRecognizedR && textL == textR)
	{
		QRText = textR;
		float leftX = (leftPoints2D[0].x + leftPoints2D[1].x + leftPoints2D[2].x + leftPoints2D[3].x) / 4;
		float rightX = (rightPoints2D[0].x + rightPoints2D[1].x + rightPoints2D[2].x + rightPoints2D[3].x) / 4;
		float leftY = (leftPoints2D[0].y + leftPoints2D[1].y + leftPoints2D[2].y + leftPoints2D[3].y) / 4;
		float rightY = (rightPoints2D[0].y + rightPoints2D[1].y + rightPoints2D[2].y + rightPoints2D[3].y) / 4;
		Point leftCenter(leftX, leftY);
		Point rightCenter(rightX, rightY);
		doubleCenter.push_back(leftCenter);
		doubleCenter.push_back(rightCenter);
		d = leftX - rightX;
		//d:视差值
		//doffs：视差偏移值
		//baseline：基线距离
		//f：焦距
		//disSc：视差缩放因子, 默认为1 针对不同视差算法生成的视差图可能在标准256位基础上乘上或除以缩放因子
		//P：二维图像的投影矩阵
		double doffs = 0, baseline = 0, f = 0, Z = 0;
		baseline = 1 / (leftQ.at<double>(3, 2));
		doffs = baseline*(leftQ.at<double>(3, 3));
		f = leftQ.at<double>(2, 3);
		Z = baseline * f / (d + doffs);
		cout << "cadlculateQR Success" << endl;
		return true;
	}
	else
	{
		cout << "cadlculateQR False" << endl;
		return false;
	}
}
/*
直接输入一个二维坐标和对应的视差值，并通过重投影矩阵生成三维点
*/
void point_to_xyz(float x, float y, float d, Mat Q, Point3f& point)  //直接输入一个二维坐标和对应的视差值，并通过重投影矩阵生成三维点
{

	float X = 0;
	float Y = 0;
	float Z = 0;
	double doffs, baseline, f;
	baseline = 1 / (Q.at<double>(3, 2));
	doffs = baseline*(Q.at<double>(3, 3));
	f = Q.at<double>(2, 3);
	double cx = Q.at<double>(0, 3);
	double cy = Q.at<double>(1, 3);

	int d_cur = d;
	Z = baseline * f / (d_cur + doffs);
	X = (x + cx) * (Z / f);
	Y = (y + cy) * (Z / f);

	point.x = X;
	point.y = Y;
	point.z = Z;

}


int main(int argc, const char *argv[])
{
	VideoCapture captureLeft;

	int MaxDetectNum = 15;	//最大检测相机数量
	int sucCamNum = 0;	//成功读取的相机计数器
	vector<VideoCapture> sucVCs;	//成功读取的相机捕捉器序列
	vector<int> sucCamSeq;			//成功读取的相机序号
	int cnum = 2;
	/**********				检测可用相机数量			*****************/
	detectCamnum(cnum, sucCamNum, MaxDetectNum, sucVCs, sucCamSeq);
	cout << "成功加入相机队列的相机数量： " << sucCamNum << endl;
	cout << "可用相机队列规模：" << sucVCs.size() << endl;
	vector<Mat> image;			//临时队列
	vector<Mat> sucImages;		//拍照时用到的图片序列
	for (int i = 0; i < sucCamNum; i++)			//为相机调整顺序做准备
	{
		sucImages.push_back(Mat());
		image.push_back(Mat());
	}
	for (int i = 0; i < sucCamNum; i++){		//设置相机参数并检测是否可用
		sucVCs[i].open(startID + i);
		sucVCs[i].set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));
		sucVCs[i].set(CV_CAP_PROP_FRAME_WIDTH, CAP_WIDTH);
		sucVCs[i].set(CV_CAP_PROP_FRAME_HEIGHT, CAP_HEIGHT);
		sucVCs[i] >> image[i];	//先检测一个图像	
		if (!image[i].data){
			cout << "第" << i << "个视频捕捉器不能取得图像！" << endl;
		}
		else{
			cout << "第" << i << "个视频捕捉器检测成功！" << endl;
		}
	}

	if (sucCamNum > 1)
	{
		char seqChoice;
		cout << "				请选择调整顺序选项" << endl;
		cout << "1.调整顺序并保存	2.从配置文件中读取顺序	3.不读取也不调整" << endl;
		do{
			cin >> seqChoice;
		} while (seqChoice <'1' || seqChoice > '3');
		/**	1.调整顺序并保存  2.从配置文件中读取顺序   3.不读取也不调整	****/
		if (seqChoice == '1')		//1.调整顺序并保存
		{
			/****************		改变相机顺序	************/
			changeCamSeq(sucCamNum, sucVCs, sucCamSeq);
		}
		else if (seqChoice == '2')		//	2.从配置文件中读取顺序	
		{
			char filename[] = "camera_seq.xml";
			readCamSeq(filename, sucCamNum, sucVCs, sucCamSeq);
		}
		else if (seqChoice == '3')		//	3.不读取也不调整
		{
		}
		destroyAllWindows();
	}

	int exposure = -7;//曝光
	Mat mapImage = imread("Map.png", 1);
	Mat tmp_image, tmp_map_image;
	ImageScanner scanner;
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
	sucVCs[0] >> tmp_image;				// 正常读取当前帧
	//cvtColor(tmp_image, imageGray, CV_RGB2GRAY);
	int width = tmp_image.cols;
	int height = tmp_image.rows;
	Mat imgContours;
	Point O;//二维码中心点像素坐标
	long int framesum = 0;
	bool isSuccess = false;
	bool showSuccessRate = false;//是否显示成功率
	int p = 0;
	if (SHOW_2D)
	{
		p += sucCamNum + 1;	//+1为Map
	}
	if (SHOW_3D)
	{
		p += sucCamNum - 1;	//-1为左目
	}
	p = ceil(sqrt(p + 1));	//摄像机平方取天棚，+2为了预留命令行
	const int detectFrameSum = 30;//检测成功率时的检测范围
	int sucRate[detectFrameSum];

	/****************	读取标定信息	************/
	if (sucCamNum > 1)
	{
		cout << "是否读取标定信息" << endl;
		cout << "1 : 读取  2 : 不读取" << endl;
		int choiceCalib;
		do
		{
			cin >> choiceCalib;
		} while (choiceCalib != 1 && choiceCalib != 2);
		if (choiceCalib == 1)
		{
			CALIB = 1;	//是否读取标定信息
		}
		else CALIB = 0;
	}

	Vector<Vector<Mat>> recImagesVect;		//校正后图像对序列	[N][2]
	vector<vector<vector<Mat>>> rmapFinalVect;			//相机映射最终队列rmapFinalVect[相机对数序号][左右][XY]
	Vector<Vector<Mat>> cameraMatrixVect, distCoeffsVect;		//内参、畸变系数序列	[N][2]
	Vector<Mat> R1Vect, R2Vect, P1Vect, P2Vect, RVect, TVect, QVect;	//相机参数	[N]
	Vector<Rect> RoiVect;				//相机ROI序列
	Vector<Mat> picLeftROI, picRightROI;			//取过感兴趣区域的图像
	//读取标定信息
	if (CALIB)
	{
		for (int i = 0; i < sucCamNum - 1; i++)
		{
			stringstream ss;
			ss << "camera" << setw(2) << setfill('0') << i + 1 << "_results.xml"; //.\camera01_results.xml - .\cameraNN_results.xml
			String filename = ss.str();
			Rect Roi[2];						//每一个相机的左右ROI临时存放的位置
			int x1, x2, x3, x4, y1, y2, y3, y4, xx = 0, yy = 0, xx2, yy2, h = CAP_HEIGHT, w = CAP_WIDTH;	//感兴趣区域序列，读取即使用
			Mat R, T, R1, R2, P1, P2, Q;		//每一个相机的相机参数临时存放的位置
			FileStorage fs(filename, FileStorage::READ);
			Mat tmpcameraMatrix, tmpdistCoeffs;
			Vector<Mat> cameraMatrix, distCoeffs;
			if (!fs.isOpened())
			{
				cout << "标定信息不存在！" << endl;
			}
			else
			{
				fs["Q"] >> Q;
				fs["Roi1"] >> Roi[0]; fs["Roi2"] >> Roi[1];
				fs["cameraMatrix1"] >> tmpcameraMatrix;
				fs["distCoeffs1"] >> tmpdistCoeffs;
				cameraMatrix.push_back(tmpcameraMatrix);
				distCoeffs.push_back(tmpdistCoeffs);
				fs["R1"] >> R1; fs["P1"] >> P1;
				fs["cameraMatrix2"] >> tmpcameraMatrix;
				fs["distCoeffs2"] >> tmpdistCoeffs;
				cameraMatrix.push_back(tmpcameraMatrix);
				distCoeffs.push_back(tmpdistCoeffs);
				fs["R2"] >> R2; fs["P2"] >> P2;
				fs["R"] >> R; fs["T"] >> T;
				RVect.push_back(R); TVect.push_back(T);
				fs.release();
				QVect.push_back(Q);
				cameraMatrixVect.push_back(cameraMatrix);	//[相机号][左右目][内参矩阵]
				distCoeffsVect.push_back(distCoeffs);		//[相机号][左右目][畸变矩阵]
				R1Vect.push_back(R1); P1Vect.push_back(P1);
				R2Vect.push_back(R2); P2Vect.push_back(P2);
				x1 = Roi[0].x; x2 = Roi[1].x;//ROI上界坐标
				x3 = Roi[0].x + Roi[0].width;//ROI下界坐标
				x4 = Roi[1].x + Roi[1].width;
				y1 = Roi[0].y; y2 = Roi[1].y;
				y3 = Roi[0].y + Roi[0].height;
				y4 = Roi[1].y + Roi[1].height;
				xx = max(x1, x2); yy = max(y1, y2);//取共同区域
				xx2 = min(x3, x4); yy2 = min(y3, y4);
				h = xx2 - xx; w = yy2 - yy;
				RoiVect.push_back(Rect(xx, yy, h, w));
			}
		}
		cout << "已读取标定信息" << endl;
	}
	//构造标定信息，只适用于单目
	else
	{
		for (int i = 0; i < sucCamNum; i++)	//读取标定信息
		{
			Mat tmpcameraMatrix = (Mat_<double>(3, 3) << 6.8242385570116965e+002, 0, 6.7208830213119222e+002, 0, 6.8244286335348984e+002, 3.8317059885462112e+002, 0, 0, 1);
			Mat tmpdistCoeffs = (Mat_<double>(1, 5) << -4.2074813851279880e-001, 2.7307760828952093e-001, 7.7173259007015649e-004, 4.0879799982298722e-006, -1.2994328103190725e-001);
			//Mat tmpcameraMatrix, tmpdistCoeffs;
			//cout << "tmpcameraMatrix" << tmpcameraMatrix << endl;
			//cout << "tmpdistCoeffs" << tmpdistCoeffs << endl;
			Vector<Mat> cameraMatrix, distCoeffs;
			cameraMatrix.push_back(tmpcameraMatrix);
			distCoeffs.push_back(tmpdistCoeffs);
			cameraMatrixVect.push_back(cameraMatrix);
			distCoeffsVect.push_back(distCoeffs);
		}
	}
	Mat rmapX, rmapY;						//单个相机的X、Y rmap

	if (CALIB)
	{
		for (int i = 0; i < sucCamNum - 1; i++)
		{
			picLeftROI.push_back(sucImages[i]);					//初始化picLeftROI队列
			picRightROI.push_back(sucImages[i + 1]);
			vector<Mat> recImages;
			recImages.push_back(sucImages[i]);
			recImages.push_back(sucImages[i + 1]);
			recImagesVect.push_back(recImages);			//初始化recImagesVect队列
		}
		//初始化remap矩阵
		for (int i = 0; i < sucCamNum - 1; i++){
			sucVCs[i] >> sucImages[i];
			sucVCs[i + 1] >> sucImages[i + 1];
			picLeftROI[i] = sucImages[i];					//初始化picLeftROI队列
			picRightROI[i] = sucImages[i + 1];
			vector<Mat> rampL, rmapR;				//一对左右相机的rmap
			vector<vector<Mat>> rmapVect;					//相机映射队列
			initUndistortRectifyMap(cameraMatrixVect[i][0], distCoeffsVect[i][0], R1Vect[i], P1Vect[i], picLeftROI[i].size(), CV_16SC2, rmapX, rmapY);	//去畸变
			rampL.push_back(rmapX); rampL.push_back(rmapY);
			rmapVect.push_back(rampL);
			initUndistortRectifyMap(cameraMatrixVect[i][1], distCoeffsVect[i][1], R2Vect[i], P2Vect[i], picLeftROI[i].size(), CV_16SC2, rmapX, rmapY);	//去畸变
			rmapR.push_back(rmapX); rmapR.push_back(rmapY);
			rmapVect.push_back(rmapR);
			rmapFinalVect.push_back(rmapVect); //[相机对数序号][左右][XY]
		}
	}
	HWND hwnd = GetForegroundWindow();
	while (1)
	{
		int counter = 0;//二维和三维得到的位置
		int windowNum = 0;
		vector<String> inf;			//多个图片中可能出现多个条形码的内容[单个相机的条形码内容]
		vector<Mat> a;					//未校正的图片队列
		vector<int> sucSerial;			//检测成功的序号
		Mat imageGray, imageGrayMore100;
		Mat tmpMapImage; mapImage.copyTo(tmpMapImage);//用于画图的临时地图
		Mat tmpImage;					//用于绘制二维码的相机临时图
		Point3f all_pos, all_coord;
		//每一个相机读取图像  二维识别位姿
		system("CLS");
		for (int i = 0; i < sucCamNum - 1; i++)
		{
			vector<Point3f> abs_pos_a;					//每个相机二维方式测量得到的角度队列 [每个出现的的坐标（可以允许多个）]k（斜率表示）
			vector<Point3f> abs_coord_a;				//每个相机二维方式测量得到的坐标绝对队列 [每个出现的的坐标（可以允许多个）]Mat(X,Y)
			sucVCs[i] >> sucImages[i];
			//显示二维标定方式
			if (SHOW_2D)
			{
				sucImages[i].copyTo(tmpImage);
				cvtColor(tmpImage, imageGray, CV_RGB2GRAY);
				a.push_back(imageGray);
				uchar *raw = (uchar *)imageGray.data;
				Image imageZbar(width, height, "Y800", raw, width * height);
				scanner.scan(imageZbar); //扫描条码    
				Image::SymbolIterator symbol = imageZbar.symbol_begin();
				if (imageZbar.symbol_begin() == imageZbar.symbol_end())
				{
					cout << "查询条码失败，请检查图片！" << endl;
				}
				//一个相机的所有二维码，每个合格的二维码都可以算出来一个位姿
				for (; symbol != imageZbar.symbol_end(); ++symbol)
				{
					if (symbol->get_location_size() == 4)	//合格的二维码 
					{
						vector<Point3f> pos_a;					//未校正图片中原坐标系的角度队列 [每个出现的的坐标（可以允许多个）]Mat(X,Y,Z)
						vector<Point3f> coord_a;					//未校正图片中距离原点的坐标队列 [每个出现的的坐标（可以允许多个）]Mat(X,Y,Z)
						//cout << "合格的二维码" << endl;
						isSuccess = true;
						vector<Point> points;
						points.push_back(Point(symbol->get_location_x(0), symbol->get_location_y(0)));
						points.push_back(Point(symbol->get_location_x(1), symbol->get_location_y(1)));
						points.push_back(Point(symbol->get_location_x(2), symbol->get_location_y(2)));
						points.push_back(Point(symbol->get_location_x(3), symbol->get_location_y(3)));
						line(tmpImage, Point(symbol->get_location_x(0), symbol->get_location_y(0)), Point(symbol->get_location_x(1), symbol->get_location_y(1)), Scalar(0, 0, 255), 2, 8, 0);
						line(tmpImage, Point(symbol->get_location_x(1), symbol->get_location_y(1)), Point(symbol->get_location_x(2), symbol->get_location_y(2)), Scalar(255, 0, 255), 2, 8, 0);
						line(tmpImage, Point(symbol->get_location_x(2), symbol->get_location_y(2)), Point(symbol->get_location_x(3), symbol->get_location_y(3)), Scalar(255, 0, 0), 2, 8, 0);
						line(tmpImage, Point(symbol->get_location_x(3), symbol->get_location_y(3)), Point(symbol->get_location_x(0), symbol->get_location_y(0)), Scalar(0, 255, 0), 2, 8, 0);

						//初始化PNPSolver类
						PNPSolver p4psolver;
						//初始化相机参数
						double fx = (cameraMatrixVect[i][0].at<double>(Point(0, 0)));
						double fy = (cameraMatrixVect[i][0].at<double>(Point(1, 1)));
						double u0 = cameraMatrixVect[i][0].at<double>(Point(2, 0));
						double v0 = cameraMatrixVect[i][0].at<double>(Point(2, 1));
						p4psolver.SetCameraMatrix(fx, fy, u0, v0);

						double k1 = distCoeffsVect[i][0].at<double>(Point(0, 0));
						double k2 = distCoeffsVect[i][0].at<double>(Point(1, 0));
						double p1 = distCoeffsVect[i][0].at<double>(Point(2, 0));
						double p2 = distCoeffsVect[i][0].at<double>(Point(3, 0));
						double k3 = distCoeffsVect[i][0].at<double>(Point(4, 0));
						p4psolver.SetDistortionCoefficients(k1, k2, p1, p2, k3);
						//设置特征点的世界坐标
						if (ReadPosFromQR)
						{//从二维码中读取位姿
							String pos_str = symbol->get_data();
							cout << "类型：" << endl << symbol->get_type_name() << endl << endl;
							cout << "条码：" << endl << symbol->get_data() << endl << endl;
							//将条码内容放进队列
							inf.push_back(pos_str);
							//从文本中读取点
							readPointsFromText(pos_str, p4psolver.Points3D, QRLen, QRheight, wallZ);
							//记录当前消息的相机序号
							sucSerial.push_back(i);
						}
						else
						{//默认左上角为（0 0 0）
							p4psolver.Points3D.push_back(cv::Point3f(0, 0, 0));     //P1三维坐标的单位是毫米
							p4psolver.Points3D.push_back(cv::Point3f(0, QRLen, 0));   //P2
							p4psolver.Points3D.push_back(cv::Point3f(QRLen, QRLen, 0));  //P4
							p4psolver.Points3D.push_back(cv::Point3f(QRLen, 0, 0));   //P3
						}
						//cout << "p4psolver.Points3D" << p4psolver.Points3D << endl;
						//cout << "test2:特征点世界坐标 = " << endl << p4psolver.Points3D << endl;
						//设置特征点的图像坐标
						p4psolver.Points2D.push_back(points[3]);  //P4
						p4psolver.Points2D.push_back(points[2]);  //P3
						p4psolver.Points2D.push_back(points[1]);  //P2
						p4psolver.Points2D.push_back(points[0]);  //P1

						//cout << "test2:图中特征点坐标 = " << endl << p4psolver.Points2D << endl;
						//double area = contourArea(points, false);
						//cout << "二维码占 " << area << " 像素" << endl << "占比 " << area *100/ (tmpImage.cols*tmpImage.rows) << " %" << endl;
						int method = 3;
						switch (method)
						{
						case 1:
							if (p4psolver.Solve(PNPSolver::METHOD::CV_P3P) == 0)
								cout << "test2:CV_P3P方法: 相机位姿→" << endl << "Oc坐标=" << p4psolver.Position_OcInW << endl << "相机旋转=" << p4psolver.Theta_W2C << endl;
							break;
						case 2:
							if (p4psolver.Solve(PNPSolver::METHOD::CV_EPNP) == 0)
								cout << "test2:CV_EPNP方法: 相机位姿→" << endl << "Oc坐标=" << p4psolver.Position_OcInW << endl << "相机旋转=" << p4psolver.Theta_W2C << endl;
							break;
						case 3:
							if (p4psolver.Solve(PNPSolver::METHOD::CV_ITERATIVE) == 0)
								cout << "test2:CV_ITERATIVE方法: 相机位姿→" << endl << "Oc坐标=" << p4psolver.Position_OcInW << endl << "相机旋转=" << p4psolver.Theta_W2C << endl;
							break;
						}
						double relativeX = p4psolver.Position_OcInW.x - p4psolver.Points3D[0].x;
						double relativeY = p4psolver.Position_OcInW.y - p4psolver.Points3D[0].y;
						double relativeZ = p4psolver.Position_OcInW.z - p4psolver.Points3D[0].z;
						cout << "相机二维方式估算距离 " << sqrt(relativeX*relativeX + relativeY*relativeY + relativeZ*relativeZ) << " 毫米" << endl;
						//LabView用写文件接口
						/*ofstream fout1("D:\\pnp_theta.txt");
						fout1 << p4psolver.Theta_W2C.x << endl << p4psolver.Theta_W2C.y << endl << p4psolver.Theta_W2C.z << endl;
						fout1.close();
						ofstream fout2("D:\\pnp_t.txt");
						fout2 << p4psolver.Position_OcInW.x << endl << p4psolver.Position_OcInW.y << endl << p4psolver.Position_OcInW.z << endl;
						fout2.close();*/
						//将坐标和角度放入坐标和角度姿态中
						coord_a.push_back(p4psolver.Position_OcInW);
						pos_a.push_back(p4psolver.Theta_W2C);
						//计算出相机在地图中的绝对位姿，单位为毫米、°
						//将每个二维码的绝对位姿加入到abs_pos_a、abs_coord_a中
						calcAbsAngleCoord(pos_a, coord_a, p4psolver.Points3D, abs_pos_a, abs_coord_a);
						for (int i = 0; i < abs_pos_a.size(); i++)
						{
							all_pos.x += abs_pos_a[i].x;
							all_pos.y += abs_pos_a[i].y;
							all_pos.z += abs_pos_a[i].z;
						}
					}
					counter++;
					//cout << "坐标：" << zbar_symbol_get_loc_x(symbol,index) << 
				}
				if (showSuccessRate)//计算二维码成功率
				{
					if (isSuccess)
					{
						sucRate[framesum % detectFrameSum] = 1;
						isSuccess = false;
					}
					else
					{
						sucRate[framesum % detectFrameSum] = 0;
					}
					framesum++;
					int sum = 0;
					if (framesum >= 30)
					{
						for (int i = 0; i < detectFrameSum; i++)
							sum += sucRate[i];
						cout << "成功率：" << (double)sum / 30.f *100.f << " %" << endl;
					}
				}
				//imshow("Dst Image", dst);
				int press = waitKey(20);
				if (press == 51) //按3键减少曝光
				{
					exposure--;
					captureLeft.set(CV_CAP_PROP_EXPOSURE, exposure);
				}
				else if (press == 54) //按6键增加曝光
				{
					exposure++;
					captureLeft.set(CV_CAP_PROP_EXPOSURE, exposure);
				}
				else if (press == 27)                        //esc退出
				{
					destroyAllWindows();
					return 0;
				}
				//清除二维码信息
				imageZbar.set_data(NULL, 0);
				//一个相机所有二维码可以算出来一个平均坐标
				Point3f finalPos_a, finalCoord_a;
				//计算单个相机的平均坐标
				calcAvgPos(abs_pos_a, abs_coord_a, finalPos_a, finalCoord_a);
				Point3f finalRightPos_a, finalRightCoord_a;
				//根据R、T计算右目相机姿态
				calcRightPos(finalPos_a, finalCoord_a, RVect[i], TVect[i], finalRightPos_a, finalRightCoord_a);
				//计算相机系统光轴坐标
				Point3f finalMidCoord(finalCoord_a.x / 2 + finalRightCoord_a.x / 2 - 1, finalCoord_a.y / 2 + finalRightCoord_a.y / 2 - 1, finalCoord_a.z / 2 + finalRightCoord_a.z / 2 - 1);
				Point3f finalMidPos(finalPos_a.x / 2 + finalRightPos_a.x / 2, finalPos_a.y / 2 + finalRightPos_a.y / 2, finalPos_a.z / 2 + finalRightPos_a.z / 2);
				//输入毫米坐标，将坐标绘制在地图上
				//文字的错位，防止重复
				Point Shift1(-30, -30);
				Point Shift2(30, 30);
				//在地图中画相机
				try
				{
					//画相机系统轴线
					drawCamOnMap(finalMidPos, finalMidCoord, Point(0, 0), Scalar(255, 255, 255), Scalar(0, 130, 255), Scalar(255, 255, 255), tmpMapImage);
					//画左相机
					drawCamOnMap(finalPos_a, finalCoord_a, Shift1, Scalar(255, 255, 0), Scalar(255, 255, 255), Scalar(0, 0, 255), tmpMapImage);
					//画右相机
					drawCamOnMap(finalRightPos_a, finalRightCoord_a, Shift2, Scalar(255, 0, 255), Scalar(255, 255, 255), Scalar(255, 0, 0), tmpMapImage);
				}
				catch (...)
				{
					cout << "draw wrong" << endl;
				}
				//显示相机画面
				resize(tmpImage, tmpImage, Size(SCREEN_WIDTH / p, SCREEN_HEIGHT / p), 0, 0);
				imshow(to_string(windowNum), tmpImage);
				moveWindow(to_string(windowNum), (windowNum % p)*tmpImage.cols, (windowNum / p)*tmpImage.rows);
				windowNum++;
			}
		}


		//采集最后的相机
		sucVCs[sucCamNum - 1] >> sucImages[sucCamNum - 1];

		vector<vector<Mat>> b;					//校正过的图片队列 [相机序号][图片数]
		vector<vector<Mat>> centroid;					//二维码形心坐标队列 [相机序号][每个相机图片中的坐标]Mat(X,Y)
		vector<Point3f> coord_b;					//校正过图片中距离原点的坐标队列 [相机序号][每个相机图片中的坐标]Mat(X,Y,Z)
		vector<Mat> disp_b;					//视差图队列

		//每两个相机识别位姿  三维识别位姿
		for (int i = 0; i < sucCamNum - 1; i++)	//每一个相机读取图像  三维识别位姿
		{

			if (SHOW_3D)
			{
				int d = 0;
				bool isQRSuc;
				vector<Point> doubleCenter;	//双目二维码形心
				//识别二维码，输出形心视差d、两个形心doubleCenter、深度Z
				int k = waitKey(1);
				if (k == 27)
				{
					destroyAllWindows();
					return 0;
				}
				String QRText;
				vector<Point3f> Point3D;
				if (IS_REMAP)
				{
					picLeftROI[i] = sucImages[i];					//初始化picLeftROI队列
					picRightROI[i] = sucImages[i + 1];
					vector<Mat> recImages;
					recImages.push_back(sucImages[i]);
					recImages.push_back(sucImages[i + 1]);

					recImagesVect[i] = recImages;			//初始化recImagesVect队列
					remap(picLeftROI[i], recImagesVect[i][0], rmapFinalVect[i][0][0], rmapFinalVect[i][0][1], CV_INTER_LINEAR);	//校正
					remap(picRightROI[i], recImagesVect[i][1], rmapFinalVect[i][1][0], rmapFinalVect[i][1][1], CV_INTER_LINEAR);	//校正	
					if (IS_ROI)
					{
						picLeftROI[i] = recImagesVect[i][0](RoiVect[i]);	//以左图的ROI为基准，取ROI
						picRightROI[i] = recImagesVect[i][1](RoiVect[i]);
					}
					else
					{
						recImagesVect[i][0].copyTo(picLeftROI[i]);
						recImagesVect[i][1].copyTo(picRightROI[i]);
					}
					vector<Point2f> leftPoints2D, rightPoints2D;
					isQRSuc = cadlculateQR(picLeftROI[i], picRightROI[i], d, leftPoints2D, rightPoints2D, doubleCenter, QRText, QVect[i]);

					if (isQRSuc)
					{
						Point3f point;
						//cout << "d " << d << " px" << endl;
						//cout << "P" << p << endl;
						//cout << "doubleCenter[0]" << doubleCenter[0] << endl << "doubleCenter[1]" << doubleCenter[1] << endl;
						//直接输入一个二维坐标和对应的视差值，并通过重投影矩阵生成世界坐标中的三维点
						if (ReadPosFromQR)
						{//从二维码中读取位姿
							//将条码内容放进队列
							String pos_str = QRText;
							//从文本中读取点
							readPointsFromText(pos_str, Point3D, QRLen, QRheight, wallZ);
							//记录当前消息的相机序号
							sucSerial.push_back(i);
						}
						else
						{//默认左上角为（0 0 0）
							Point3D.push_back(cv::Point3f(0, 0, 0));     //P1三维坐标的单位是毫米
							Point3D.push_back(cv::Point3f(0, QRLen, 0));   //P2
							Point3D.push_back(cv::Point3f(QRLen, QRLen, 0));  //P4
							Point3D.push_back(cv::Point3f(QRLen, 0, 0));   //P3
						}
						float leftX = (Point3D[0].x + Point3D[1].x + Point3D[2].x + Point3D[3].x) / 4;
						float leftY = (Point3D[0].y + Point3D[1].y + Point3D[2].y + Point3D[3].y) / 4;
						float leftZ = (Point3D[0].z + Point3D[1].z + Point3D[2].z + Point3D[3].z) / 4;
						Point3f leftCenter(leftX, leftY, leftZ);
						point_to_xyz(doubleCenter[0].x, doubleCenter[0].y, d, QVect[i], point);
						//cout << "point_c" << point << endl;
						//cout << "Point3D[0]" << Point3D[0] << endl;
						if (Point3D[3].x < Point3D[0].x)
						{
							point.x = leftCenter.x + point.x;
							point.y = leftCenter.y + point.y;
							point.z = leftCenter.z + point.z;
						}
						else
						{
							point.x = leftCenter.x - point.x;
							point.y = leftCenter.y + point.y;
							point.z = leftCenter.z - point.z;
						}
						if (point.z <= 2000 && point.z >= 300)
							coord_b.push_back(point);
						//cout << "point_w" << point << endl;
						stringstream ss;
						ss << "Z dist: " << point.z;
						//ss << "(" << point.x << "," << point.y << "," << point.z << ")";
						//画圆 cvPoint:确定圆的坐标  200：圆的半径 CV_RGB：圆的颜色 3：线圈的粗细  
						circle(picLeftROI[i], doubleCenter[0], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(picLeftROI[i], leftPoints2D[0], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(picLeftROI[i], leftPoints2D[1], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(picLeftROI[i], leftPoints2D[2], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(picLeftROI[i], leftPoints2D[3], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(picRightROI[i], doubleCenter[1], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(picRightROI[i], rightPoints2D[0], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(picRightROI[i], rightPoints2D[1], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(picRightROI[i], rightPoints2D[2], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(picRightROI[i], rightPoints2D[3], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						//标出三维坐标
						putText(picLeftROI[i], ss.str(), doubleCenter[0], FONT_HERSHEY_DUPLEX, 0.6, CV_RGB(255, 0, 0), 1, 8, false); //P3 text
						cout << "相机三维方式估算距离 " << sqrt(point.x*point.x + point.y*point.y + point.z*point.z) << " 毫米" << endl;
						//cout << "相机三维方式估算Z距离 " << point.z << " 毫米" << endl;
						resize(picLeftROI[i], picLeftROI[i], Size(SCREEN_WIDTH / p, SCREEN_HEIGHT / p), 0, 0);
						imshow(to_string(windowNum), picLeftROI[i]);
						moveWindow(to_string(windowNum), (windowNum % p)*picLeftROI[0].cols, (windowNum / p)*picLeftROI[0].rows);
						windowNum++;
						putText(picRightROI[i], ss.str(), doubleCenter[1], FONT_HERSHEY_DUPLEX, 0.6, CV_RGB(255, 0, 0), 1, 8, false); //P3 text				
						resize(picRightROI[i], picRightROI[i], Size(SCREEN_WIDTH / p, SCREEN_HEIGHT / p), 0, 0);
						imshow(to_string(windowNum), picRightROI[i]);
						moveWindow(to_string(windowNum), (windowNum % p)*picLeftROI[0].cols, (windowNum / p)*picLeftROI[0].rows);
						windowNum++;
						cout << "cadlculateQR Over" << endl;
					}
					else
					{
						resize(picLeftROI[i], picLeftROI[i], Size(SCREEN_WIDTH / p, SCREEN_HEIGHT / p), 0, 0);
						imshow(to_string(windowNum), picLeftROI[i]);
						moveWindow(to_string(windowNum), (windowNum % p)*picLeftROI[0].cols, (windowNum / p)*picLeftROI[0].rows);
						windowNum++;
						resize(picRightROI[i], picRightROI[i], Size(SCREEN_WIDTH / p, SCREEN_HEIGHT / p), 0, 0);
						imshow(to_string(windowNum), picRightROI[i]);
						moveWindow(to_string(windowNum), (windowNum % p)*picLeftROI[0].cols, (windowNum / p)*picLeftROI[0].rows);
						windowNum++;
						cout << "cadlculateQR Fail" << endl;
					}
				}
				else
				{
					vector<Point2f> leftPoints2D, rightPoints2D;
					isQRSuc = cadlculateQR(sucImages[i], sucImages[i + 1], d, leftPoints2D, rightPoints2D, doubleCenter, QRText, QVect[i]);

					system("CLS");
					if (isQRSuc)
					{
						Point3f point;
						cout << "d " << d << " px" << endl;
						cout << "P" << p << endl;
						//直接输入一个二维坐标和对应的视差值，并通过重投影矩阵生成世界坐标中的三维点
						point_to_xyz(doubleCenter[0].x, doubleCenter[0].y, d, QVect[i], point);
						cout << "point" << point << endl;
						stringstream ss;
						//ss << "(" << point.x << "," << point.y << "," << point.z << ")";
						ss << "Z距离："  << point.z ;
						//画圆 cvPoint:确定圆的坐标  200：圆的半径 CV_RGB：圆的颜色 3：线圈的粗细  
						circle(sucImages[i], doubleCenter[0], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(sucImages[i], leftPoints2D[0], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(sucImages[i], leftPoints2D[1], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(sucImages[i], leftPoints2D[2], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(sucImages[i], leftPoints2D[3], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(sucImages[i + 1], doubleCenter[1], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(sucImages[i + 1], rightPoints2D[0], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(sucImages[i + 1], rightPoints2D[1], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(sucImages[i + 1], rightPoints2D[2], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						circle(sucImages[i + 1], rightPoints2D[3], 3, CV_RGB(255, 255, 0), 3, 8, 0);
						//标出三维坐标
						putText(sucImages[i], ss.str(), doubleCenter[0], FONT_HERSHEY_DUPLEX, 0.6, CV_RGB(255, 0, 0), 1, 12, false); //P3 text
						cout << "相机三维方式估算Z距离 " << point.z << " 毫米" << endl;
						cout << "相机三维方式估算距离 " << sqrt(point.x*point.x + point.y*point.y + point.z*point.z) << " 毫米" << endl;
						resize(sucImages[i], sucImages[i], Size(SCREEN_WIDTH / p, SCREEN_HEIGHT / p), 0, 0);
						imshow(to_string(windowNum), sucImages[i]);
						moveWindow(to_string(windowNum), (windowNum % p)*sucImages[0].cols, (windowNum / p)*sucImages[0].rows);
						windowNum++;
						putText(sucImages[i + 1], ss.str(), doubleCenter[1], FONT_HERSHEY_DUPLEX, 0.6, CV_RGB(255, 0, 0), 1, 8, false); //P3 text				
						resize(sucImages[i + 1], sucImages[i + 1], Size(SCREEN_WIDTH / p, SCREEN_HEIGHT / p), 0, 0);
						imshow(to_string(windowNum), sucImages[i + 1]);
						moveWindow(to_string(windowNum), (windowNum % p)*sucImages[0].cols, (windowNum / p)*sucImages[0].rows);
						windowNum++;
						cout << "cadlculateQR Over" << endl;
					}
					else
					{
						resize(sucImages[i], sucImages[i], Size(SCREEN_WIDTH / p, SCREEN_HEIGHT / p), 0, 0);
						imshow(to_string(windowNum), sucImages[i]);
						moveWindow(to_string(windowNum), (windowNum % p)*sucImages[0].cols, (windowNum / p)*sucImages[0].rows);
						windowNum++;
						resize(sucImages[i + 1], sucImages[i + 1], Size(SCREEN_WIDTH / p, SCREEN_HEIGHT / p), 0, 0);
						imshow(to_string(windowNum), sucImages[i + 1]);
						moveWindow(to_string(windowNum), (windowNum % p)*sucImages[0].cols, (windowNum / p)*sucImages[0].rows);
						windowNum++;
						cout << "cadlculateQR Fail" << endl;
					}
				}
			}
		}
		for (int i = 0; i < coord_b.size(); i++)
		{
			all_pos.x += coord_b[i].x;
			all_pos.y += coord_b[i].y;
			all_pos.z += coord_b[i].z;
			counter++;
		}		
		//显示地图
		if (SHOW_2D)
		{
			resize(tmpImage, tmpImage, Size(SCREEN_WIDTH / p, SCREEN_HEIGHT / p), 0, 0);
			imshow("Map", tmpMapImage);
			moveWindow("Map", (windowNum % p)*tmpImage.cols, (windowNum / p)*tmpImage.rows);
			windowNum++;
		}
		//MoveWindow(hwnd, ((windowNum % p) * SCREEN_WIDTH / p, ((windowNum * 2) / p)* SCREEN_HEIGHT / p, SCREEN_WIDTH / p, SCREEN_HEIGHT / p, TRUE);
	}
	return 0;
}