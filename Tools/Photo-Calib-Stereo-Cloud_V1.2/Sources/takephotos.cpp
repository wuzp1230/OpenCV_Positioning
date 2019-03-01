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
//#define ROI_WIDTH (ROI_HEIGHT / 5 * 7)													// �궨��ĳ����Ϊ7:5	
//#define ROI_WIDTH (ROI_HEIGHT / 9 * 16)													// �궨��ĳ����Ϊ7:5	
//#define ROI_WIDTH (ROI_HEIGHT / 7 * 6)													// �궨��ĳ����Ϊ7:5	
#define ROI_WIDTH (ROI_HEIGHT / 3 * 4)													// �궨��ĳ����Ϊ7:5	
//#define CALIB 1


//#define CAP_WIDTH 2592
//#define CAP_HEIGHT 1944

int MONO_CALIB = 0;
int CALIB = 0;				//�Ƿ��ȡ�궨��Ϣ
int CONTRAST_ENHANCE = 1;	//�Ƿ���ǿ�Աȶ�
int CENTER_LINE = 0;		//�Ƿ����Ļ�׼��
//#define CAP_WIDTH 1280
//#define CAP_HEIGHT 960
const int startID = 0;		//��һ������ı�ʶ��


//void mkdir(String dirname)
void mkdir(char* dirname)	//�ڵ�ǰĿ¼�½���һ������Ϊdirname���ļ���
{

	stringstream namestream;
	//system("echo %~dp0");
	//system("dir");
	namestream << ".\\" << dirname;
	//String name = ss.str();
	char *mkch = new char[255];		//���ڽ����ļ��е��ַ���
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
		cout << "�ѳɹ����� : " << namestream.str() << endl;
		//if (_rmdir(mkch) == 0) printf("Directory '\\testtmp' was successfully removed\n");
		//else printf("Problem removing directory '\\testtmp'\n");
	}
	else if (status == -1)
	{
		cout << namestream.str() << "�ļ����Ѵ��ڣ�����ʧ��" << endl;
	}

}
void logInfo(String s)
{
	cout << s << endl;
}

