#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include "math.h"
#ifdef linux  
#include <unistd.h>  
#include <dirent.h>  
#endif  
#ifdef WIN32  
#include <direct.h>  
#include <io.h>  
#endif

using namespace cv;
using namespace std;

void saveStereoResults(const string fname, const double err, const vector<Mat> cameraMatrix, const vector<Mat> distCoeffs, const Mat R, const Mat T, const Mat R1, const Mat R2, const Mat P1, const Mat P2, const Mat Q){

	FileStorage fs(fname, FileStorage::WRITE);
	if (!fs.isOpened()){
		//bug_info(fname + "打开失败！！");
		return;
	}
	fs << "err" << err;
	int cnum = cameraMatrix.size();
	for (int i = 0; i < cnum; i++){
		string s = "cameraMatrix" + to_string(i + 1);
		fs << s << cameraMatrix[i];
		s = "distCoeffs" + to_string(i + 1);
		fs << s << distCoeffs[i];
	}

	fs << "R" << R;
	fs << "T" << T;
	fs << "R1" << R1;
	fs << "R2" << R2;
	fs << "P1" << P1;
	fs << "P2" << P2;
	fs << "Q" << Q;

	cout << "结果已保存至" << fname << endl;

	fs.release();
}
void loadStereoResults(const string fname, const int cnum, vector<Mat> &cameraMatrix, vector<Mat> &distCoeffs, Mat &R, Mat &T, Mat &R1, Mat &R2, Mat &P1, Mat &P2, Mat &Q){
	FileStorage fs(fname, FileStorage::READ);
	if (!fs.isOpened()){
		//bug_info(fname + "打开失败！！");
		return;
	}
	cameraMatrix.resize(cnum);
	distCoeffs.resize(cnum);
	for (int i = 0; i < cnum; i++){
		string s = "cameraMatrix" + to_string(i + 1);
		fs[s] >> cameraMatrix[i];
		s = "distCoeffs" + to_string(i + 1);
		fs[s] >> distCoeffs[i];
	}

	fs["R"] >> R;
	fs["T"] >> T;
	fs["R1"] >> R1;
	fs["R2"] >> R2;
	fs["P1"] >> P1;
	fs["P2"] >> P2;
	fs["Q"] >> Q;
	fs.release();

	cout << "已成功从" + fname + "读入数据" << endl;
}


int main(){
	string dir1 = "camera12";		//图片所在文件夹
	string fname1 = dir1 + ".xml";	//生成的图片列表xml名字
	string resultDir1 = dir1 + "_results.xml";//保存结果xml名字

	string dir2 = "camera13";		//图片所在文件夹
	string fname2 = dir2 + ".xml";	//生成的图片列表xml名字
	string resultDir2 = dir2 + "_results.xml";//保存结果xml名字

	string dir3 = "camera23_Cal";		//图片所在文件夹
	string fname3 = dir3 + ".xml";	//生成的图片列表xml名字
	string resultDir3 = dir3 + "_results.xml";//保存结果xml名字

	string dir4 = "camera23";		//图片所在文件夹
	string fname4 = dir4 + ".xml";	//生成的图片列表xml名字
	string resultDir4 = dir4 + "_results.xml";//保存结果xml名字

	vector<string> l[2]; //图片名字列表
	vector<Mat> images[2]; //图片列表

	vector<Mat> cameraMatrix, distCoeffs; //相机内参数  畸变参数
	Mat R12, R13, R23, R1, R2, R12Invert, T12, T13, T23, R23Cal, T23Cal, RErr, R23CalInv, RErr1, RErr2, 
		TErr, TErr1,TErr2, R23Inv, T23Inv, T23CalInv, P1, P2, Q;  //旋转  平移  相对距离

	double err = 0.;// = stereo_calib(images, cameraMatrix, distCoeffs, R, T, R1, R2, P1, P2, Q, 1);
	//图片列表，相机内参数，畸变参数，平移，旋转，相对距离

	loadStereoResults(resultDir1, 2, cameraMatrix, distCoeffs, R12, T12, R1, R2, P1, P2, Q);	//从xml读入相机1-2结果R12
	loadStereoResults(resultDir2, 2, cameraMatrix, distCoeffs, R13, T13, R1, R2, P1, P2, Q);	//从xml读入相机1-3结果R13
	loadStereoResults(resultDir4, 2, cameraMatrix, distCoeffs, R23, T23, R1, R2, P1, P2, Q);	//从xml读入相机1-3结果R23

	int showProcess = 0;	//是否显示过程
	R12Invert = R12.inv(0); //R12^(-1)
	R23Cal = R12Invert.mul(R13); // Rerr = R23 - R23Cal
	Scalar r_cal = sum(R23Cal);
	Scalar r_origin = sum(R23);
	cout << "计算所得R矩阵的和" << r_cal << R23Cal << endl;	//计算R矩阵的和	
	err = fabs(r_cal[0] - r_origin[0]) / r_cal[0];
	RErr = R23 - R23Cal; // Rerr = R23 - R23Cal
	R23Inv = R23.inv(DECOMP_LU);  // 1 / R23
	R23CalInv = R23Cal.inv(DECOMP_LU); //1 / R23Cal
	RErr1 = RErr.mul(R23Inv); //RErrPercentage = RErr *1 / R23
	RErr2 = RErr.mul(R23CalInv); //RErrPercentage = RErr *1 / R23Cal
	if(showProcess)
	{
		cout << "err = " << err << endl;
		cout << "计算所得R矩阵的和"<<r_cal[0] << endl;	//计算R矩阵的和	
		cout <<  "标定所得R矩阵的和" << r_origin[0] << endl;//标定R矩阵的和  
		cout << "标定所得R23：" << endl << R23 << endl << "计算所得R23：" << endl << R23Cal << endl << "误差：" << endl << RErr<<endl;
		cout << "标定R23的逆矩阵为:" << endl << R23Inv << endl << "计算R23的逆矩阵为:" << endl << R23CalInv << endl;
		cout << "误差与标定R23比例为：" << endl << RErr1 << endl << "与计算所得R23比例为：" << endl << RErr2 << endl;
	}
	T23Cal = (T13 - T12);	
	Scalar t_cal = sum(T23Cal);
	Scalar t_origin = sum(T23);
	//cout << t_cal[0] << endl;//计算T矩阵的和	
	//cout << t_origin[0] << endl;//标定T矩阵的和
	TErr = T23 - T23Cal; // TErr = T23 - T23Cal
	cout << "标定所得T23为" << endl << T23 << endl << "计算所得T23为" << endl << T23Cal << endl << "T23的误差为" << TErr << endl;

	saveStereoResults(resultDir3, err, cameraMatrix, distCoeffs, R23Cal, T23Cal, R1, R2, P1, P2, Q);	//若结果已保存则可注释掉这两句
	cout << "计算并存储完成！" << endl;
	getchar();

}