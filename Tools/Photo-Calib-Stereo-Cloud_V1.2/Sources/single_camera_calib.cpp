#include "single_camera_calib.h"
#include "stereo_camera_calib.h"


extern void bug_info(string s){
	//bug信息处理
	cout << s << endl;
}


double single_calib(InputArrayOfArrays _images, OutputArray _cameraMatrix, OutputArray _distCoeffs, vector<Mat> &rvecs, vector<Mat> &tvecs, bool showProcess){
	/*
		单目标定

		_images:	输入图片
		_cameraMatrix：	输出内参矩阵
		_distCoeffs：	输出畸变向量
		rvecs：		输出旋转矩阵
		tvecs：		输出平移矩阵
		showProcess：	是否显示中间过程结果
	*/
	vector<Mat> images;
	_images.getMatVector(images);

	if (images.size() == 0) {
		bug_info("无输入图片!");
		return -1;
	}

	int realGridWidth = 0; int realGridHeight = 0;	//标定板真实格子大小
	//const int realGridWidth = 28.9;const int realGridHeight = 28.9;	//标定板真实格子大小
	//const int realGridWidth = 40;
	//const int realGridHeight = 40;	//标定板真实格子大小
	Size boardSize ;            /****    定标板上每行、列的角点数       ****/
	int boardWidth, boardHeight, squareSize;
	cout << "请输入标定板横向角点个数（多的）" << endl;
	cin >> boardWidth;
	cout << "请输入标定板纵向角点个数（少的）" << endl;
	cin >> boardHeight;
	cout << "请输入标定板角点间距大小" << endl;
	cin >> realGridWidth;
	realGridHeight = realGridWidth;
	boardSize = Size(boardWidth, boardHeight);	//opencv标准标定板

	//构造虚拟角点三维坐标
	vector<Point3f> singleObjectSet;
	for (int i = 0; i < boardSize.height; i++)
	{
		for (int j = 0; j < boardSize.width; j++)
		{
			// 假设定标板放在世界坐标系中z=0的平面上
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
		//转换成灰度图
		if (images[i].channels() == 3) cvtColor(images[i], imageGray, CV_RGB2GRAY);  
		else images[i].copyTo(imageGray);

		bool patternfound = findChessboardCorners(imageGray, boardSize, corners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE 
			+CALIB_CB_FAST_CHECK);
		if (!patternfound) {
			if (showProcess) printf("第%d张图片未能成功找到角点\n", i + 1);
			continue;
		}
		cornerSubPix(imageGray, corners, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
		if (corners.size() == boardSize.area()){
			if (showProcess){
				//输出角点图像
				drawChessboardCorners(images[i], boardSize, corners, patternfound);
				Mat tmpimage;
				resize(images[i], tmpimage, Size(), 0.5, 0.5);
				destroyWindow(to_string(i-1));
				cv::imshow("Images", tmpimage);
				waitKey(10);
			}
			//每成功对一张图片进行角点查找，就添加一个虚拟三维平面
			cornerPoints.push_back(corners);
			objectPoints.push_back(singleObjectSet);
		}
		else{
			if (showProcess) printf("第%d张图片未能成功找到角点\n", i + 1);
			continue;
		}
	}
	if (cornerPoints.size() == 0){
		bug_info("合法输入图片过少！");
		return -1;
	}
	Mat cameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0));                // 摄像机内参数矩阵
	Mat distCoeffs = Mat(1, 5, CV_32FC1, Scalar::all(0));				   // 摄像机的5个畸变系数 

	Size imageSize = images[0].size();
	double real_err = calibrateCamera(objectPoints, cornerPoints, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs, 0);

	_cameraMatrix.create(cameraMatrix.size(), cameraMatrix.type());
	cameraMatrix.copyTo(_cameraMatrix);
	_distCoeffs.create(distCoeffs.size(), distCoeffs.type());
	distCoeffs.copyTo(_distCoeffs);
	return real_err;
}


