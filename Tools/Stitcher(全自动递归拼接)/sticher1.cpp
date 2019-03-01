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
void addImages(vector<Mat> &ipt, int start, int end)//添加图片至ipt队列，命名格式：./chessboardPano/chessboardN.jpg
{	
	Size image_size;
	//读入标定图片
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
		ipt.push_back(image);  //将图片放入图片向量队列
		cout << "已读入第" << i << "张图片" << endl;
	}
	//return ipt;
}
Mat stitchhh(vector<Mat> ipt, int start, int end)//拼接子程序
{	
	Stitcher stitcher = Stitcher::createDefault(1);
	vector<vector<Rect>> rois;
	Mat res;
	cout << "准备拼接" << endl;
	Stitcher::Status status = stitcher.stitch(ipt, res);
	cout << start << "-" << end  << "拼接完毕"<< endl;

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
Mat AutoStitch(vector<Mat>ipt, int start, int end,int particle)//全自动递归拼接，ipt是图片序列，start是开始序号，end是结束序号，particle是图片拼接粒度（即允许进行单次拼接的最大图片数）
{	
	Stitcher stitcher = Stitcher::createDefault(1);
	Mat res;
	Mat part[20],tmp_image;
	vector<Mat> ipt_sum, ipt_sub[20];
	cout << "开始拼接" << start << "-" << end << endl;
	double image_num = end - start + 1;  //图片总数量 22  ，只能分为4组，不能再多

	//		if需要分组，分组递归后拼接
	if (image_num > particle)  
	{	
		int img_per_grp = ceil(image_num / particle); //每组图片最大数量 6
		int groupnum = ceil(image_num / img_per_grp); //总组数，取天棚 4
		cout << "图片数量为" << image_num << "，分为 " << groupnum << " 组." << endl;
		int crtGrpNum=0; //当前组号

		ipt_sub[crtGrpNum].push_back(ipt.at(0));
		cout << "已添加第 " << (start) / img_per_grp << " 组 ,第 " << (start)  << " 张图片" << endl;

		for (int i = 0; i < image_num; i++)	//正常的添加方法
		{//i / particle为当前组号，
			crtGrpNum = i / img_per_grp;
			ipt_sub[crtGrpNum].push_back(ipt.at(i));
			cout << "已添加第 " << (start + i) / img_per_grp << " 组 ,第 " << (start + i) << " 张图片" << endl;
		}
		//每组再次递归
		for(int i = 0; i < groupnum; i++)
		{
			tmp_image = AutoStitch(ipt_sub[i], start + i * img_per_grp, start + (i + 1) * img_per_grp - 1, particle);
			ipt_sum.push_back(tmp_image);
		}
		//递归完成后拼接
		cout << "准备拼接递归返回后的分块" << start << "-" << end << endl;
		Stitcher::Status status = stitcher.stitch(ipt_sum, res);
		if (status != Stitcher::OK)
		{
			cout << "Can't stitch images, error code = " << status << endl;
		}
		else{
			cout << "已拼接 " << start << "-" << end << endl;
			return res;
		}
	}

	//		if不需要分组，直接拼接
	else
	{
		cout << "准备拼接分块" << start << "-" << end << endl;
		Stitcher::Status status = stitcher.stitch(ipt, res);
		if (status != Stitcher::OK)
		{
			cout << "Can't stitch images, error code = " << status << endl;
		}
		else{
			cout << "已拼接 " << start << "-" << end << endl;
			return res;
		}
	}
}

int main()
{
	vector<Mat> ipt1;
	int autostitch = 0;		//是否全自动拼接
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
		//		制作边缘图片列表
		addImages(ipt1, 21, 22);addImages(ipt1, 1, 2);
		//		拼接边缘图片
		resE = stitchhh(ipt1, 21, 2);
		string imageFileName1;
		std::stringstream StrStm;
		StrStm << "result" << 19 << "-" << 3 << ".jpg";
		StrStm >> imageFileName1;
		resE = imread(imageFileName1, 1); 
		//imshow("resE", resE);
		waitKey(10);
		//      分割边缘图片
		Size size_of_edge = resE.size();
		cout << "Size of size_of_edge:" << size_of_edge << endl;
		Mat res1_L(size_of_edge.height, size_of_edge.width / 2, CV_8UC3);
		Mat res1_R(size_of_edge.height, size_of_edge.width / 2, CV_8UC3);

		res1_Lt = resE.colRange(0, size_of_edge.width / 2);
		res1_Rt = resE.colRange(size_of_edge.width / 2 , size_of_edge.width-1);

		//		将分割后的图片以及所有图片加入首尾队列中
		//		根据ipt队列拼出四幅图像res2-5	
		ipt2.push_back(res1_Lt); cout << "首图片已放入" << endl;  //将首图片放入首图片向量队列
		addImages(ipt2, 2, 6);
		cout << ipt2.size() << endl;
		res2 = stitchhh(ipt2, 1, 6);
		addImages(ipt3, 6, 12);
		res3 = stitchhh(ipt3, 6, 12);
		addImages(ipt4, 11, 16);
		res4 = stitchhh(ipt4, 11, 16);
		addImages(ipt5, 16, 21);
		ipt5.push_back(res1_Rt); cout << "尾图片已放入" << endl;//将尾图片放入尾图片向量队列
		res5 = stitchhh(ipt5, 16, 22);

		//		准备总体拼接
		Stitcher stitcher = Stitcher::createDefault(1);
		vector<vector<Rect>> rois;
		Mat res;
		cout << "准备拼接" << endl;
		//ipt1.push_back(res1);  //将图片放入图片向量队列
		ipt_final.push_back(res2);  //将图片放入图片向量队列
		ipt_final.push_back(res3);  //将图片放入图片向量队列
		ipt_final.push_back(res4);  //将图片放入图片向量队列
		ipt_final.push_back(res5);  //将图片放入图片向量队列
		Stitcher::Status status = stitcher.stitch(ipt_final, res);
		cout << "拼接完毕" << endl;

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