// NewStereo1.cpp : �������̨Ӧ�ó������ڵ㡣
//

//#include "stdafx.h"

#include "single_camera_calib.h"
#include "stereo_camera_calib.h"
#include <io.h>  
#include <string>
#include <sstream>
#include <windows.h>
#include <vector>
#include <iomanip>
#include <opencv2/opencv.hpp>  
#include <iomanip>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
using namespace std;
using namespace cv;



static bool readStringList(const string& filename, vector<string>& l){
	//get file namelist from xml.
	l.resize(0);
	FileStorage fs(filename, FileStorage::READ);
	if (!fs.isOpened())
		return false;
	FileNode n = fs.getFirstTopLevelNode();
	if (n.type() != FileNode::SEQ)
		return false;
	FileNodeIterator it = n.begin(), it_end = n.end();
	for (; it != it_end; ++it)
		l.push_back((string)*it);
	return true;
}



static int readImages(const vector<string> l, vector<Mat> &images){
	/*
	get images from file name list
	l��ͼƬ��������
	dir:	ͼƬ�����ļ�������
	*/
	images.resize(0);
	Size imageSize;
	int n = l.size();
	for (int i = 0; i < n; ++i){
		//cout << l[i] << endl;	//�ļ�����
		Mat t = imread(l[i], -1);
		if (t.empty()) continue;
		//resize(t, t, Size(640, 360));
		if (imageSize == Size())
			imageSize = t.size();
		if (t.size() != imageSize) continue;
		images.push_back(t);
	}
	return images.size();
}
static int readImages(const vector<string> l, const string dir, vector<Mat> &images){
	/*
	get images from file name list
	l��ͼƬ��������
	dir:	ͼƬ�����ļ�������
	*/
	images.resize(0);
	Size imageSize;
	int n = l.size();
	cout << n << endl;
	for (int i = 0; i < n; ++i){
		string curDir = dir + "\\" + l[i];
		cout << curDir << endl;
		Mat t = imread(curDir, -1);
		if (t.empty()) continue;
		if (imageSize == Size())
			imageSize = t.size();
		if (t.size() != imageSize) continue;
		images.push_back(t);
	}
	return images.size();
}

static void makeXML(const string fname, const string dir){
	/*
	����dir�ļ����������ļ����֣�����ͼƬ�б�xml

	fname:	���ɵ�xml����
	dir:	ͼƬ�����ļ�������
	*/
	//cout << "makeXML" << endl;
	FileStorage fs(fname, FileStorage::WRITE);
	//cout << "fname" << fname << endl;
	fs << "images" << "[";

	_finddata_t file;
	long lf;
	char tdir[155];
	_getcwd(tdir, 150);
	string cate_dir = string(tdir) + "\\" + dir + "\\*";

	//cout << "cate_dir" << cate_dir << endl;

	int nimages = 0;
	if ((lf = _findfirst(cate_dir.c_str(), &file)) == -1) {
		cout << "δ�ҵ����ļ��У���" << endl;
		return;
	}

	else {
		while (_findnext(lf, &file) == 0) {
			if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0 || file.attrib & _A_SUBDIR)	continue;
			string curFile = dir + "\\" + string(file.name);
			fs << curFile;
			cout << file.name << endl;
			nimages++;
		}
	}
	_findclose(lf);

	fs << "]";
	fs.release();

	printf("���ҵ�%d��ͼƬ!\n", nimages);
}
static void saveResult(const string filename, const Mat cameraMatrix, const Mat distCoeffs, const vector<Mat> rvecs, const vector<Mat> tvecs, const double err){
	//save the result to xml
	FileStorage fs(filename, FileStorage::WRITE);
	fs << "err" << err;
	fs << "cameraMatrix" << cameraMatrix;
	fs << "distCoeffs" << distCoeffs;
	int num = rvecs.size();
	for (int i = 0; i < num; i++){
		string cur = "rvecOf" + to_string(i + 1) + "image";
		fs << cur << rvecs[i];
		cur = "tvecOf" + to_string(i + 1) + "image";
		fs << cur << tvecs[i];
	}
	fs.release();
}
static void handleResult(const string name, const Mat cameraMatrix, const Mat distCoeffs, const vector<Mat> rvecs, const vector<Mat> tvecs, const double err){
	//handle the result
	saveResult("camera01_results.xml", cameraMatrix, distCoeffs, rvecs, tvecs, err);
	cout << "���Ϊ" << err << endl;
}


