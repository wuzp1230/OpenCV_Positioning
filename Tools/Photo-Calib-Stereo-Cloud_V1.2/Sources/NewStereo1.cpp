// NewStereo1.cpp : 定义控制台应用程序的入口点。
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
	l：图片名字序列
	dir:	图片所在文件夹名字
	*/
	images.resize(0);
	Size imageSize;
	int n = l.size();
	for (int i = 0; i < n; ++i){
		//cout << l[i] << endl;	//文件名字
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
	l：图片名字序列
	dir:	图片所在文件夹名字
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
	读入dir文件夹内所有文件名字，生成图片列表xml

	fname:	生成的xml名字
	dir:	图片所在文件夹名字
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
		cout << "未找到该文件夹！！" << endl;
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

	printf("共找到%d张图片!\n", nimages);
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
	cout << "误差为" << err << endl;
}


int main(int argc, char** argv){	//hello.exe Shiqi Yu  argc的值是 3，argv[0]是"hello.exe"，argv[1]是"Shiqi"，argv[2]是"Yu"。
	//cout << argv[1] << endl;
	int MONO_CALIB = 0;
	cout << "请选择单目标定或多目标定" << endl;
	cout << "1 : 单目标定  2 : 多目标定" << endl;
	char choiceMono;
	do
	{
		cin >> choiceMono;
	} while (choiceMono != '1' && choiceMono != '2');
	if (choiceMono == '1')
	{
		MONO_CALIB = 1;	//是否增强对比度
	}
	else MONO_CALIB = 0;
	//单目标定
	if (MONO_CALIB)
	{
		int dirNum = 1; //文件夹数量
		//dirNum = 2; //文件夹数量
		cout << "文件夹的数量为" << dirNum << endl;//文件夹数量
		//system("PAUSE"); 
		vector<string> fname;
		vector<string> resultDir;				//标定结果的文件名集合
		vector<string> imageAbsDir;				//单个相机的图片完整路径集合
		vector<vector<string>> imageAbsDirVect;	//图片路径总序列
		vector<Mat> images;						//每个相机的图片Mat序列
		vector<vector<Mat>> imagesVect;			//图片Mat总序列

		stringstream ss1;
		String dir;					//xml生成的名字
		ss1 << "camera01"; //.\camera01
		ss1 >> dir;
		stringstream ss2;
		String dir1;				//真实路径名
		ss2 << dir << "\\" << "left";					//.\camera01\left
		ss2 >> dir1;
		stringstream ss3;
		String filelistname;
		ss3 << dir << ".xml"; //.\camera01.xml
		ss3 >> filelistname;
		//fname.push_back(filename);
		makeXML(filelistname, dir1);	//根据dir1 make出filename.xml文件
		stringstream ss4;
		String resultname;
		ss4 << dir << "_results.xml"; //.\camera01_results.xml
		ss4 >> resultname;

		resultDir.push_back(resultname);			//标定结果文件名放进文件名集合
		readStringList(filelistname, imageAbsDir);	//制作出单个相机的所有绝对路径
		imageAbsDirVect.push_back(imageAbsDir);		//制作出图片路径总序列
		readImages(imageAbsDir, images);			//根据总序列读出单个相机的所有图像

		Mat cameraMatrix, distCoeffs;
		vector<Mat> rvecs, tvecs;
		double err = single_calib(images, cameraMatrix, distCoeffs, rvecs, tvecs, 1);
		handleResult(resultname, cameraMatrix, distCoeffs, rvecs, tvecs, err);
	}
	//双目标定
	else
	{
		char argnum = *argv[1];
		int dirNum = (int)argnum - 48; //文件夹数量
		//dirNum = 2; //文件夹数量
		cout << "文件夹的数量为" << dirNum << endl;//文件夹数量
		//system("PAUSE"); 
		vector<string> fname;
		vector<string> resultDir;				//标定结果的文件名集合
		vector<string> imageAbsDir;				//单个相机的图片完整路径集合
		vector<vector<string>> imageAbsDirVect;	//图片路径总序列
		vector<Mat> images;						//每个相机的图片Mat序列
		vector<vector<Mat>> imagesVect;			//图片Mat总序列

		/***********				读取图片			****************/
		for (int i = 0; i < dirNum; i++)
		{
			stringstream ss1;
			String dir;					//xml生成的名字
			ss1 << "camera" << setw(2) << setfill('0') << i + 1; //.\camera01
			ss1 >> dir;
			stringstream ss2;
			String dir1;				//真实路径名
			ss2 << dir << "\\" << "left";					//.\camera01\left
			ss2 >> dir1;
			stringstream ss3;
			String filelistname;
			ss3 << dir << ".xml"; //.\camera01.xml
			ss3 >> filelistname;
			//fname.push_back(filename);
			makeXML(filelistname, dir1);	//根据dir1 make出filename.xml文件
			stringstream ss4;
			String resultname;
			ss4 << dir << "_results.xml"; //.\camera01_results.xml
			ss4 >> resultname;

			resultDir.push_back(resultname);			//标定结果文件名放进文件名集合
			readStringList(filelistname, imageAbsDir);	//制作出单个相机的所有绝对路径
			imageAbsDirVect.push_back(imageAbsDir);		//制作出图片路径总序列
			readImages(imageAbsDir, images);			//根据总序列读出单个相机的所有图像
			imagesVect.push_back(images);				//将单张图像放入图片Mat总序列
			//ss << /////////////////////////////
		}
		cout << "图片读取完毕" << endl;

		/***********		开始标定并写入结果		****************/
		vector<Mat> cameraMatrix, distCoeffs;
		Mat R, T, R1, R2, P1, P2, Q;
		vector<vector<Mat>> cameraMatrixVect, distCoeffsVect;
		vector<Mat> RVect, TVect, R1Vect, R2Vect, P1Vect, P2Vect, QVect;
		vector<Rect> ROI;					//一对相机的ROI
		vector<vector<Rect>> ROIVect;		//一对又一对相机的ROI
		for (int i = 0; i < dirNum - 1; i++)
		{
			vector<Mat> tmpimg[2];				//左右两个相机的图片队列
			tmpimg[0] = imagesVect[i];
			tmpimg[1] = imagesVect[i + 1];
			cout << "正在标定第 " << i << " 对相机" << endl;
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
			saveStereoResults(resultDir[i], err, cameraMatrix, distCoeffs, R, T, R1, R2, P1, P2, Q, ROI);	//若结果已保存则可注释掉这两句
			cout << "第 " << i << " 对相机结果已存至" << resultDir[i] << endl;
			//ss << /////////////////////////////
		}
		Mat rmap[2][2];
		vector<vector<Mat>> unremapImageVect, remappedImageVect;//未校正和校正后的图片		
		for (int i = 0; i < dirNum - 1; i++)
		{
			vector<Mat> unremapImage, remappedImage;	//未校正的一对和校正后的一对图片
			unremapImage.push_back(imagesVect[i][0]);			//初始化unremap和remapped序列
			unremapImage.push_back(imagesVect[i + 1][0]);
			unremapImageVect.push_back(unremapImage);			//初始化unremapImageVect和remappedImageVect序列
			remappedImage.push_back(imagesVect[i][0]);
			remappedImage.push_back(imagesVect[i + 1][0]);
			remappedImageVect.push_back(remappedImage);
			cout << "正在校正第 " << i << " 对图片" << endl;
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
/*string dir1 = "camera01";		//图片所在文件夹
string fname1 = dir1 + ".xml";	//生成的图片列表xml名字
string resultDir1 = dir1 + "_results.xml";//保存结果xml名字

string dir2 = "camera02";		//图片所在文件夹
string fname2 = dir2 + ".xml";	//生成的图片列表xml名字
string resultDir2 = dir2 + "_results.xml";//保存结果xml名字

makeXML(fname1, dir1);
makeXML(fname2, dir2);	//XML只需生成一次

vector<string> l[2];//图片路径总序列
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
saveStereoResults(resultDir1, err, cameraMatrix, distCoeffs, R, T, R1, R2, P1, P2, Q, ROI);	//若结果已保存则可注释掉这两句

//loadStereoResults(resultDir1, 2, cameraMatrix, distCoeffs, R, T, R1, R2, P1, P2, Q, ROI);	//从xml读入结果
Mat rmap[2][2];
initUndistortRectifyMap(cameraMatrix[0], distCoeffs[0], R1, P1, img[0].size(), CV_16SC2, rmap[0][0], rmap[0][1]);
initUndistortRectifyMap(cameraMatrix[1], distCoeffs[1], R2, P2, img[0].size(), CV_16SC2, rmap[1][0], rmap[1][1]);

remap(img[0], recImages[0], rmap[0][0], rmap[0][1], CV_INTER_LINEAR);
remap(img[1], recImages[1], rmap[1][0], rmap[1][1], CV_INTER_LINEAR);

imshow("1", recImages[0]);
imshow("2", recImages[1]);
waitKey(0);
lengthErrorMeasure(recImages, Q);		//通过估算两对角角点间距离评估标定效果

//Mat img[2] = { images[0][0], images[1][0] };//校正前图像
//Mat recImages[2];		//校正后图像



//imshow("1", img[0]);
//imshow("2", img[1]);
//waitKey(0);*/


