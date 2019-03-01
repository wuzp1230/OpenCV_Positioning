#include <iostream>
#include <string>
#include <sstream>
#include <math.h>
#include <opencv2/opencv.hpp>  
#include <iomanip>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/stitching/stitcher.hpp"
using namespace cv;
using namespace std;
double factor = 0.5;
int image_count = 10;
void addImages(vector<Mat> &ipt, int start, int end)//���ͼƬ��ipt���У�������ʽ��./chessboardPano/chessboardN.jpg
{	
	Size image_size;
	//����궨ͼƬ
	for (int i = start; i <= end; i++)
	{
		string imageFileName;
		std::stringstream StrStm;
		StrStm << std::noskipws << "./chessboardPano/chessboard(" << i << ")";
		StrStm >> imageFileName;
		imageFileName += ".jpg";
		cv::Mat image = imread(imageFileName);
		resize(image, image, Size(1080, 1440));
		image_size = image.size();
		ipt.push_back(image);  //��ͼƬ����ͼƬ��������
		cout << "�Ѷ����" << i << "��ͼƬ" << endl;
	}
	//return ipt;
}
Mat stitchhh(vector<Mat> ipt, int start, int end)//ƴ���ӳ���
{	
	Stitcher stitcher = Stitcher::createDefault(1);
	vector<vector<Rect>> rois;
	Mat res;
	cout << "׼��ƴ��" << endl;
	Stitcher::Status status = stitcher.stitch(ipt, res);
	cout << start << "-" << end  << "ƴ�����"<< endl;

	if (status != Stitcher::OK)
	{
		cout << "Can't stitch images, error code = " << status << endl;
	}
	else{
		//imshow("3", res);
		//imwrite("result_stiches_1.png", res);	
		//StrStm << "./Pano1/camera" << setw(3) << setfill('0') << i;
		string imageFileName1;
		std::stringstream StrStm;
		StrStm << "result" << start << "-" << end << ".jpg";
		StrStm >> imageFileName1;
		imwrite(imageFileName1, res);
		return res;
	}

}
Mat AutoStitch(vector<Mat>ipt, int start, int end,int particle)//ȫ�Զ��ݹ�ƴ�ӣ�ipt��ͼƬ���У�start�ǿ�ʼ��ţ�end�ǽ�����ţ�particle��ͼƬƴ�����ȣ���������е���ƴ�ӵ����ͼƬ����
{	
	Stitcher stitcher = Stitcher::createDefault(1);
	Mat res;
	Mat part[20],tmp_image;
	vector<Mat> ipt_sum, ipt_sub[20];
	cout << "��ʼƴ��" << start << "-" << end << endl;
	double image_num = end - start + 1;  //ͼƬ������ 22  ��ֻ�ܷ�Ϊ4�飬�����ٶ�

	//		if��Ҫ���飬����ݹ��ƴ��
	if (image_num > particle)  
	{	
		int img_per_grp = ceil(image_num / particle); //ÿ��ͼƬ������� 6
		int groupnum = ceil(image_num / img_per_grp); //��������ȡ���� 4
		cout << "ͼƬ����Ϊ" << image_num << "����Ϊ " << groupnum << " ��." << endl;
		int crtGrpNum=0; //��ǰ���

		ipt_sub[crtGrpNum].push_back(ipt.at(0));
		cout << "����ӵ� " << (start) / img_per_grp << " �� ,�� " << (start)  << " ��ͼƬ" << endl;

		for (int i = 0; i < image_num; i++)	//��������ӷ���
		{//i / particleΪ��ǰ��ţ�
			crtGrpNum = i / img_per_grp;
			ipt_sub[crtGrpNum].push_back(ipt.at(i));
			cout << "����ӵ� " << (start + i) / img_per_grp << " �� ,�� " << (start + i) << " ��ͼƬ" << endl;
		}
		//ÿ���ٴεݹ�
		for(int i = 0; i < groupnum; i++)
		{
			tmp_image = AutoStitch(ipt_sub[i], start + i * img_per_grp, start + (i + 1) * img_per_grp - 1, particle);
			ipt_sum.push_back(tmp_image);
		}
		//�ݹ���ɺ�ƴ��
		cout << "׼��ƴ�ӵݹ鷵�غ�ķֿ�" << start << "-" << end << endl;
		Stitcher::Status status = stitcher.stitch(ipt_sum, res);
		if (status != Stitcher::OK)
		{
			cout << "Can't stitch images, error code = " << status << endl;
		}
		else{
			cout << "��ƴ�� " << start << "-" << end << endl;
			return res;
		}
	}

	//		if����Ҫ���飬ֱ��ƴ��
	else
	{
		cout << "׼��ƴ�ӷֿ�" << start << "-" << end << endl;
		Stitcher::Status status = stitcher.stitch(ipt, res);
		if (status != Stitcher::OK)
		{
			cout << "Can't stitch images, error code = " << status << endl;
		}
		else{
			cout << "��ƴ�� " << start << "-" << end << endl;
			return res;
		}
	}
}