void takePictures(const int cnum, const string name)
{
	/*cout << "��ѡ��Ŀ���ջ��Ŀ����" << endl;
	cout << "1 : ��Ŀ����  2 : ��Ŀ����" << endl;
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
	//��Ŀ����
	if (MONO_CALIB)
	{
	}
	//��Ŀ����
	else
	{*/
		/*��i�����j��ͼƬ����·��Ϊ"name(0i)//left||right(0j)*/
	vector<VideoCapture> VCs;
	vector<Mat> images;

	vector<Mat>  image[3];
	Mat picLeftResize, picRightResize;	// ��Сͼ�񵽺��ʴ�С����ʾ	

	int press;				//������¼��
	int picnumCount = 0;	//ͼƬ���ռ�����

	bool isReversed = false;


	int MaxDetectNum = 30;	//������������
	int sucCamNum = 0;	//�ɹ���ȡ�����������
	vector<VideoCapture> sucVCs;	//�ɹ���ȡ�������׽������
	vector<Mat> sucImages;		//�ɹ���ȡ��ͼƬ����
	vector<int> sucCamSeq;

	cout << "�Ƿ���ǿ�Աȶ�" << endl;
	cout << "1 : ��ǿ  2 : ����ǿ" << endl;
	char choiceContrast;
	do
	{
		cin >> choiceContrast;
	} while (choiceContrast != '1' && choiceContrast != '2');
	if (choiceContrast == '1')
	{
		CONTRAST_ENHANCE = 1;	//�Ƿ���ǿ�Աȶ�
	}
	else CONTRAST_ENHANCE = 0;


#define CAP_WIDTH 1280
#define CAP_HEIGHT 720

	double SCREEN_WIDTH = 1600;
	double SCREEN_HEIGHT = 900;

	/**********				�������������			*****************/
	for (int i = 0; sucCamNum < cnum && i < MaxDetectNum; i++){		//�����������������Ƿ����
		printf("���ڼ���%d����Ƶ��׽����\n", i);
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
		VCs[i] >> images[i];	//�ȼ��һ��ͼ��	
		if (!images[i].data){
			printf("��%d����Ƶ��׽������ȡ��ͼ�񣡣�\n", i);
		}
		else{
			sucVCs.push_back(VideoCapture(startID + i));
			sucCamSeq.push_back(sucCamNum);
			sucCamNum++;
			printf("�ѳɹ�����%d����Ƶ��׽��������У�\n", sucCamNum);
			sucImages.push_back(Mat());
		}
	}
	printf("�ɹ�������%d��\n", sucCamNum);
	printf("�ɹ����й�ģ��%d��\n", sucVCs.size());
	if (sucCamNum == 0)	//δ��⵽���˳�
	{
		printf("δ��⵽��Ƶ��׽��,����ȡ��ͼ�񣡣�\n");
		waitKey(0);
		return;
	}


	char seqChoice;
	cout << "				��ѡ�����˳��ѡ��" << endl;
	cout << "1.����˳�򲢱���	2.�������ļ��ж�ȡ˳��	3.����ȡҲ������" << endl;
	do{
		cin >> seqChoice;
	} while (seqChoice <'1' || seqChoice > '3');

	long long int frameSum = 0;
	bool showSwapInfo = true;
	int swappingCamNum = 0;	//Ҫ�Ե���������豸�ţ���
	int swappingWinNum = 0;	//Ҫ�Ե��������Ŀ�괰�ںţ�С��
	bool isConfirmed = false;			//�ٴ�ȷ�ϵĿ���
	stringstream camNum;	//���������������
	VideoCapture tempVC;

	/**	1.����˳�򲢱���  2.�������ļ��ж�ȡ˳��   3.����ȡҲ������	****/
	if (seqChoice == '1')		//1.����˳�򲢱���
	{
		/****************		�ı����˳��	************/
		while (1)
		{
			if (showSwapInfo)
			{
				cout << "������� " << swappingWinNum << " �������ǰ���ڵĴ��ڱ�ţ����س���ȷ����" << endl;
				showSwapInfo = false;
			}
			for (int i = 0; i < sucCamNum; i++)	//��ȡͼ��
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
			else if (press == 13)			//�س�13	�ո�32  
			{
				camNum >> swappingCamNum;
				camNum.clear();
				if (!isConfirmed)				//δȷ��
				{
					if (swappingCamNum >= sucCamNum)	//����ų��磬���Ϸ�
					{
						cout << "����ų��磬����������" << endl;
						isConfirmed = false;
					}
					else
					{
						cout << "ȷ�Ͻ��� " << swappingCamNum << " ������Ļ���������" << swappingWinNum << "��������" << endl;
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
					if (swappingWinNum == sucCamNum)	//ȫ���Ե���ϣ���ʼ�����ļ�
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
	else if (seqChoice == '2')		//	2.�������ļ��ж�ȡ˳��	
	{

		char filename[] = "camera_seq.xml";
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
	else if (seqChoice == '3')		//	3.����ȡҲ������
	{

	}

	/****************	��ȡ�궨��Ϣ	************/
	Vector<Vector<Mat>> recImagesVect;		//У����ͼ�������	[N][2]
	Mat rmap[2][2];					//ÿ���������ͶӰ����
	//Vector<Mat[2][2]> rmapVect;				//��ͶӰ��������
	int x1, x2, x3, x4, y1, y2, y3, y4, xx = 0, yy = 0, xx2, yy2, h = CAP_HEIGHT, w = CAP_WIDTH;	//����Ȥ��������
	Vector<Vector<Mat>> cameraMatrixVect, distCoeffsVect;		//У����ͼ�������	[N][2]
	Vector<Mat> RVect, TVect, R1Vect, R2Vect, P1Vect, P2Vect, QVect;	//�������	[N]
	Vector<Rect> RoiVect;				//���ROI����
	Vector<Mat> picLeft, picRight;				// ����һ��Mat���������ڴ洢ÿһ֡��ͼ��
	Vector<Mat> picLeftROI, picRightROI;			//ȡ������Ȥ�����ͼ��
	vector<Mat> picROI;				//ȡ������Ȥ�����ͼ��

	destroyAllWindows();
	//cout << sucImages[i].size() << endl;
	if (sucCamNum != 1)
	{
		cout << "�Ƿ��ȡ�궨��Ϣ" << endl;
		cout << "1 : ��ȡ  2 : ����ȡ" << endl;
		int choiceCalib;
		do
		{
			cin >> choiceCalib;
		} while (choiceCalib != 1 && choiceCalib != 2);
		if (choiceCalib == 1)
			CALIB = 1;	//�Ƿ��ȡ�궨��Ϣ
		else CALIB = 0;
	}


	//CALIB = 0;				//�Ƿ��ȡ�궨��Ϣ
	//��ȡ�궨��Ϣ
	if (CALIB)
	{
		//system("echo %~dp0");
		//system("dir");

		for (int i = 0; i < sucCamNum - 1; i++)	//��ȡ�궨��Ϣ
		{
			stringstream ss;
			ss << "camera" << setw(2) << setfill('0') << i + 1 << "_results.xml"; //.\camera01_results.xml - .\cameraNN_results.xml
			String filename = ss.str();
			Rect Roi[2];						//ÿһ�����������ROI
			Mat R, T, R1, R2, P1, P2, Q;	//�������	
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


	/****************	�����ļ���	************/
	for (int i = 0; i < sucCamNum; i++)	//�����ļ���
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

	cout << "�Ƿ����Ļ�׼��" << endl;
	cout << "1 : ��  2 : ����" << endl;
	int choiceCenterLine;
	do
	{
		cin >> choiceCenterLine;
	} while (choiceCenterLine != 1 && choiceCenterLine != 2);
	if (choiceCenterLine == 1)
		CENTER_LINE = 1;	//�Ƿ����Ļ�׼��
	else CENTER_LINE = 0;



	/****************	��ʼ����	************/
	frameSum = 0;
	for (int i = 0; i < sucCamNum - 1; i++){
		picLeftROI.push_back(sucImages[i]);					//��ʼ��picLeftROI����
		picRightROI.push_back(sucImages[i + 1]);

		vector<Mat> recImages;
		recImages.push_back(sucImages[i]);
		recImages.push_back(sucImages[i + 1]);

		recImagesVect.push_back(recImages);			//��ʼ��recImagesVect����
	}

	int p = ceil(sqrt(sucCamNum + 1));	//�����ƽ��ȡ���+1Ϊ��Ԥ��������
	double scaleX, scaleY, scaleX_CALIB, scaleY_CALIB;
	scaleX = 0.95*min(1.0, SCREEN_WIDTH / CAP_WIDTH / p);		//δ�궨ʱ������С		
	scaleY = 0.95*min(1.0, SCREEN_HEIGHT / CAP_HEIGHT / p);		//δ�궨ʱ������С	

	bool show_frame = true;
	cout << "�Ƿ���ʾ֡��" << endl;
	cout << "1 : ��ʾ  2 : ����ʾ" << endl;
	int choiceShowFrame;
	do
	{
		cin >> choiceShowFrame;
	} while (choiceShowFrame != 1 && choiceShowFrame != 2);
	if (choiceShowFrame == 1)
		show_frame = true;	//�Ƿ����Ļ�׼��
	else show_frame = false;

	RECT rect;
	HWND hwnd = GetForegroundWindow();
	GetWindowRect(hwnd, &rect);
	sucVCs[0] >> sucImages[0];
	MoveWindow(hwnd, ((sucCamNum) % p)*sucImages[0].cols * scaleX, ((sucCamNum) / p)*sucImages[0].rows * scaleY, sucImages[0].cols * scaleX, sucImages[0].rows * scaleY, TRUE);

	double timeStart = 0;					//һ֡��ʼ��ʱ��
	double timeInit = getTickCount();		//���տ�ʼ��ʱ��
	double timeEnd = 0;						//һ֡������ʱ��
	double timeSub = 0;						//һ֡��ʱ���
	double curFrame = 0;					//��ǰ֡��
	double timeAll = 0;						//��ʱ��
	double avgframe = 0;					//ƽ��֡��
	while (1)
	{
		frameSum++;
		for (int i = 0; i < sucCamNum; i++)	//��ȡͼ��
		{
			sucVCs[i] >> sucImages[i];
		}

		if (CALIB)			//����ͼƬУ��
		{
			for (int i = 0; i < sucCamNum - 1; i++){
				//cout << " ��ʼУ�� " << endl;
				//printf("����У����%d������ͷ��%d֡��ͼ�񣡣�\n", i, frameSum);
				picLeftROI[i] = sucImages[i];					//��ʼ��picLeftROI����
				picRightROI[i] = sucImages[i + 1];

				vector<Mat> recImages;
				recImages.push_back(sucImages[i]);
				recImages.push_back(sucImages[i + 1]);

				recImagesVect[i] = recImages;			//��ʼ��recImagesVect����
				//cout << " ͼƬ�Ѽ������ " << endl;

				initUndistortRectifyMap(cameraMatrixVect[i][0], distCoeffsVect[i][0], R1Vect[i], P1Vect[i], picLeftROI[i].size(), CV_16SC2, rmap[0][0], rmap[0][1]);	//ȥ����
				initUndistortRectifyMap(cameraMatrixVect[i][1], distCoeffsVect[i][1], R2Vect[i], P2Vect[i], picLeftROI[i].size(), CV_16SC2, rmap[1][0], rmap[1][1]);	//ȥ����
				//cout << " ȥ���� " << endl;

				remap(picLeftROI[i], recImagesVect[i][0], rmap[0][0], rmap[0][1], CV_INTER_LINEAR);	//��ӳ��
				remap(picRightROI[i], recImagesVect[i][1], rmap[1][0], rmap[1][1], CV_INTER_LINEAR);	//��ӳ��
				//cout << " ��ӳ�� " << endl;

				picLeftROI[i] = recImagesVect[i][0](RoiVect[i]);	//����ͼ��ROIΪ��׼��ȡROI
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
			if (press == 32){	//δУ����ͼ����sucCamNum[i]�У�У�����ͼ��ΪpicLeftROI[i]��picRightROI[i]��
				picnumCount++;
				for (int i = 0; i < sucCamNum - 1; i++){
					stringstream ss;
					//curName = name + to_string(i + 1); //camera1
					ss << name << setw(2) << setfill('0') << i + 1 << "\\" << "left";					//camera01\left
					ss << "\\" << "left" << setw(2) << setfill('0') << picnumCount << ".png";	//camera01\left\left01.png
					//cout << ss.str() << endl;
					if (!imwrite(ss.str(), sucImages[i])){
						cout << ss.str() << endl;
						printf("��%d�������%d����Ƭ����ʧ�ܣ�������Ӧ�ļ����Ƿ���ڣ�\n", i + 1, picnumCount);
						break;
					}
					cout << ss.str() << " �ѱ���" << endl;
					ss.str("");
					ss << name << setw(2) << setfill('0') << i + 2 << "\\" << "right";					//camera02\right
					ss << "\\" << "right" << setw(2) << setfill('0') << picnumCount << ".png";	//camera02\right\right01.png
					//cout << ss.str() << endl;
					if (!imwrite(ss.str(), sucImages[i])){
						cout << ss.str() << endl;
						printf("��%d�������%d����Ƭ����ʧ�ܣ�������Ӧ�ļ����Ƿ���ڣ�\n", i + 1, picnumCount);
						break;
					}
					cout << ss.str() << " �ѱ���" << endl;
				}
			}
			else if (press == 27) break; //Esc  �˳�	
			for (int i = 0; i < sucCamNum - 1; i++){
				scaleX_CALIB = min(1.0, SCREEN_WIDTH / picLeftROI[i].cols / p);		//δ�궨ʱ������С		
				scaleY_CALIB = min(1.0, SCREEN_HEIGHT / picLeftROI[i].rows / p);		//δ�궨ʱ������С	
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
		else		//���궨///////////////////////////////////////////////////////////////////////////////
		{
			if (CENTER_LINE){
				for (int i = 0; i < sucCamNum; i++){
					line(sucImages[i], Point(0, CAP_HEIGHT / 2), Point(CAP_WIDTH, CAP_HEIGHT / 2), Scalar(100, 100, 100));
					line(sucImages[i], Point(CAP_WIDTH / 2, 0), Point(CAP_WIDTH / 2, CAP_HEIGHT), Scalar(100, 100, 100));
				}
			}
			press = waitKey(10);
			if (press == 32){	//δУ����ͼ����sucCamNum[i]�У�У�����ͼ��ΪpicLeftROI[i]��picRightROI[i]��
				picnumCount++;
				for (int i = 0; i < sucCamNum; i++){
					stringstream ss;
					ss << name << setw(2) << setfill('0') << i + 1 << "\\" << "left";					//camera01\left
					ss << "\\" << "left" << setw(2) << setfill('0') << picnumCount << ".png";	//camera01\left\left01.png
					String ss1 = ss.str();
					if (!imwrite(ss1, sucImages[i])){
						cout << ss.str();
						printf("  ��%d�������%d����Ƭ����ʧ�ܣ�������Ӧ�ļ����Ƿ���ڣ�\n", i + 1, picnumCount);
						break;
					}
					cout << ss.str() << " �ѱ���" << endl;
				}
			}
			else if (press == 27) break; //Esc  �˳�	
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

/*   ����ҡ��Demo    */
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

/*   ��ʱDemo    */
void showTime()
{
	double start = getTickCount();
	Sleep(1000);
	double end = getTickCount();
	double sub = end - start;
	cout << "Time " << sub / 2500000 << endl;
}

int main(){

	cout << "�������������" << endl;
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
	//char *mkch = new char[255];		//���ڽ����ļ��е��ַ���
	//ss >> mkch;			//stringstream to char*
	//mkdir(mkch);
}