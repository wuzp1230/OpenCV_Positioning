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

using namespace zbar;  //���zbar���ƿռ�    
using namespace cv;

#define IMAGE_WIDTH 1280
#define IMAGE_HEIGHT 720											// �궨��ĳ����Ϊ7:5	
#define CLEFT 0
#define Pi 3.1415926

#define pxLen 5.2*1e-3//����
#define focLen 508 * pxLen
#define QRLen 200//��ά�볤�Ⱥ���
#define QRheight 1500//��ά�붥���������룬����
#define wallZ 5810	//ǽ��Z����

#define ROI_HEIGHT IMAGE_HEIGHT
#define ROI_WIDTH (ROI_HEIGHT / 3 * 4)													// �궨��ĳ����Ϊ7:5	

#define CAP_WIDTH 1280
#define CAP_HEIGHT 720
#define ReadPosFromQR 1//�Ƿ�Ӷ�ά���ж�ȡ����
#define SHOW_2D 1
#define SHOW_3D 1
#define IS_REMAP 1
#define IS_ROI 1
double SCREEN_WIDTH = 1600;
double SCREEN_HEIGHT = 900;

int CALIB = 0;				//�Ƿ��ȡ�궨��Ϣ
int CONTRAST_ENHANCE = 1;	//�Ƿ���ǿ�Աȶ�
int CENTER_LINE = 0;		//�Ƿ����Ļ�׼��
const int startID = 0;		//��һ������ı�ʶ��

