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

//#define IMAGE_WIDTH 1280
//#define IMAGE_HEIGHT 720
//#define IMAGE_WIDTH  1280
//#define IMAGE_HEIGHT 720
//#define IMAGE_WIDTH  2592
//#define IMAGE_HEIGHT 1944
#define ROI_HEIGHT IMAGE_HEIGHT
//#define ROI_WIDTH (ROI_HEIGHT / 5 * 7)													// 标定板的长宽比为7:5	
//#define ROI_WIDTH (ROI_HEIGHT / 9 * 16)													// 标定板的长宽比为7:5	
//#define ROI_WIDTH (ROI_HEIGHT / 7 * 6)													// 标定板的长宽比为7:5	
#define ROI_WIDTH (ROI_HEIGHT / 3 * 4)													// 标定板的长宽比为7:5	
//#define CALIB 1


//#define CAP_WIDTH 2592
//#define CAP_HEIGHT 1944

int MONO_CALIB = 0;
int CALIB = 0;				//是否读取标定信息
int CONTRAST_ENHANCE = 1;	//是否增强对比度
int CENTER_LINE = 0;		//是否画中心基准线
//#define CAP_WIDTH 1280
//#define CAP_HEIGHT 960
const int startID = 0;		//第一个相机的标识号


//void mkdir(String dirname)
void mkdir(char* dirname)	//在当前目录下建立一个名字为dirname的文件夹
{

	stringstream namestream;
	//system("echo %~dp0");
	//system("dir");
	namestream << ".\\" << dirname;
	//String name = ss.str();
	char *mkch = new char[255];		//用于建立文件夹的字符串
	namestream >> mkch;			//stringstream to char*
	//ss.str("");
	/*ss << "dir " << name;
	char *dir_ch = new char[255];
	ss >> dir_ch;*/
	cout << mkch << endl;
	int status = _mkdir(mkch);
	if (status == 0)
	{
		//system(dir_ch);
		cout << "已成功创建 : " << namestream.str() << endl;
		//if (_rmdir(mkch) == 0) printf("Directory '\\testtmp' was successfully removed\n");
		//else printf("Problem removing directory '\\testtmp'\n");
	}
	else if (status == -1)
	{
		cout << namestream.str() << "文件夹已存在，创建失败" << endl;
	}

}
void logInfo(String s)
{
	cout << s << endl;
}