int main()
{
	vector<Mat> ipt1;
	int autostitch = 0;		//�Ƿ�ȫ�Զ�ƴ��
	if (autostitch)
	{
		addImages(ipt1, 1, 22);
		Mat final = AutoStitch(ipt1, 1, 22, 4);
		imshow("final", final);
		//Stitcher stitcher = Stitcher::createDefault(1);
		imwrite("result_stiches_final.png", final);
		waitKey(0);
	}
	else
	{
		vector<Mat> ipt1, ipt2, ipt3, ipt4, ipt5, ipt_final;
		Mat resE, res1_Lt, res1_Rt, res2, res3, res4, res5;
		//		������ԵͼƬ�б�
		addImages(ipt1, 21, 22);addImages(ipt1, 1, 2);
		//		ƴ�ӱ�ԵͼƬ
		resE = stitchhh(ipt1, 21, 2);
		string imageFileName1;
		std::stringstream StrStm;
		StrStm << "result" << 19 << "-" << 3 << ".jpg";
		StrStm >> imageFileName1;
		resE = imread(imageFileName1, 1); 
		//imshow("resE", resE);
		waitKey(10);
		//      �ָ��ԵͼƬ
		Size size_of_edge = resE.size();
		cout << "Size of size_of_edge:" << size_of_edge << endl;
		Mat res1_L(size_of_edge.height, size_of_edge.width / 2, CV_8UC3);
		Mat res1_R(size_of_edge.height, size_of_edge.width / 2, CV_8UC3);

		res1_Lt = resE.colRange(0, size_of_edge.width / 2);
		res1_Rt = resE.colRange(size_of_edge.width / 2 , size_of_edge.width-1);

		//		���ָ���ͼƬ�Լ�����ͼƬ������β������
		//		����ipt����ƴ���ķ�ͼ��res2-5	
		ipt2.push_back(res1_Lt); cout << "��ͼƬ�ѷ���" << endl;  //����ͼƬ������ͼƬ��������
		addImages(ipt2, 2, 6);
		cout << ipt2.size() << endl;
		res2 = stitchhh(ipt2, 1, 6);
		addImages(ipt3, 6, 12);
		res3 = stitchhh(ipt3, 6, 12);
		addImages(ipt4, 11, 16);
		res4 = stitchhh(ipt4, 11, 16);
		addImages(ipt5, 16, 21);
		ipt5.push_back(res1_Rt); cout << "βͼƬ�ѷ���" << endl;//��βͼƬ����βͼƬ��������
		res5 = stitchhh(ipt5, 16, 22);

		//		׼������ƴ��
		Stitcher stitcher = Stitcher::createDefault(1);
		vector<vector<Rect>> rois;
		Mat res;
		cout << "׼��ƴ��" << endl;
		//ipt1.push_back(res1);  //��ͼƬ����ͼƬ��������
		ipt_final.push_back(res2);  //��ͼƬ����ͼƬ��������
		ipt_final.push_back(res3);  //��ͼƬ����ͼƬ��������
		ipt_final.push_back(res4);  //��ͼƬ����ͼƬ��������
		ipt_final.push_back(res5);  //��ͼƬ����ͼƬ��������
		Stitcher::Status status = stitcher.stitch(ipt_final, res);
		cout << "ƴ�����" << endl;

		if (status != Stitcher::OK)
		{
			cout << "Can't stitch images, error code = " << status << endl;
		}
		else{
			imshow("final", res);
			imwrite("result_stiches_final.png", res);
			waitKey(0);
		}
	}


	
}