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
		//bug_info(fname + "��ʧ�ܣ���");
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

	cout << "����ѱ�����" << fname << endl;

	fs.release();
}
void loadStereoResults(const string fname, const int cnum, vector<Mat> &cameraMatrix, vector<Mat> &distCoeffs, Mat &R, Mat &T, Mat &R1, Mat &R2, Mat &P1, Mat &P2, Mat &Q){
	FileStorage fs(fname, FileStorage::READ);
	if (!fs.isOpened()){
		//bug_info(fname + "��ʧ�ܣ���");
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

	cout << "�ѳɹ���" + fname + "��������" << endl;
}


int main(){
	string dir1 = "camera12";		//ͼƬ�����ļ���
	string fname1 = dir1 + ".xml";	//���ɵ�ͼƬ�б�xml����
	string resultDir1 = dir1 + "_results.xml";//������xml����

	string dir2 = "camera13";		//ͼƬ�����ļ���
	string fname2 = dir2 + ".xml";	//���ɵ�ͼƬ�б�xml����
	string resultDir2 = dir2 + "_results.xml";//������xml����

	string dir3 = "camera23_Cal";		//ͼƬ�����ļ���
	string fname3 = dir3 + ".xml";	//���ɵ�ͼƬ�б�xml����
	string resultDir3 = dir3 + "_results.xml";//������xml����

	string dir4 = "camera23";		//ͼƬ�����ļ���
	string fname4 = dir4 + ".xml";	//���ɵ�ͼƬ�б�xml����
	string resultDir4 = dir4 + "_results.xml";//������xml����

	vector<string> l[2]; //ͼƬ�����б�
	vector<Mat> images[2]; //ͼƬ�б�

	vector<Mat> cameraMatrix, distCoeffs; //����ڲ���  �������
	Mat R12, R13, R23, R1, R2, R12Invert, T12, T13, T23, R23Cal, T23Cal, RErr, R23CalInv, RErr1, RErr2, 
		TErr, TErr1,TErr2, R23Inv, T23Inv, T23CalInv, P1, P2, Q;  //��ת  ƽ��  ��Ծ���

	double err = 0.;// = stereo_calib(images, cameraMatrix, distCoeffs, R, T, R1, R2, P1, P2, Q, 1);
	//ͼƬ�б�����ڲ��������������ƽ�ƣ���ת����Ծ���

	loadStereoResults(resultDir1, 2, cameraMatrix, distCoeffs, R12, T12, R1, R2, P1, P2, Q);	//��xml�������1-2���R12
	loadStereoResults(resultDir2, 2, cameraMatrix, distCoeffs, R13, T13, R1, R2, P1, P2, Q);	//��xml�������1-3���R13
	loadStereoResults(resultDir4, 2, cameraMatrix, distCoeffs, R23, T23, R1, R2, P1, P2, Q);	//��xml�������1-3���R23

	int showProcess = 0;	//�Ƿ���ʾ����
	R12Invert = R12.inv(0); //R12^(-1)
	R23Cal = R12Invert.mul(R13); // Rerr = R23 - R23Cal
	Scalar r_cal = sum(R23Cal);
	Scalar r_origin = sum(R23);
	cout << "��������R����ĺ�" << r_cal << R23Cal << endl;	//����R����ĺ�	
	err = fabs(r_cal[0] - r_origin[0]) / r_cal[0];
	RErr = R23 - R23Cal; // Rerr = R23 - R23Cal
	R23Inv = R23.inv(DECOMP_LU);  // 1 / R23
	R23CalInv = R23Cal.inv(DECOMP_LU); //1 / R23Cal
	RErr1 = RErr.mul(R23Inv); //RErrPercentage = RErr *1 / R23
	RErr2 = RErr.mul(R23CalInv); //RErrPercentage = RErr *1 / R23Cal
	if(showProcess)
	{
		cout << "err = " << err << endl;
		cout << "��������R����ĺ�"<<r_cal[0] << endl;	//����R����ĺ�	
		cout <<  "�궨����R����ĺ�" << r_origin[0] << endl;//�궨R����ĺ�  
		cout << "�궨����R23��" << endl << R23 << endl << "��������R23��" << endl << R23Cal << endl << "��" << endl << RErr<<endl;
		cout << "�궨R23�������Ϊ:" << endl << R23Inv << endl << "����R23�������Ϊ:" << endl << R23CalInv << endl;
		cout << "�����궨R23����Ϊ��" << endl << RErr1 << endl << "���������R23����Ϊ��" << endl << RErr2 << endl;
	}
	T23Cal = (T13 - T12);	
	Scalar t_cal = sum(T23Cal);
	Scalar t_origin = sum(T23);
	//cout << t_cal[0] << endl;//����T����ĺ�	
	//cout << t_origin[0] << endl;//�궨T����ĺ�
	TErr = T23 - T23Cal; // TErr = T23 - T23Cal
	cout << "�궨����T23Ϊ" << endl << T23 << endl << "��������T23Ϊ" << endl << T23Cal << endl << "T23�����Ϊ" << TErr << endl;

	saveStereoResults(resultDir3, err, cameraMatrix, distCoeffs, R23Cal, T23Cal, R1, R2, P1, P2, Q);	//������ѱ������ע�͵�������
	cout << "���㲢�洢��ɣ�" << endl;
	getchar();

}