void takePictures(const int cnum, const string name)
{
	/*cout << "请选择单目拍照或多目拍照" << endl;
	cout << "1 : 单目拍照  2 : 多目拍照" << endl;
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
	//单目拍照
	if (MONO_CALIB)
	{
	}
	//多目拍照
	else
	{*/
		/*第i相机第j张图片保存路径为"name(0i)//left||right(0j)*/
	vector<VideoCapture> VCs;
	vector<Mat> images;

	vector<Mat>  image[3];
	Mat picLeftResize, picRightResize;	// 缩小图像到合适大小并显示	

	int press;				//按键记录器
	int picnumCount = 0;	//图片拍照计数器

	bool isReversed = false;


	int MaxDetectNum = 30;	//最大检测相机数量
	int sucCamNum = 0;	//成功读取的相机计数器
	vector<VideoCapture> sucVCs;	//成功读取的相机捕捉器序列
	vector<Mat> sucImages;		//成功读取的图片序列
	vector<int> sucCamSeq;

	cout << "是否增强对比度" << endl;
	cout << "1 : 增强  2 : 不增强" << endl;
	char choiceContrast;
	do
	{
		cin >> choiceContrast;
	} while (choiceContrast != '1' && choiceContrast != '2');
	if (choiceContrast == '1')
	{
		CONTRAST_ENHANCE = 1;	//是否增强对比度
	}
	else CONTRAST_ENHANCE = 0;


#define CAP_WIDTH 1280
#define CAP_HEIGHT 720

	double SCREEN_WIDTH = 1600;
	double SCREEN_HEIGHT = 900;

	/**********				检测可用相机数量			*****************/
	for (int i = 0; sucCamNum < cnum && i < MaxDetectNum; i++){		//设置相机参数并检测是否可用
		printf("正在检测第%d个视频捕捉器！\n", i);
		VCs.push_back(VideoCapture(startID + i));
		VCs[i].open(startID + i);
		int width = 0;
		VCs[i].set(CV_CAP_PROP_FRAME_WIDTH, width);
		VCs[i].set(CV_CAP_PROP_FRAME_WIDTH, CAP_WIDTH);
		VCs[i].set(CV_CAP_PROP_FRAME_HEIGHT, CAP_HEIGHT);
		if (CONTRAST_ENHANCE)
		{
			VCs[i].set(CV_CAP_PROP_CONTRAST, 200);
		}
		//images.push_back(Mat(CAP_HEIGHT, CAP_WIDTH, CV_8UC3));
		images.push_back(Mat());
		VCs[i] >> images[i];	//先检测一个图像	
		if (!images[i].data){
			printf("第%d个视频捕捉器不能取得图像！！\n", i);
		}
		else{
			sucVCs.push_back(VideoCapture(startID + i));
			sucCamSeq.push_back(sucCamNum);
			sucCamNum++;
			printf("已成功将第%d个视频捕捉器加入队列！\n", sucCamNum);
			sucImages.push_back(Mat());
		}
	}
	printf("成功数量：%d！\n", sucCamNum);
	printf("成功队列规模：%d！\n", sucVCs.size());
	if (sucCamNum == 0)	//未检测到，退出
	{
		printf("未检测到视频捕捉器,不能取得图像！！\n");
		waitKey(0);
		return;
	}


	char seqChoice;
	cout << "				请选择调整顺序选项" << endl;
	cout << "1.调整顺序并保存	2.从配置文件中读取顺序	3.不读取也不调整" << endl;
	do{
		cin >> seqChoice;
	} while (seqChoice <'1' || seqChoice > '3');

	long long int frameSum = 0;
	bool showSwapInfo = true;
	int swappingCamNum = 0;	//要对调的相机的设备号（大）
	int swappingWinNum = 0;	//要对调的相机的目标窗口号（小）
	bool isConfirmed = false;			//再次确认的开关
	stringstream camNum;	//保存输入的相机编号
	VideoCapture tempVC;

	/**	1.调整顺序并保存  2.从配置文件中读取顺序   3.不读取也不调整	****/
	if (seqChoice == '1')		//1.调整顺序并保存
	{
		/****************		改变相机顺序	************/
		while (1)
		{
			if (showSwapInfo)
			{
				cout << "请输入第 " << swappingWinNum << " 个相机当前所在的窗口编号，按回车键确定：" << endl;
				showSwapInfo = false;
			}
			for (int i = 0; i < sucCamNum; i++)	//读取图像
			{
				sucVCs[i] >> sucImages[i];
				imshow(to_string(i), sucImages[i]);
				//cout << sucImages[i].size() << endl;
			}

			press = waitKey(10);

			if (press >= 48 && press <= 57)	//0  -  9  48   -  57
			{
				//cout << press - 48 << endl;
				camNum << press - 48;
			}
			else if (press == 13)			//回车13	空格32  
			{
				camNum >> swappingCamNum;
				camNum.clear();
				if (!isConfirmed)				//未确认
				{
					if (swappingCamNum >= sucCamNum)	//相机号超界，不合法
					{
						cout << "相机号超界，请重新输入" << endl;
						isConfirmed = false;
					}
					else
					{
						cout << "确认将第 " << swappingCamNum << " 个相机的画面移至第" << swappingWinNum << "个窗口吗？" << endl;
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
					if (swappingWinNum == sucCamNum)	//全部对调完毕，开始保存文件
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
	else if (seqChoice == '2')		//	2.从配置文件中读取顺序	
	{

		char filename[] = "camera_seq.xml";
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
	else if (seqChoice == '3')		//	3.不读取也不调整
	{

	}

	/****************	读取标定信息	************/
	Vector<Vector<Mat>> recImagesVect;		//校正后图像对序列	[N][2]
	Mat rmap[2][2];					//每个相机的重投影矩阵
	//Vector<Mat[2][2]> rmapVect;				//重投影矩阵序列
	int x1, x2, x3, x4, y1, y2, y3, y4, xx = 0, yy = 0, xx2, yy2, h = CAP_HEIGHT, w = CAP_WIDTH;	//感兴趣区域序列
	Vector<Vector<Mat>> cameraMatrixVect, distCoeffsVect;		//校正后图像对序列	[N][2]
	Vector<Mat> RVect, TVect, R1Vect, R2Vect, P1Vect, P2Vect, QVect;	//相机参数	[N]
	Vector<Rect> RoiVect;				//相机ROI序列
	Vector<Mat> picLeft, picRight;				// 定义一个Mat变量，用于存储每一帧的图像
	Vector<Mat> picLeftROI, picRightROI;			//取过感兴趣区域的图像
	vector<Mat> picROI;				//取过感兴趣区域的图像

	destroyAllWindows();
	//cout << sucImages[i].size() << endl;
	if (sucCamNum != 1)
	{
		cout << "是否读取标定信息" << endl;
		cout << "1 : 读取  2 : 不读取" << endl;
		int choiceCalib;
		do
		{
			cin >> choiceCalib;
		} while (choiceCalib != 1 && choiceCalib != 2);
		if (choiceCalib == 1)
			CALIB = 1;	//是否读取标定信息
		else CALIB = 0;
	}


	//CALIB = 0;				//是否读取标定信息
	//读取标定信息
	if (CALIB)
	{
		//system("echo %~dp0");
		//system("dir");

		for (int i = 0; i < sucCamNum - 1; i++)	//读取标定信息
		{
			stringstream ss;
			ss << "camera" << setw(2) << setfill('0') << i + 1 << "_results.xml"; //.\camera01_results.xml - .\cameraNN_results.xml
			String filename = ss.str();
			Rect Roi[2];						//每一个相机的左右ROI
			Mat R, T, R1, R2, P1, P2, Q;	//相机参数	
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
				fs["Roi1"] >> Roi[0];
				fs["Roi2"] >> Roi[1];
				fs["cameraMatrix1"] >> tmpcameraMatrix;
				fs["distCoeffs1"] >> tmpdistCoeffs;
				cameraMatrix.push_back(tmpcameraMatrix);
				distCoeffs.push_back(tmpdistCoeffs);
				fs["R1"] >> R1;
				fs["P1"] >> P1;
				fs["cameraMatrix2"] >> tmpcameraMatrix;
				fs["distCoeffs2"] >> tmpdistCoeffs;
				cameraMatrix.push_back(tmpcameraMatrix);
				distCoeffs.push_back(tmpdistCoeffs);
				fs["R2"] >> R2;
				fs["P2"] >> P2;
				fs.release();
				QVect.push_back(Q);
				cameraMatrixVect.push_back(cameraMatrix);
				distCoeffsVect.push_back(distCoeffs);
				R1Vect.push_back(R1);
				P1Vect.push_back(P1);
				R2Vect.push_back(R2);
				P2Vect.push_back(P2);

				x1 = Roi[0].x;
				x2 = Roi[1].x;
				x3 = Roi[0].x + Roi[0].width;
				x4 = Roi[1].x + Roi[1].width;

				y1 = Roi[0].y;
				y2 = Roi[1].y;
				y3 = Roi[0].y + Roi[0].height;
				y4 = Roi[1].y + Roi[1].height;

				xx = max(x1, x2);
				yy = max(y1, y2);
				xx2 = min(x3, x4);
				yy2 = min(y3, y4);
				h = xx2 - xx;
				w = yy2 - yy;
				RoiVect.push_back(Rect(xx, yy, h, w));
			}
		}
	}


	/****************	创建文件夹	************/
	for (int i = 0; i < sucCamNum; i++)	//创建文件夹
	{
		stringstream ss1;
		char dirname1[255];
		ss1 << "camera" << setw(2) << setfill('0') << i + 1;
		ss1 >> dirname1;
		mkdir(dirname1);
		stringstream ss2;
		char dirname2[255];
		ss2 << "camera" << setw(2) << setfill('0') << i + 1 << "\\" << "left" << "\\";
		ss2 >> dirname2;
		mkdir(dirname2);
		stringstream ss3;
		char dirname3[255];
		ss3 << "camera" << setw(2) << setfill('0') << i + 1 << "\\" << "right" << "\\";
		ss3 >> dirname3;
		mkdir(dirname3);
	}

	cout << "是否画中心基准线" << endl;
	cout << "1 : 画  2 : 不画" << endl;
	int choiceCenterLine;
	do
	{
		cin >> choiceCenterLine;
	} while (choiceCenterLine != 1 && choiceCenterLine != 2);
	if (choiceCenterLine == 1)
		CENTER_LINE = 1;	//是否画中心基准线
	else CENTER_LINE = 0;



	/****************	开始拍照	************/
	frameSum = 0;
	for (int i = 0; i < sucCamNum - 1; i++){
		picLeftROI.push_back(sucImages[i]);					//初始化picLeftROI队列
		picRightROI.push_back(sucImages[i + 1]);

		vector<Mat> recImages;
		recImages.push_back(sucImages[i]);
		recImages.push_back(sucImages[i + 1]);

		recImagesVect.push_back(recImages);			//初始化recImagesVect队列
	}

	int p = ceil(sqrt(sucCamNum + 1));	//摄像机平方取天棚，+1为了预留命令行
	double scaleX, scaleY, scaleX_CALIB, scaleY_CALIB;
	scaleX = 0.95*min(1.0, SCREEN_WIDTH / CAP_WIDTH / p);		//未标定时画面缩小		
	scaleY = 0.95*min(1.0, SCREEN_HEIGHT / CAP_HEIGHT / p);		//未标定时画面缩小	

	bool show_frame = true;
	cout << "是否显示帧率" << endl;
	cout << "1 : 显示  2 : 不显示" << endl;
	int choiceShowFrame;
	do
	{
		cin >> choiceShowFrame;
	} while (choiceShowFrame != 1 && choiceShowFrame != 2);
	if (choiceShowFrame == 1)
		show_frame = true;	//是否画中心基准线
	else show_frame = false;

	RECT rect;
	HWND hwnd = GetForegroundWindow();
	GetWindowRect(hwnd, &rect);
	sucVCs[0] >> sucImages[0];
	MoveWindow(hwnd, ((sucCamNum) % p)*sucImages[0].cols * scaleX, ((sucCamNum) / p)*sucImages[0].rows * scaleY, sucImages[0].cols * scaleX, sucImages[0].rows * scaleY, TRUE);

	double timeStart = 0;					//一帧开始的时间
	double timeInit = getTickCount();		//拍照开始的时间
	double timeEnd = 0;						//一帧结束的时间
	double timeSub = 0;						//一帧的时间差
	double curFrame = 0;					//当前帧率
	double timeAll = 0;						//总时间
	double avgframe = 0;					//平均帧率
	while (1)
	{
		frameSum++;
		for (int i = 0; i < sucCamNum; i++)	//读取图像
		{
			sucVCs[i] >> sucImages[i];
		}

		if (CALIB)			//逐张图片校正
		{
			for (int i = 0; i < sucCamNum - 1; i++){
				//cout << " 开始校正 " << endl;
				//printf("正在校正第%d个摄像头第%d帧的图像！！\n", i, frameSum);
				picLeftROI[i] = sucImages[i];					//初始化picLeftROI队列
				picRightROI[i] = sucImages[i + 1];

				vector<Mat> recImages;
				recImages.push_back(sucImages[i]);
				recImages.push_back(sucImages[i + 1]);

				recImagesVect[i] = recImages;			//初始化recImagesVect队列
				//cout << " 图片已加入队列 " << endl;

				initUndistortRectifyMap(cameraMatrixVect[i][0], distCoeffsVect[i][0], R1Vect[i], P1Vect[i], picLeftROI[i].size(), CV_16SC2, rmap[0][0], rmap[0][1]);	//去畸变
				initUndistortRectifyMap(cameraMatrixVect[i][1], distCoeffsVect[i][1], R2Vect[i], P2Vect[i], picLeftROI[i].size(), CV_16SC2, rmap[1][0], rmap[1][1]);	//去畸变
				//cout << " 去畸变 " << endl;

				remap(picLeftROI[i], recImagesVect[i][0], rmap[0][0], rmap[0][1], CV_INTER_LINEAR);	//重映射
				remap(picRightROI[i], recImagesVect[i][1], rmap[1][0], rmap[1][1], CV_INTER_LINEAR);	//重映射
				//cout << " 重映射 " << endl;

				picLeftROI[i] = recImagesVect[i][0](RoiVect[i]);	//以左图的ROI为基准，取ROI
				picRightROI[i] = recImagesVect[i][1](RoiVect[i]);

			}
			if (CENTER_LINE){
				for (int i = 0; i < sucCamNum - 1; i++){
					line(picLeftROI[i], Point(0, RoiVect[i].height / 2), Point(RoiVect[i].width, RoiVect[i].height / 2), Scalar(100, 100, 100));
					line(picLeftROI[i], Point(RoiVect[i].width / 2, 0), Point(RoiVect[i].width / 2, RoiVect[i].height), Scalar(100, 100, 100));
					line(picRightROI[i], Point(0, RoiVect[i].height / 2), Point(RoiVect[i].width, RoiVect[i].height / 2), Scalar(100, 100, 100));
					line(picRightROI[i], Point(RoiVect[i].width / 2, 0), Point(RoiVect[i].width / 2, RoiVect[i].height), Scalar(100, 100, 100));

				}
			}
			press = waitKey(10);
			if (press == 32){	//未校正的图像在sucCamNum[i]中，校正后的图像为picLeftROI[i]、picRightROI[i]。
				picnumCount++;
				for (int i = 0; i < sucCamNum - 1; i++){
					stringstream ss;
					//curName = name + to_string(i + 1); //camera1
					ss << name << setw(2) << setfill('0') << i + 1 << "\\" << "left";					//camera01\left
					ss << "\\" << "left" << setw(2) << setfill('0') << picnumCount << ".png";	//camera01\left\left01.png
					//cout << ss.str() << endl;
					if (!imwrite(ss.str(), sucImages[i])){
						cout << ss.str() << endl;
						printf("第%d个相机第%d张照片保存失败！请检查相应文件夹是否存在！\n", i + 1, picnumCount);
						break;
					}
					cout << ss.str() << " 已保存" << endl;
					ss.str("");
					ss << name << setw(2) << setfill('0') << i + 2 << "\\" << "right";					//camera02\right
					ss << "\\" << "right" << setw(2) << setfill('0') << picnumCount << ".png";	//camera02\right\right01.png
					//cout << ss.str() << endl;
					if (!imwrite(ss.str(), sucImages[i])){
						cout << ss.str() << endl;
						printf("第%d个相机第%d张照片保存失败！请检查相应文件夹是否存在！\n", i + 1, picnumCount);
						break;
					}
					cout << ss.str() << " 已保存" << endl;
				}
			}
			else if (press == 27) break; //Esc  退出	
			for (int i = 0; i < sucCamNum - 1; i++){
				scaleX_CALIB = min(1.0, SCREEN_WIDTH / picLeftROI[i].cols / p);		//未标定时画面缩小		
				scaleY_CALIB = min(1.0, SCREEN_HEIGHT / picLeftROI[i].rows / p);		//未标定时画面缩小	
				resize(picLeftROI[i], picLeftROI[i], Size(), scaleX_CALIB, scaleY_CALIB);
				if (show_frame)
				{
					timeEnd = getTickCount();
					timeSub = (timeEnd - timeStart) / 2500000; //s
					timeStart = getTickCount();
					curFrame = 1 / timeSub;
					stringstream ss1;
					ss1 << "Current Frame: " << curFrame;
					putText(picLeftROI[i], ss1.str(), Point(0, 100), FONT_HERSHEY_DUPLEX, 0.6, CV_RGB(255, 0, 0), 1, 8, false); //Start text
					timeAll = (timeEnd - timeInit) / 2500000; //s
					avgframe = frameSum / timeAll;
					stringstream ss2;
					ss2 << "All Frame: " << avgframe;
					putText(picLeftROI[i], ss2.str(), Point(0, 70), FONT_HERSHEY_DUPLEX, 0.6, CV_RGB(255, 0, 0), 1, 8, false); //Start text
				}
				imshow(to_string(i), picLeftROI[i]);
				moveWindow(to_string(i), (i % p)*picLeftROI[i].cols, (i / p)*picLeftROI[i].rows);
			}
		}
		else		//不标定///////////////////////////////////////////////////////////////////////////////
		{
			if (CENTER_LINE){
				for (int i = 0; i < sucCamNum; i++){
					line(sucImages[i], Point(0, CAP_HEIGHT / 2), Point(CAP_WIDTH, CAP_HEIGHT / 2), Scalar(100, 100, 100));
					line(sucImages[i], Point(CAP_WIDTH / 2, 0), Point(CAP_WIDTH / 2, CAP_HEIGHT), Scalar(100, 100, 100));
				}
			}
			press = waitKey(10);
			if (press == 32){	//未校正的图像在sucCamNum[i]中，校正后的图像为picLeftROI[i]、picRightROI[i]。
				picnumCount++;
				for (int i = 0; i < sucCamNum; i++){
					stringstream ss;
					ss << name << setw(2) << setfill('0') << i + 1 << "\\" << "left";					//camera01\left
					ss << "\\" << "left" << setw(2) << setfill('0') << picnumCount << ".png";	//camera01\left\left01.png
					String ss1 = ss.str();
					if (!imwrite(ss1, sucImages[i])){
						cout << ss.str();
						printf("  第%d个相机第%d张照片保存失败！请检查相应文件夹是否存在！\n", i + 1, picnumCount);
						break;
					}
					cout << ss.str() << " 已保存" << endl;
				}
			}
			else if (press == 27) break; //Esc  退出	
			for (int i = 0; i < sucCamNum; i++){
				resize(sucImages[i], sucImages[i], Size(), scaleX, scaleY);
				//resize(sucImages[i + 1], sucImages[i + 1], Size(), scaleX, scaleY);
				if (show_frame)
				{
					timeEnd = getTickCount();
					timeSub = (timeEnd - timeStart) / 2500000; //s
					curFrame = 1 / timeSub;
					stringstream ss1;
					ss1 << "Current Frame: " << curFrame;
					putText(sucImages[i], ss1.str(), Point(0, 100), FONT_HERSHEY_DUPLEX, 0.6, CV_RGB(255, 0, 0), 1, 8, false); //Start text
					timeAll = (timeEnd - timeInit) / 2500000; //s
					avgframe = frameSum / timeAll;
					stringstream ss2;
					ss2 << "All Frame: " << avgframe;
					putText(sucImages[i], ss2.str(), Point(0, 70), FONT_HERSHEY_DUPLEX, 0.6, CV_RGB(255, 0, 0), 1, 8, false); //Start text
				}
				imshow(to_string(i), sucImages[i]);
				//imshow(to_string(i + 1), sucImages[i + 1]);
				moveWindow(to_string(i), (i % p)*sucImages[i].cols, (i / p)*sucImages[i].rows);
				//moveWindow(to_string(i + 1), ((i + 1) % p)*sucImages[i + 1].cols, ((i + 1) / p)*sucImages[i + 1].rows);
			}
			timeStart = getTickCount();
		}
		/*
		sucVCs[0] >> sucImages[0];
		sucVCs[1] >> sucImages[1];
		imshow("0", sucImages[0]);
		imshow("1", sucImages[1]);
		waitKey(10);*/
	}
	
}

/*   窗口摇摆Demo    */
void ShakeWindow(){
	int SHAKE = 5;
	RECT rect;
	HWND hwnd = GetForegroundWindow();
	GetWindowRect(hwnd, &rect);
	MoveWindow(hwnd, rect.left + SHAKE, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
	Sleep(28);
	MoveWindow(hwnd, rect.left + SHAKE, rect.top - SHAKE, rect.right - rect.left, rect.bottom - rect.top, TRUE);
	Sleep(28);
	MoveWindow(hwnd, rect.left, rect.top - SHAKE, rect.right - rect.left, rect.bottom - rect.top, TRUE);
	Sleep(28);
	MoveWindow(hwnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
}

/*   计时Demo    */
void showTime()
{
	double start = getTickCount();
	Sleep(1000);
	double end = getTickCount();
	double sub = end - start;
	cout << "Time " << sub / 2500000 << endl;
}

int main(){

	cout << "请输入相机数量" << endl;
	int cnum;
	cin >> cnum;
	takePictures(cnum, "camera");

	/*	while (1) {
			showTime();
			}
			while (1) {
			ShakeWindow();
			Sleep(20);
			}*/
	//stringstream ss;
	//ss << ".\\" << "testtmp";
	//String name = ss.str();
	//char *mkch = new char[255];		//用于建立文件夹的字符串
	//ss >> mkch;			//stringstream to char*
	//mkdir(mkch);
}