int main(int argc, char** argv){	//hello.exe Shiqi Yu  argc��ֵ�� 3��argv[0]��"hello.exe"��argv[1]��"Shiqi"��argv[2]��"Yu"��
	//cout << argv[1] << endl;
	int MONO_CALIB = 0;
	cout << "��ѡ��Ŀ�궨���Ŀ�궨" << endl;
	cout << "1 : ��Ŀ�궨  2 : ��Ŀ�궨" << endl;
	char choiceMono;
	do
	{
		cin >> choiceMono;
	} while (choiceMono != '1' && choiceMono != '2');
	if (choiceMono == '1')
	{
		MONO_CALIB = 1;	//�Ƿ���ǿ�Աȶ�
	}
	else MONO_CALIB = 0;
	//��Ŀ�궨
	if (MONO_CALIB)
	{
		int dirNum = 1; //�ļ�������
		//dirNum = 2; //�ļ�������
		cout << "�ļ��е�����Ϊ" << dirNum << endl;//�ļ�������
		//system("PAUSE"); 
		vector<string> fname;
		vector<string> resultDir;				//�궨������ļ�������
		vector<string> imageAbsDir;				//���������ͼƬ����·������
		vector<vector<string>> imageAbsDirVect;	//ͼƬ·��������
		vector<Mat> images;						//ÿ�������ͼƬMat����
		vector<vector<Mat>> imagesVect;			//ͼƬMat������

		stringstream ss1;
		String dir;					//xml���ɵ�����
		ss1 << "camera01"; //.\camera01
		ss1 >> dir;
		stringstream ss2;
		String dir1;				//��ʵ·����
		ss2 << dir << "\\" << "left";					//.\camera01\left
		ss2 >> dir1;
		stringstream ss3;
		String filelistname;
		ss3 << dir << ".xml"; //.\camera01.xml
		ss3 >> filelistname;
		//fname.push_back(filename);
		makeXML(filelistname, dir1);	//����dir1 make��filename.xml�ļ�
		stringstream ss4;
		String resultname;
		ss4 << dir << "_results.xml"; //.\camera01_results.xml
		ss4 >> resultname;

		resultDir.push_back(resultname);			//�궨����ļ����Ž��ļ�������
		readStringList(filelistname, imageAbsDir);	//������������������о���·��
		imageAbsDirVect.push_back(imageAbsDir);		//������ͼƬ·��������
		readImages(imageAbsDir, images);			//���������ж����������������ͼ��

		Mat cameraMatrix, distCoeffs;
		vector<Mat> rvecs, tvecs;
		double err = single_calib(images, cameraMatrix, distCoeffs, rvecs, tvecs, 1);
		handleResult(resultname, cameraMatrix, distCoeffs, rvecs, tvecs, err);
	}
	//˫Ŀ�궨
	else
	{
		char argnum = *argv[1];
		int dirNum = (int)argnum - 48; //�ļ�������
		//dirNum = 2; //�ļ�������
		cout << "�ļ��е�����Ϊ" << dirNum << endl;//�ļ�������
		//system("PAUSE"); 
		vector<string> fname;
		vector<string> resultDir;				//�궨������ļ�������
		vector<string> imageAbsDir;				//���������ͼƬ����·������
		vector<vector<string>> imageAbsDirVect;	//ͼƬ·��������
		vector<Mat> images;						//ÿ�������ͼƬMat����
		vector<vector<Mat>> imagesVect;			//ͼƬMat������

		/***********				��ȡͼƬ			****************/
		for (int i = 0; i < dirNum; i++)
		{
			stringstream ss1;
			String dir;					//xml���ɵ�����
			ss1 << "camera" << setw(2) << setfill('0') << i + 1; //.\camera01
			ss1 >> dir;
			stringstream ss2;
			String dir1;				//��ʵ·����
			ss2 << dir << "\\" << "left";					//.\camera01\left
			ss2 >> dir1;
			stringstream ss3;
			String filelistname;
			ss3 << dir << ".xml"; //.\camera01.xml
			ss3 >> filelistname;
			//fname.push_back(filename);
			makeXML(filelistname, dir1);	//����dir1 make��filename.xml�ļ�
			stringstream ss4;
			String resultname;
			ss4 << dir << "_results.xml"; //.\camera01_results.xml
			ss4 >> resultname;

			resultDir.push_back(resultname);			//�궨����ļ����Ž��ļ�������
			readStringList(filelistname, imageAbsDir);	//������������������о���·��
			imageAbsDirVect.push_back(imageAbsDir);		//������ͼƬ·��������
			readImages(imageAbsDir, images);			//���������ж����������������ͼ��
			imagesVect.push_back(images);				//������ͼ�����ͼƬMat������
			//ss << /////////////////////////////
		}
		cout << "ͼƬ��ȡ���" << endl;

		/***********		��ʼ�궨��д����		****************/
		vector<Mat> cameraMatrix, distCoeffs;
		Mat R, T, R1, R2, P1, P2, Q;
		vector<vector<Mat>> cameraMatrixVect, distCoeffsVect;
		vector<Mat> RVect, TVect, R1Vect, R2Vect, P1Vect, P2Vect, QVect;
		vector<Rect> ROI;					//һ�������ROI
		vector<vector<Rect>> ROIVect;		//һ����һ�������ROI
		for (int i = 0; i < dirNum - 1; i++)
		{
			vector<Mat> tmpimg[2];				//�������������ͼƬ����
			tmpimg[0] = imagesVect[i];
			tmpimg[1] = imagesVect[i + 1];
			cout << "���ڱ궨�� " << i << " �����" << endl;
			double err = stereo_calib(tmpimg, cameraMatrix, distCoeffs, R, T, R1, R2, P1, P2, Q, ROI, 1);
			cameraMatrixVect.push_back(cameraMatrix);
			distCoeffsVect.push_back(distCoeffs);
			RVect.push_back(R);
			TVect.push_back(T);
			R1Vect.push_back(R1);
			R2Vect.push_back(R2);
			P1Vect.push_back(P1);
			P2Vect.push_back(P2);
			QVect.push_back(Q);
			ROIVect.push_back(ROI);
			saveStereoResults(resultDir[i], err, cameraMatrix, distCoeffs, R, T, R1, R2, P1, P2, Q, ROI);	//������ѱ������ע�͵�������
			cout << "�� " << i << " ���������Ѵ���" << resultDir[i] << endl;
			//ss << /////////////////////////////
		}
		Mat rmap[2][2];
		vector<vector<Mat>> unremapImageVect, remappedImageVect;//δУ����У�����ͼƬ		
		for (int i = 0; i < dirNum - 1; i++)
		{
			vector<Mat> unremapImage, remappedImage;	//δУ����һ�Ժ�У�����һ��ͼƬ
			unremapImage.push_back(imagesVect[i][0]);			//��ʼ��unremap��remapped����
			unremapImage.push_back(imagesVect[i + 1][0]);
			unremapImageVect.push_back(unremapImage);			//��ʼ��unremapImageVect��remappedImageVect����
			remappedImage.push_back(imagesVect[i][0]);
			remappedImage.push_back(imagesVect[i + 1][0]);
			remappedImageVect.push_back(remappedImage);
			cout << "����У���� " << i << " ��ͼƬ" << endl;
			initUndistortRectifyMap(cameraMatrixVect[i][0], distCoeffsVect[i][0], R1Vect[i], P1Vect[i], unremapImageVect[i][0].size(), CV_16SC2, rmap[0][0], rmap[0][1]);
			initUndistortRectifyMap(cameraMatrixVect[i][1], distCoeffsVect[i][1], R2Vect[i], P2Vect[i], unremapImageVect[i][0].size(), CV_16SC2, rmap[1][0], rmap[1][1]);
			remap(unremapImageVect[i][0], remappedImageVect[i][0], rmap[0][0], rmap[0][1], CV_INTER_LINEAR);
			remap(unremapImageVect[i][1], remappedImageVect[i][1], rmap[1][0], rmap[1][1], CV_INTER_LINEAR);
			//imshow("left", remappedImageVect[i][0]);
			//imshow("right", remappedImageVect[i][1]);
			lengthErrorMeasure(remappedImageVect[i], QVect[i], i);
			waitKey(0);
		}
	}
}
/*string dir1 = "camera01";		//ͼƬ�����ļ���
string fname1 = dir1 + ".xml";	//���ɵ�ͼƬ�б�xml����
string resultDir1 = dir1 + "_results.xml";//������xml����

string dir2 = "camera02";		//ͼƬ�����ļ���
string fname2 = dir2 + ".xml";	//���ɵ�ͼƬ�б�xml����
string resultDir2 = dir2 + "_results.xml";//������xml����

makeXML(fname1, dir1);
makeXML(fname2, dir2);	//XMLֻ������һ��

vector<string> l[2];//ͼƬ·��������
vector<Mat> images[2];
readStringList(fname1, l[0]);

readImages(l[0], images[0]);
//cout << "!" << endl;
readStringList(fname2, l[1]);
readImages(l[1], images[1]);

vector<Mat> cameraMatrix, distCoeffs;
Mat R, T, R1, R2, P1, P2, Q;
vector<Rect> ROI;
double err = stereo_calib(images, cameraMatrix, distCoeffs, R, T, R1, R2, P1, P2, Q, ROI, 1);
saveStereoResults(resultDir1, err, cameraMatrix, distCoeffs, R, T, R1, R2, P1, P2, Q, ROI);	//������ѱ������ע�͵�������

//loadStereoResults(resultDir1, 2, cameraMatrix, distCoeffs, R, T, R1, R2, P1, P2, Q, ROI);	//��xml������
Mat rmap[2][2];
initUndistortRectifyMap(cameraMatrix[0], distCoeffs[0], R1, P1, img[0].size(), CV_16SC2, rmap[0][0], rmap[0][1]);
initUndistortRectifyMap(cameraMatrix[1], distCoeffs[1], R2, P2, img[0].size(), CV_16SC2, rmap[1][0], rmap[1][1]);

remap(img[0], recImages[0], rmap[0][0], rmap[0][1], CV_INTER_LINEAR);
remap(img[1], recImages[1], rmap[1][0], rmap[1][1], CV_INTER_LINEAR);

imshow("1", recImages[0]);
imshow("2", recImages[1]);
waitKey(0);
lengthErrorMeasure(recImages, Q);		//ͨ���������Խǽǵ����������궨Ч��

//Mat img[2] = { images[0][0], images[1][0] };//У��ǰͼ��
//Mat recImages[2];		//У����ͼ��



//imshow("1", img[0]);
//imshow("2", img[1]);
//waitKey(0);*/