/*�������������
cnum�������������
sucCamNum��������ɹ����������
MaxDetectNum������������豸��
sucVCs������ɹ��������׽�����У���Ҫ�ں������������һ��Open������
sucCamSeq������ɹ��ĳ�ʼ���˳�����
*/
void detectCamnum(int cnum, int &sucCamNum, int MaxDetectNum, vector<VideoCapture> &sucVCs, vector<int> &sucCamSeq)
{
	vector<Mat> images;												//���ڳ�ʼ������ʱ�ɼ�ͼƬ����
	vector<VideoCapture> VCs;										//���ڳ�ʼ���������׽������
	for (int i = 0; sucCamNum < cnum && i < MaxDetectNum; i++){		//�����������������Ƿ����
		cout << "���ڼ���" << i << "����Ƶ��׽����" << endl;
		VCs.push_back(VideoCapture(startID + i));					//��ʼ���з��벶׽��
		VCs[i].open(startID + i);
		VCs[i].set(CV_CAP_PROP_FRAME_WIDTH, CAP_WIDTH);
		VCs[i].set(CV_CAP_PROP_FRAME_HEIGHT, CAP_HEIGHT);
		if (CONTRAST_ENHANCE)
		{
			VCs[i].set(CV_CAP_PROP_CONTRAST, 200);					//��ǿ�Աȶ�
		}
		//images.push_back(Mat(CAP_HEIGHT, CAP_WIDTH, CV_8UC3));
		images.push_back(Mat());									//��ʼ���ɼ�ͼƬ����
		VCs[i] >> images[i];										//�ȼ��һ��ͼ���Ƿ���ͼƬ����	
		if (!images[i].data){										//������
			cout << "��" << i << "����Ƶ��׽������ȡ��ͼ��" << endl;
		}
		else{														//������
			sucVCs.push_back(VideoCapture(startID + i));			//����ɹ�������
			sucCamSeq.push_back(sucCamNum);							//��ʼ�������Ŷ���
			sucCamNum++;											//�ɹ�������+1
			cout << "�ѳɹ�����" << sucCamNum << "����Ƶ��׽��������У�" << endl;
		}
	}
	if (sucCamNum == 0)	//δ��⵽���˳�
	{
		cout << "δ��⵽��Ƶ��׽��,����ȡ��ͼ��" << endl;
		system("PAUSE");
		return;
	}
}
/*�ֶ��������˳��
sucCamNum��Ҫ�������������
sucVCs�����롢����������
sucCamSeq�����롢��������Ŷ���
ԭ��ʹ�����ǰ���ڵĴ��ںź�Ӧ���ڵĴ��ںŶԵ�
*/
void changeCamSeq(int sucCamNum, vector<VideoCapture> &sucVCs, vector<int> &sucCamSeq)
{
	bool showSwapInfo = true;
	int swappingCamNum = 0;	//Ҫ�Ե���������豸�ţ���
	int swappingWinNum = 0;	//Ҫ�Ե��������Ŀ�괰�ںţ�С��
	bool isConfirmed = false;			//�ٴ�ȷ�ϵĿ���
	double scaleView = 0.7;
	int press;
	stringstream camNum;	//���������������
	VideoCapture tempVC;
	vector<Mat> sucImages;		//�������˳��ʱ�õ���ͼƬ����
	for (int i = 0; i < sucCamNum; i++)			//Ϊ�������˳����׼��
	{
		sucImages.push_back(Mat());
	}
	while (1)
	{
		if (showSwapInfo)
		{
			cout << "1.���������λ�ã����ֻ�Ծ������������洰���ϣ������������д��ڿɼ�" << endl;
			cout << "2.����� " << swappingWinNum << " �������ǰ���ڵĴ��ڱ�ţ����س���ȷ����" << endl;
			showSwapInfo = false;
		}
		for (int i = 0; i < sucCamNum; i++)	//��ȡͼ��
		{
			sucVCs[i] >> sucImages[i];
			//cout << "�ɹ���ȡ��" << i << "��ͼ��" << endl;
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
		else if (press == 13)			//�س�13	�ո�32  
		{
			camNum >> swappingCamNum;
			camNum.clear();
			if (!isConfirmed)				//δȷ��
			{
				cout << endl;
				if (swappingCamNum >= sucCamNum)	//����ų��磬���Ϸ�
				{
					cout << "����ų��磬����������" << endl;
					camNum.str("");
					isConfirmed = false;
				}
				else
				{
					cout << "ȷ�Ͻ��� " << swappingCamNum << " ������Ļ���͵�" << swappingWinNum << "�����ڵĻ��潻����" << endl;
					cout << "�ٴΰ��س���ȷ�ϣ���Esc��ȡ��" << endl;
					isConfirmed = true;
				}
			}
			else //isConfirmed
			{
				//�Ե�
				tempVC = sucVCs[swappingCamNum];
				sucVCs[swappingCamNum] = sucVCs[swappingWinNum];
				sucVCs[swappingWinNum] = tempVC;
				//��ŶԵ�
				int tempSeq = sucCamSeq[swappingCamNum];
				sucCamSeq[swappingCamNum] = sucCamSeq[swappingWinNum];
				sucCamSeq[swappingWinNum] = tempSeq;

				swappingWinNum++;
				isConfirmed = false;
				showSwapInfo = true;	//�Ե���ϣ���ʾ��һ���������Ϣ
				camNum.str("");
				if (swappingWinNum == sucCamNum - 1)	//ȫ���Ե���ϣ���ʼ�����ļ�
				{
					char filename[] = "camera_seq.xml";
					FileStorage fs(filename, FileStorage::WRITE);
					if (!fs.isOpened())
					{
						cout << "��ʧ�ܣ�" << endl;
					}
					else
					{
						fs << "seq" << sucCamSeq;
					}
					cout << "���˳���ѱ�����" << filename << endl;
					fs.release();

					break;//�Ե�������ϣ��˳�
				}
			}

		}
		else if (press == 27)		//Esc
		{
			if (!isConfirmed)
			{		//�˳�
				cout << "���˳�����" << endl;
				break;
				//isConfirmed = true;
			}
			else
			{
				cout << "��ȡ���˴ε���" << endl;
				isConfirmed = false;//ȡ��ȷ��״̬
			}
		}
	}
}
/*�������ļ��ж�ȡ������
filename���ļ���
sucCamNum���������
sucVCs���������
sucCamSeq�������Ŷ���
ԭ����ȡ�����ţ�ͨ����Ž�����������ʵ���˳��
*/
void readCamSeq(char *filename, int sucCamNum, vector<VideoCapture> &sucVCs, vector<int> &sucCamSeq)
{
	FileStorage fs(filename, FileStorage::READ);
	if (!fs.isOpened())
	{
		cout << "��ʧ�ܣ�" << endl;
	}
	else
	{
		fs["seq"] >> sucCamSeq;
	}
	cout << "���˳���Ѷ�ȡ" << filename << endl;
	fs.release();
	Vector<VideoCapture> tmpsucCamSeq;
	for (int i = 0; i < sucCamNum; i++)	//��ʼ��temp����
	{
		tmpsucCamSeq.push_back(sucVCs[i]);
	}
	for (int i = 0; i < sucCamNum; i++)
	{
		sucVCs[i] = tmpsucCamSeq[sucCamSeq[i]];
	}
	cout << "���˳���ѵ���" << filename << endl;
	//�Ե�������ϣ��˳�
}
/*
����threshold��ֵѰ��srcͼ���е����б�Ե������Matͼ��
*/
Mat findMoreContours(Mat src, int threshold)
{
	Mat dst = Mat::zeros(src.rows, src.cols, CV_8UC3);
	Mat imageGrayMoreThres = src > threshold;
	cvtColor(imageGrayMoreThres, imageGrayMoreThres, CV_RGB2GRAY);
	vector<vector<Point>> contours;
	vector<Vec4i>hierarchy; //hierarchy[ i ][ 0 ] ~hierarchy[ i ][ 3 ]���ֱ��ʾ��һ��������ǰһ������������������Ƕ�������������
	findContours(imageGrayMoreThres, contours, hierarchy, CV_RETR_CCOMP, CV_RETR_TREE);
	if (!contours.empty() && !hierarchy.empty())
	{
		int idx = 0;
		for (; idx >= 0; idx = hierarchy[idx][0])
		{
			Scalar color(255, 255, 255);
			drawContours(dst, contours, idx, color, 1, 8, hierarchy);	//��idx������
		}
	}
	waitKey(1);
	return dst;
}
/*
����P1��P2����ľ��룬����doubleֵ
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
����Z����ͼ���еĵ�
�����ά���λ�˺Ͷ�ά���ĸ���  ���Z��Ӧ�ڵ�λ��
*/
void calcZImagePoint(Point3f angles, Point3f coords, Vector<Point> points, Point &Z)
{
	double alpha = -(angles.x + 5)*CV_PI / 180.f;//x�ḩ��  ��������
	alpha = alpha > 0 ? alpha - CV_PI : alpha + CV_PI;
	//alpha = -alpha;
	double beta = -(angles.y - 10)*CV_PI / 180.f;//y������ ���Ϊ��
	double gamma = angles.z*CV_PI / 180.f;//z����������  ��ʱ��Ϊ��
	gamma = gamma > 0 ? gamma - CV_PI : gamma + CV_PI;
	//gamma = -gamma;
	//cout << alpha << endl << beta << endl << gamma << endl;

	double L = coords.z;
	double x0 = points[3].x, y0 = points[3].y, z0 = abs(QRLen * 5 * focLen / L / pxLen);
	double x2, y2, z2;
	x2 = z0*cos(alpha)*sin(beta); y2 = -z0*sin(alpha);
	//x2���Ϊ�� y2����  P��x��������ʼ��ʱ��ת��
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
������ԽǶ�
�������λ�� ���������� ��ͼ ��ά���Ľ��������� �����ά��������� ���ԽǶȶ���
*/
void calcAbsAngleCoord(vector<Point3f> pos_a, vector<Point3f> coord_a, vector<Point3f> points, vector<Point3f> &abs_pos_a, vector<Point3f> &abs_coord_a)
{
	Point3f tmp_pos, tmp_coord;//��ʱ�����̬������
	if (points[3].x < points[0].x)
	{//0ǽ������[3]>[0]
		//cout << "0ǽ������" << endl;
		for (int i = 0; i < pos_a.size(); i++)
		{
			abs_coord_a.push_back(coord_a[i]);
			abs_pos_a.push_back(pos_a[i]);
		}
	}
	else
	{//581ǽ������[3]<[0]
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
�������λ��ƽ��ֵ
�����ͼ�����о���λ�˶���  ���һ��ƽ����λ�˵�
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
ͨ�������תƽ�ƾ��������һ���������תƽ��
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
�ڵ�ͼ�л��������
�������������������  �Ժ���Ϊ��λ �����ɫBGR ��̬����ɫ ����������ƹ������ͼ
*/
void drawCamOnMap(Point3f finalPos_a, Point3f finalCoord_a, Point textShift, Scalar scCam, Scalar scPos, Scalar scText, Mat &mapImage)
{//2D�����Խ��߳�19pxBGR(255,255,0)  �����̬�᳤63px(0,130,255)	3D�����Խ��߳�19px(255,0,255)
	Point2f camCoordPoint;		//��¼����������ڵ�
	Point2f camPosPoint;		//�����̬������
	camCoordPoint.x = finalCoord_a.x / 10;
	camCoordPoint.y = finalCoord_a.z / 10;
	vector<Point> camBorder4Points;
	//������������ĸ���
	double finalPosy = finalPos_a.y + 180;
	double camLen = 5;
	camBorder4Points.push_back(Point(camCoordPoint.x + camLen * cos((finalPosy + 45) / 180 * CV_PI), camCoordPoint.y + camLen * sin((finalPosy + 45) / 180 * CV_PI)));
	camBorder4Points.push_back(Point(camCoordPoint.x + camLen * cos((finalPosy + 135) / 180 * CV_PI), camCoordPoint.y + camLen * sin((finalPosy + 135) / 180 * CV_PI)));
	camBorder4Points.push_back(Point(camCoordPoint.x + camLen * cos((finalPosy + 225) / 180 * CV_PI), camCoordPoint.y + camLen * sin((finalPosy + 225) / 180 * CV_PI)));
	camBorder4Points.push_back(Point(camCoordPoint.x + camLen * cos((finalPosy + 315) / 180 * CV_PI), camCoordPoint.y + camLen * sin((finalPosy + 315) / 180 * CV_PI)));
	//cout << "camBorder4Points" <<camBorder4Points << endl;
	//������̬������
	camPosPoint.x = camCoordPoint.x + 63 * cos((finalPosy + 90) / 180 * CV_PI);
	camPosPoint.y = camCoordPoint.y + 63 * sin((finalPosy + 90) / 180 * CV_PI);

	if (scPos != Scalar(255, 255, 255))
	{
		//�������̬��
		line(mapImage, camCoordPoint, camPosPoint, scPos, 2, 8, 0);
	}
	if (scCam != Scalar(255, 255, 255))
	{	//�����������
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
ʶ��QR��ά�룬���ض����ά���ĽǵĶ�ά����
����ͼƬpic, ��ͼrightPic, �����ά���ı�text, �����Ŀ��ά���ĸ���Points2D
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
	scanner.scan(imageZbar); //ɨ������      
	Image::SymbolIterator symbol = imageZbar.symbol_begin();
	if (imageZbar.symbol_begin() == imageZbar.symbol_end())
	{
		cout << "��ѯ����ʧ�ܣ�����ͼƬ��" << endl;
		return false;
	}
	for (; symbol != imageZbar.symbol_end(); ++symbol)
	{
		if (symbol->get_location_size() == 4)
		{//��ά��ֱ��2.4cm 0 - 1 - 2- 3  ����  ����  ���� ����
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
�������Ҷ�ά��ͼ��Q������ȣ������Ӳ�d��Z��������
������ͼleftPic, ��ͼrightPic, ����Ӳ�d, �����Ŀ��ά���ĸ���leftPoints2D, �����Ŀ��ά���ĸ���rightPoints2D, ���˫Ŀ����doubleCenter, �����ά���ı�QRText, ����˫ĿQ����leftQ
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
		//d:�Ӳ�ֵ
		//doffs���Ӳ�ƫ��ֵ
		//baseline�����߾���
		//f������
		//disSc���Ӳ���������, Ĭ��Ϊ1 ��Բ�ͬ�Ӳ��㷨���ɵ��Ӳ�ͼ�����ڱ�׼256λ�����ϳ��ϻ������������
		//P����άͼ���ͶӰ����
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
ֱ������һ����ά����Ͷ�Ӧ���Ӳ�ֵ����ͨ����ͶӰ����������ά��
*/
void point_to_xyz(float x, float y, float d, Mat Q, Point3f& point)  //ֱ������һ����ά����Ͷ�Ӧ���Ӳ�ֵ����ͨ����ͶӰ����������ά��
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

	int MaxDetectNum = 15;	//������������
	int sucCamNum = 0;	//�ɹ���ȡ�����������
	vector<VideoCapture> sucVCs;	//�ɹ���ȡ�������׽������
	vector<int> sucCamSeq;			//�ɹ���ȡ��������
	int cnum = 2;
	/**********				�������������			*****************/
	detectCamnum(cnum, sucCamNum, MaxDetectNum, sucVCs, sucCamSeq);
	cout << "�ɹ�����������е���������� " << sucCamNum << endl;
	cout << "����������й�ģ��" << sucVCs.size() << endl;
	vector<Mat> image;			//��ʱ����
	vector<Mat> sucImages;		//����ʱ�õ���ͼƬ����
	for (int i = 0; i < sucCamNum; i++)			//Ϊ�������˳����׼��
	{
		sucImages.push_back(Mat());
		image.push_back(Mat());
	}
	for (int i = 0; i < sucCamNum; i++){		//�����������������Ƿ����
		sucVCs[i].open(startID + i);
		sucVCs[i].set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));
		sucVCs[i].set(CV_CAP_PROP_FRAME_WIDTH, CAP_WIDTH);
		sucVCs[i].set(CV_CAP_PROP_FRAME_HEIGHT, CAP_HEIGHT);
		sucVCs[i] >> image[i];	//�ȼ��һ��ͼ��	
		if (!image[i].data){
			cout << "��" << i << "����Ƶ��׽������ȡ��ͼ��" << endl;
		}
		else{
			cout << "��" << i << "����Ƶ��׽�����ɹ���" << endl;
		}
	}

	if (sucCamNum > 1)
	{
		char seqChoice;
		cout << "				��ѡ�����˳��ѡ��" << endl;
		cout << "1.����˳�򲢱���	2.�������ļ��ж�ȡ˳��	3.����ȡҲ������" << endl;
		do{
			cin >> seqChoice;
		} while (seqChoice <'1' || seqChoice > '3');
		/**	1.����˳�򲢱���  2.�������ļ��ж�ȡ˳��   3.����ȡҲ������	****/
		if (seqChoice == '1')		//1.����˳�򲢱���
		{
			/****************		�ı����˳��	************/
			changeCamSeq(sucCamNum, sucVCs, sucCamSeq);
		}
		else if (seqChoice == '2')		//	2.�������ļ��ж�ȡ˳��	
		{
			char filename[] = "camera_seq.xml";
			readCamSeq(filename, sucCamNum, sucVCs, sucCamSeq);
		}
		else if (seqChoice == '3')		//	3.����ȡҲ������
		{
		}
		destroyAllWindows();
	}

	int exposure = -7;//�ع�
	Mat mapImage = imread("Map.png", 1);
	Mat tmp_image, tmp_map_image;
	ImageScanner scanner;
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
	sucVCs[0] >> tmp_image;				// ������ȡ��ǰ֡
	//cvtColor(tmp_image, imageGray, CV_RGB2GRAY);
	int width = tmp_image.cols;
	int height = tmp_image.rows;
	Mat imgContours;
	Point O;//��ά�����ĵ���������
	long int framesum = 0;
	bool isSuccess = false;
	bool showSuccessRate = false;//�Ƿ���ʾ�ɹ���
	int p = 0;
	if (SHOW_2D)
	{
		p += sucCamNum + 1;	//+1ΪMap
	}
	if (SHOW_3D)
	{
		p += sucCamNum - 1;	//-1Ϊ��Ŀ
	}
	p = ceil(sqrt(p + 1));	//�����ƽ��ȡ���+2Ϊ��Ԥ��������
	const int detectFrameSum = 30;//���ɹ���ʱ�ļ�ⷶΧ
	int sucRate[detectFrameSum];

	/****************	��ȡ�궨��Ϣ	************/
	if (sucCamNum > 1)
	{
		cout << "�Ƿ��ȡ�궨��Ϣ" << endl;
		cout << "1 : ��ȡ  2 : ����ȡ" << endl;
		int choiceCalib;
		do
		{
			cin >> choiceCalib;
		} while (choiceCalib != 1 && choiceCalib != 2);
		if (choiceCalib == 1)
		{
			CALIB = 1;	//�Ƿ��ȡ�궨��Ϣ
		}
		else CALIB = 0;
	}

	Vector<Vector<Mat>> recImagesVect;		//У����ͼ�������	[N][2]
	vector<vector<vector<Mat>>> rmapFinalVect;			//���ӳ�����ն���rmapFinalVect[����������][����][XY]
	Vector<Vector<Mat>> cameraMatrixVect, distCoeffsVect;		//�ڲΡ�����ϵ������	[N][2]
	Vector<Mat> R1Vect, R2Vect, P1Vect, P2Vect, RVect, TVect, QVect;	//�������	[N]
	Vector<Rect> RoiVect;				//���ROI����
	Vector<Mat> picLeftROI, picRightROI;			//ȡ������Ȥ�����ͼ��
	//��ȡ�궨��Ϣ
	if (CALIB)
	{
		for (int i = 0; i < sucCamNum - 1; i++)
		{
			stringstream ss;
			ss << "camera" << setw(2) << setfill('0') << i + 1 << "_results.xml"; //.\camera01_results.xml - .\cameraNN_results.xml
			String filename = ss.str();
			Rect Roi[2];						//ÿһ�����������ROI��ʱ��ŵ�λ��
			int x1, x2, x3, x4, y1, y2, y3, y4, xx = 0, yy = 0, xx2, yy2, h = CAP_HEIGHT, w = CAP_WIDTH;	//����Ȥ�������У���ȡ��ʹ��
			Mat R, T, R1, R2, P1, P2, Q;		//ÿһ����������������ʱ��ŵ�λ��
			FileStorage fs(filename, FileStorage::READ);
			Mat tmpcameraMatrix, tmpdistCoeffs;
			Vector<Mat> cameraMatrix, distCoeffs;
			if (!fs.isOpened())
			{
				cout << "�궨��Ϣ�����ڣ�" << endl;
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
				cameraMatrixVect.push_back(cameraMatrix);	//[�����][����Ŀ][�ڲξ���]
				distCoeffsVect.push_back(distCoeffs);		//[�����][����Ŀ][�������]
				R1Vect.push_back(R1); P1Vect.push_back(P1);
				R2Vect.push_back(R2); P2Vect.push_back(P2);
				x1 = Roi[0].x; x2 = Roi[1].x;//ROI�Ͻ�����
				x3 = Roi[0].x + Roi[0].width;//ROI�½�����
				x4 = Roi[1].x + Roi[1].width;
				y1 = Roi[0].y; y2 = Roi[1].y;
				y3 = Roi[0].y + Roi[0].height;
				y4 = Roi[1].y + Roi[1].height;
				xx = max(x1, x2); yy = max(y1, y2);//ȡ��ͬ����
				xx2 = min(x3, x4); yy2 = min(y3, y4);
				h = xx2 - xx; w = yy2 - yy;
				RoiVect.push_back(Rect(xx, yy, h, w));
			}
		}
		cout << "�Ѷ�ȡ�궨��Ϣ" << endl;
	}
	//����궨��Ϣ��ֻ�����ڵ�Ŀ
	else
	{
		for (int i = 0; i < sucCamNum; i++)	//��ȡ�궨��Ϣ
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
	Mat rmapX, rmapY;						//���������X��Y rmap

	if (CALIB)
	{
		for (int i = 0; i < sucCamNum - 1; i++)
		{
			picLeftROI.push_back(sucImages[i]);					//��ʼ��picLeftROI����
			picRightROI.push_back(sucImages[i + 1]);
			vector<Mat> recImages;
			recImages.push_back(sucImages[i]);
			recImages.push_back(sucImages[i + 1]);
			recImagesVect.push_back(recImages);			//��ʼ��recImagesVect����
		}
		//��ʼ��remap����
		for (int i = 0; i < sucCamNum - 1; i++){
			sucVCs[i] >> sucImages[i];
			sucVCs[i + 1] >> sucImages[i + 1];
			picLeftROI[i] = sucImages[i];					//��ʼ��picLeftROI����
			picRightROI[i] = sucImages[i + 1];
			vector<Mat> rampL, rmapR;				//һ�����������rmap
			vector<vector<Mat>> rmapVect;					//���ӳ�����
			initUndistortRectifyMap(cameraMatrixVect[i][0], distCoeffsVect[i][0], R1Vect[i], P1Vect[i], picLeftROI[i].size(), CV_16SC2, rmapX, rmapY);	//ȥ����
			rampL.push_back(rmapX); rampL.push_back(rmapY);
			rmapVect.push_back(rampL);
			initUndistortRectifyMap(cameraMatrixVect[i][1], distCoeffsVect[i][1], R2Vect[i], P2Vect[i], picLeftROI[i].size(), CV_16SC2, rmapX, rmapY);	//ȥ����
			rmapR.push_back(rmapX); rmapR.push_back(rmapY);
			rmapVect.push_back(rmapR);
			rmapFinalVect.push_back(rmapVect); //[����������][����][XY]
		}
	}
	HWND hwnd = GetForegroundWindow();
	while (1)
	{
		int counter = 0;//��ά����ά�õ���λ��
		int windowNum = 0;
		vector<String> inf;			//���ͼƬ�п��ܳ��ֶ�������������[�������������������]
		vector<Mat> a;					//δУ����ͼƬ����
		vector<int> sucSerial;			//���ɹ������
		Mat imageGray, imageGrayMore100;
		Mat tmpMapImage; mapImage.copyTo(tmpMapImage);//���ڻ�ͼ����ʱ��ͼ
		Mat tmpImage;					//���ڻ��ƶ�ά��������ʱͼ
		Point3f all_pos, all_coord;
		//ÿһ�������ȡͼ��  ��άʶ��λ��
		system("CLS");
		for (int i = 0; i < sucCamNum - 1; i++)
		{
			vector<Point3f> abs_pos_a;					//ÿ�������ά��ʽ�����õ��ĽǶȶ��� [ÿ�����ֵĵ����꣨������������]k��б�ʱ�ʾ��
			vector<Point3f> abs_coord_a;				//ÿ�������ά��ʽ�����õ���������Զ��� [ÿ�����ֵĵ����꣨������������]Mat(X,Y)
			sucVCs[i] >> sucImages[i];
			//��ʾ��ά�궨��ʽ
			if (SHOW_2D)
			{
				sucImages[i].copyTo(tmpImage);
				cvtColor(tmpImage, imageGray, CV_RGB2GRAY);
				a.push_back(imageGray);
				uchar *raw = (uchar *)imageGray.data;
				Image imageZbar(width, height, "Y800", raw, width * height);
				scanner.scan(imageZbar); //ɨ������    
				Image::SymbolIterator symbol = imageZbar.symbol_begin();
				if (imageZbar.symbol_begin() == imageZbar.symbol_end())
				{
					cout << "��ѯ����ʧ�ܣ�����ͼƬ��" << endl;
				}
				//һ����������ж�ά�룬ÿ���ϸ�Ķ�ά�붼���������һ��λ��
				for (; symbol != imageZbar.symbol_end(); ++symbol)
				{
					if (symbol->get_location_size() == 4)	//�ϸ�Ķ�ά�� 
					{
						vector<Point3f> pos_a;					//δУ��ͼƬ��ԭ����ϵ�ĽǶȶ��� [ÿ�����ֵĵ����꣨������������]Mat(X,Y,Z)
						vector<Point3f> coord_a;					//δУ��ͼƬ�о���ԭ���������� [ÿ�����ֵĵ����꣨������������]Mat(X,Y,Z)
						//cout << "�ϸ�Ķ�ά��" << endl;
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

						//��ʼ��PNPSolver��
						PNPSolver p4psolver;
						//��ʼ���������
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
						//�������������������
						if (ReadPosFromQR)
						{//�Ӷ�ά���ж�ȡλ��
							String pos_str = symbol->get_data();
							cout << "���ͣ�" << endl << symbol->get_type_name() << endl << endl;
							cout << "���룺" << endl << symbol->get_data() << endl << endl;
							//���������ݷŽ�����
							inf.push_back(pos_str);
							//���ı��ж�ȡ��
							readPointsFromText(pos_str, p4psolver.Points3D, QRLen, QRheight, wallZ);
							//��¼��ǰ��Ϣ��������
							sucSerial.push_back(i);
						}
						else
						{//Ĭ�����Ͻ�Ϊ��0 0 0��
							p4psolver.Points3D.push_back(cv::Point3f(0, 0, 0));     //P1��ά����ĵ�λ�Ǻ���
							p4psolver.Points3D.push_back(cv::Point3f(0, QRLen, 0));   //P2
							p4psolver.Points3D.push_back(cv::Point3f(QRLen, QRLen, 0));  //P4
							p4psolver.Points3D.push_back(cv::Point3f(QRLen, 0, 0));   //P3
						}
						//cout << "p4psolver.Points3D" << p4psolver.Points3D << endl;
						//cout << "test2:�������������� = " << endl << p4psolver.Points3D << endl;
						//�����������ͼ������
						p4psolver.Points2D.push_back(points[3]);  //P4
						p4psolver.Points2D.push_back(points[2]);  //P3
						p4psolver.Points2D.push_back(points[1]);  //P2
						p4psolver.Points2D.push_back(points[0]);  //P1

						//cout << "test2:ͼ������������ = " << endl << p4psolver.Points2D << endl;
						//double area = contourArea(points, false);
						//cout << "��ά��ռ " << area << " ����" << endl << "ռ�� " << area *100/ (tmpImage.cols*tmpImage.rows) << " %" << endl;
						int method = 3;
						switch (method)
						{
						case 1:
							if (p4psolver.Solve(PNPSolver::METHOD::CV_P3P) == 0)
								cout << "test2:CV_P3P����: ���λ�ˡ�" << endl << "Oc����=" << p4psolver.Position_OcInW << endl << "�����ת=" << p4psolver.Theta_W2C << endl;
							break;
						case 2:
							if (p4psolver.Solve(PNPSolver::METHOD::CV_EPNP) == 0)
								cout << "test2:CV_EPNP����: ���λ�ˡ�" << endl << "Oc����=" << p4psolver.Position_OcInW << endl << "�����ת=" << p4psolver.Theta_W2C << endl;
							break;
						case 3:
							if (p4psolver.Solve(PNPSolver::METHOD::CV_ITERATIVE) == 0)
								cout << "test2:CV_ITERATIVE����: ���λ�ˡ�" << endl << "Oc����=" << p4psolver.Position_OcInW << endl << "�����ת=" << p4psolver.Theta_W2C << endl;
							break;
						}
						double relativeX = p4psolver.Position_OcInW.x - p4psolver.Points3D[0].x;
						double relativeY = p4psolver.Position_OcInW.y - p4psolver.Points3D[0].y;
						double relativeZ = p4psolver.Position_OcInW.z - p4psolver.Points3D[0].z;
						cout << "�����ά��ʽ������� " << sqrt(relativeX*relativeX + relativeY*relativeY + relativeZ*relativeZ) << " ����" << endl;
						//LabView��д�ļ��ӿ�
						/*ofstream fout1("D:\\pnp_theta.txt");
						fout1 << p4psolver.Theta_W2C.x << endl << p4psolver.Theta_W2C.y << endl << p4psolver.Theta_W2C.z << endl;
						fout1.close();
						ofstream fout2("D:\\pnp_t.txt");
						fout2 << p4psolver.Position_OcInW.x << endl << p4psolver.Position_OcInW.y << endl << p4psolver.Position_OcInW.z << endl;
						fout2.close();*/
						//������ͽǶȷ�������ͽǶ���̬��
						coord_a.push_back(p4psolver.Position_OcInW);
						pos_a.push_back(p4psolver.Theta_W2C);
						//���������ڵ�ͼ�еľ���λ�ˣ���λΪ���ס���
						//��ÿ����ά��ľ���λ�˼��뵽abs_pos_a��abs_coord_a��
						calcAbsAngleCoord(pos_a, coord_a, p4psolver.Points3D, abs_pos_a, abs_coord_a);
						for (int i = 0; i < abs_pos_a.size(); i++)
						{
							all_pos.x += abs_pos_a[i].x;
							all_pos.y += abs_pos_a[i].y;
							all_pos.z += abs_pos_a[i].z;
						}
					}
					counter++;
					//cout << "���꣺" << zbar_symbol_get_loc_x(symbol,index) << 
				}
				if (showSuccessRate)//�����ά��ɹ���
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
						cout << "�ɹ��ʣ�" << (double)sum / 30.f *100.f << " %" << endl;
					}
				}
				//imshow("Dst Image", dst);
				int press = waitKey(20);
				if (press == 51) //��3�������ع�
				{
					exposure--;
					captureLeft.set(CV_CAP_PROP_EXPOSURE, exposure);
				}
				else if (press == 54) //��6�������ع�
				{
					exposure++;
					captureLeft.set(CV_CAP_PROP_EXPOSURE, exposure);
				}
				else if (press == 27)                        //esc�˳�
				{
					destroyAllWindows();
					return 0;
				}
				//�����ά����Ϣ
				imageZbar.set_data(NULL, 0);
				//һ��������ж�ά����������һ��ƽ������
				Point3f finalPos_a, finalCoord_a;
				//���㵥�������ƽ������
				calcAvgPos(abs_pos_a, abs_coord_a, finalPos_a, finalCoord_a);
				Point3f finalRightPos_a, finalRightCoord_a;
				//����R��T������Ŀ�����̬
				calcRightPos(finalPos_a, finalCoord_a, RVect[i], TVect[i], finalRightPos_a, finalRightCoord_a);
				//�������ϵͳ��������
				Point3f finalMidCoord(finalCoord_a.x / 2 + finalRightCoord_a.x / 2 - 1, finalCoord_a.y / 2 + finalRightCoord_a.y / 2 - 1, finalCoord_a.z / 2 + finalRightCoord_a.z / 2 - 1);
				Point3f finalMidPos(finalPos_a.x / 2 + finalRightPos_a.x / 2, finalPos_a.y / 2 + finalRightPos_a.y / 2, finalPos_a.z / 2 + finalRightPos_a.z / 2);
				//����������꣬����������ڵ�ͼ��
				//���ֵĴ�λ����ֹ�ظ�
				Point Shift1(-30, -30);
				Point Shift2(30, 30);
				//�ڵ�ͼ�л����
				try
				{
					//�����ϵͳ����
					drawCamOnMap(finalMidPos, finalMidCoord, Point(0, 0), Scalar(255, 255, 255), Scalar(0, 130, 255), Scalar(255, 255, 255), tmpMapImage);
					//�������
					drawCamOnMap(finalPos_a, finalCoord_a, Shift1, Scalar(255, 255, 0), Scalar(255, 255, 255), Scalar(0, 0, 255), tmpMapImage);
					//�������
					drawCamOnMap(finalRightPos_a, finalRightCoord_a, Shift2, Scalar(255, 0, 255), Scalar(255, 255, 255), Scalar(255, 0, 0), tmpMapImage);
				}
				catch (...)
				{
					cout << "draw wrong" << endl;
				}
				//��ʾ�������
				resize(tmpImage, tmpImage, Size(SCREEN_WIDTH / p, SCREEN_HEIGHT / p), 0, 0);
				imshow(to_string(windowNum), tmpImage);
				moveWindow(to_string(windowNum), (windowNum % p)*tmpImage.cols, (windowNum / p)*tmpImage.rows);
				windowNum++;
			}
		}


		//�ɼ��������
		sucVCs[sucCamNum - 1] >> sucImages[sucCamNum - 1];

		vector<vector<Mat>> b;					//У������ͼƬ���� [������][ͼƬ��]
		vector<vector<Mat>> centroid;					//��ά������������� [������][ÿ�����ͼƬ�е�����]Mat(X,Y)
		vector<Point3f> coord_b;					//У����ͼƬ�о���ԭ���������� [������][ÿ�����ͼƬ�е�����]Mat(X,Y,Z)
		vector<Mat> disp_b;					//�Ӳ�ͼ����

		//ÿ�������ʶ��λ��  ��άʶ��λ��
		for (int i = 0; i < sucCamNum - 1; i++)	//ÿһ�������ȡͼ��  ��άʶ��λ��
		{

			if (SHOW_3D)
			{
				int d = 0;
				bool isQRSuc;
				vector<Point> doubleCenter;	//˫Ŀ��ά������
				//ʶ���ά�룬��������Ӳ�d����������doubleCenter�����Z
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
					picLeftROI[i] = sucImages[i];					//��ʼ��picLeftROI����
					picRightROI[i] = sucImages[i + 1];
					vector<Mat> recImages;
					recImages.push_back(sucImages[i]);
					recImages.push_back(sucImages[i + 1]);

					recImagesVect[i] = recImages;			//��ʼ��recImagesVect����
					remap(picLeftROI[i], recImagesVect[i][0], rmapFinalVect[i][0][0], rmapFinalVect[i][0][1], CV_INTER_LINEAR);	//У��
					remap(picRightROI[i], recImagesVect[i][1], rmapFinalVect[i][1][0], rmapFinalVect[i][1][1], CV_INTER_LINEAR);	//У��	
					if (IS_ROI)
					{
						picLeftROI[i] = recImagesVect[i][0](RoiVect[i]);	//����ͼ��ROIΪ��׼��ȡROI
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
						//ֱ������һ����ά����Ͷ�Ӧ���Ӳ�ֵ����ͨ����ͶӰ�����������������е���ά��
						if (ReadPosFromQR)
						{//�Ӷ�ά���ж�ȡλ��
							//���������ݷŽ�����
							String pos_str = QRText;
							//���ı��ж�ȡ��
							readPointsFromText(pos_str, Point3D, QRLen, QRheight, wallZ);
							//��¼��ǰ��Ϣ��������
							sucSerial.push_back(i);
						}
						else
						{//Ĭ�����Ͻ�Ϊ��0 0 0��
							Point3D.push_back(cv::Point3f(0, 0, 0));     //P1��ά����ĵ�λ�Ǻ���
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
						//��Բ cvPoint:ȷ��Բ������  200��Բ�İ뾶 CV_RGB��Բ����ɫ 3����Ȧ�Ĵ�ϸ  
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
						//�����ά����
						putText(picLeftROI[i], ss.str(), doubleCenter[0], FONT_HERSHEY_DUPLEX, 0.6, CV_RGB(255, 0, 0), 1, 8, false); //P3 text
						cout << "�����ά��ʽ������� " << sqrt(point.x*point.x + point.y*point.y + point.z*point.z) << " ����" << endl;
						//cout << "�����ά��ʽ����Z���� " << point.z << " ����" << endl;
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
						//ֱ������һ����ά����Ͷ�Ӧ���Ӳ�ֵ����ͨ����ͶӰ�����������������е���ά��
						point_to_xyz(doubleCenter[0].x, doubleCenter[0].y, d, QVect[i], point);
						cout << "point" << point << endl;
						stringstream ss;
						//ss << "(" << point.x << "," << point.y << "," << point.z << ")";
						ss << "Z���룺"  << point.z ;
						//��Բ cvPoint:ȷ��Բ������  200��Բ�İ뾶 CV_RGB��Բ����ɫ 3����Ȧ�Ĵ�ϸ  
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
						//�����ά����
						putText(sucImages[i], ss.str(), doubleCenter[0], FONT_HERSHEY_DUPLEX, 0.6, CV_RGB(255, 0, 0), 1, 12, false); //P3 text
						cout << "�����ά��ʽ����Z���� " << point.z << " ����" << endl;
						cout << "�����ά��ʽ������� " << sqrt(point.x*point.x + point.y*point.y + point.z*point.z) << " ����" << endl;
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
		//��ʾ��ͼ
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