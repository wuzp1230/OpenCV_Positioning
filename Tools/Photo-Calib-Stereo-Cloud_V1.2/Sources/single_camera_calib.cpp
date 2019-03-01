#include "single_camera_calib.h"
#include "stereo_camera_calib.h"


extern void bug_info(string s){
	//bug��Ϣ����
	cout << s << endl;
}


double single_calib(InputArrayOfArrays _images, OutputArray _cameraMatrix, OutputArray _distCoeffs, vector<Mat> &rvecs, vector<Mat> &tvecs, bool showProcess){
	/*
		��Ŀ�궨

		_images:	����ͼƬ
		_cameraMatrix��	����ڲξ���
		_distCoeffs��	�����������
		rvecs��		�����ת����
		tvecs��		���ƽ�ƾ���
		showProcess��	�Ƿ���ʾ�м���̽��
	*/
	vector<Mat> images;
	_images.getMatVector(images);

	if (images.size() == 0) {
		bug_info("������ͼƬ!");
		return -1;
	}

	int realGridWidth = 0; int realGridHeight = 0;	//�궨����ʵ���Ӵ�С
	//const int realGridWidth = 28.9;const int realGridHeight = 28.9;	//�궨����ʵ���Ӵ�С
	//const int realGridWidth = 40;
	//const int realGridHeight = 40;	//�궨����ʵ���Ӵ�С
	Size boardSize ;            /****    �������ÿ�С��еĽǵ���       ****/
	int boardWidth, boardHeight, squareSize;
	cout << "������궨�����ǵ��������ģ�" << endl;
	cin >> boardWidth;
	cout << "������궨������ǵ�������ٵģ�" << endl;
	cin >> boardHeight;
	cout << "������궨��ǵ����С" << endl;
	cin >> realGridWidth;
	realGridHeight = realGridWidth;
	boardSize = Size(boardWidth, boardHeight);	//opencv��׼�궨��

	//��������ǵ���ά����
	vector<Point3f> singleObjectSet;
	for (int i = 0; i < boardSize.height; i++)
	{
		for (int j = 0; j < boardSize.width; j++)
		{
			// ���趨��������������ϵ��z=0��ƽ����
			Point3f tempPoint;
			tempPoint.x = i * realGridWidth;
			tempPoint.y = j * realGridHeight;
			tempPoint.z = 0;
			singleObjectSet.push_back(tempPoint);
		}
	}

	int imagesNum = images.size();
	vector<vector<Point2f>>  cornerPoints;
	vector<vector<Point3f>>  objectPoints;
	vector<Point2f> corners;


	for (int i = 0; i < imagesNum; i++){
		Mat imageGray;
		//ת���ɻҶ�ͼ
		if (images[i].channels() == 3) cvtColor(images[i], imageGray, CV_RGB2GRAY);  
		else images[i].copyTo(imageGray);

		bool patternfound = findChessboardCorners(imageGray, boardSize, corners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE 
			+CALIB_CB_FAST_CHECK);
		if (!patternfound) {
			if (showProcess) printf("��%d��ͼƬδ�ܳɹ��ҵ��ǵ�\n", i + 1);
			continue;
		}
		cornerSubPix(imageGray, corners, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
		if (corners.size() == boardSize.area()){
			if (showProcess){
				//����ǵ�ͼ��
				drawChessboardCorners(images[i], boardSize, corners, patternfound);
				Mat tmpimage;
				resize(images[i], tmpimage, Size(), 0.5, 0.5);
				destroyWindow(to_string(i-1));
				cv::imshow("Images", tmpimage);
				waitKey(10);
			}
			//ÿ�ɹ���һ��ͼƬ���нǵ���ң������һ��������άƽ��
			cornerPoints.push_back(corners);
			objectPoints.push_back(singleObjectSet);
		}
		else{
			if (showProcess) printf("��%d��ͼƬδ�ܳɹ��ҵ��ǵ�\n", i + 1);
			continue;
		}
	}
	if (cornerPoints.size() == 0){
		bug_info("�Ϸ�����ͼƬ���٣�");
		return -1;
	}
	Mat cameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0));                // ������ڲ�������
	Mat distCoeffs = Mat(1, 5, CV_32FC1, Scalar::all(0));				   // �������5������ϵ�� 

	Size imageSize = images[0].size();
	double real_err = calibrateCamera(objectPoints, cornerPoints, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs, 0);

	_cameraMatrix.create(cameraMatrix.size(), cameraMatrix.type());
	cameraMatrix.copyTo(_cameraMatrix);
	_distCoeffs.create(distCoeffs.size(), distCoeffs.type());
	distCoeffs.copyTo(_distCoeffs);
	return real_err;
}


