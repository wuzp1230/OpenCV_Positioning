  //#include "stdafx.h"
#include "stereo_camera_calib.h"


using namespace cv;
using namespace std;

//Size boardSize = Size(9, 6);	//opencv标准标定板
//double squareSize = 28.9;

//const Size boardSize = Size(7, 6);	//棋盘标定板
//const double squareSize = 40.f;

int chess_or_grid = 0;
int boardWidth = 9;
int boardHeight = 6;
double squareSize;
char bugInfo[200];
Size boardSize;// = Size(9, 6);


static string type2str(int type) {
	string r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth) {
	case CV_8U:  r = "8U"; break;
	case CV_8S:  r = "8S"; break;
	case CV_16U: r = "16U"; break;
	case CV_16S: r = "16S"; break;
	case CV_32S: r = "32S"; break;
	case CV_32F: r = "32F"; break;
	case CV_64F: r = "64F"; break;
	default:     r = "User"; break;
	}

	r += "C";
	r += (chans + '0');

	return r;
}

double stereo_calib(const vector<Mat> images[2], vector<Mat> &_cameraMatrix, vector<Mat> &_distCoeffs, Mat &_R, Mat &_T, Mat &_R1, Mat &_R2, Mat &_P1, Mat &_P2, Mat &_Q, vector<Rect> &ROI, bool showProcess = 0){
	vector<vector<Point2f> > cornerPoints[2];
	vector<vector<Point3f> > objectPoints;
	Size imageSize;

	int i, j, k, nimages = (int)images[0].size();
	if (images[1].size() != nimages){
		bug_info("双目图像数量不同!!\n");
		return -1;
	}
	cornerPoints[0].resize(nimages);
	cornerPoints[1].resize(nimages);

	bool matched = true;
	vector<Point2f> curCornerPoints[2];		
	int matchedCount = 0;

	do
	{
		cout << "请选择标定板类型：" << endl;
		cout << "1.棋盘板" << endl;
		cout << "2.圆点板" << endl;
		cin >> chess_or_grid;
	} while (chess_or_grid != 1 && chess_or_grid != 2);

	cout << "请输入标定板横向角点个数（多的）" << endl;
	cin >> boardWidth;
	cout << "请输入标定板纵向角点个数（少的）" << endl;
	cin >> boardHeight;
	cout << "请输入标定板角点间距大小" << endl;
	cin >> squareSize;
	boardSize = Size(boardWidth, boardHeight);	//opencv标准标定板



	if (!images[0][0].empty()) imageSize = images[0][0].size();

	//遍历每张图片并寻找角点
	for (i = 0; i < nimages; i++){
		matched = true;
		for (k = 0; k < 2; k++)
		{
			Mat img = images[k][i];
			if (img.empty() || img.size() != imageSize){
				sprintf_s(bugInfo,"第%d个相机第%d张图片分辨率不同!!\n", k, i);
				matched = false;
				bug_info(bugInfo);
				break;
			}
		
			bool found = false;
			for (int scale = 1; scale <= 2; scale++)
			{
				Mat timg;
				if (scale == 1)
					timg = img;
				else
					resize(img, timg, Size(), scale, scale);
				//cout << "boardSize" << boardSize << endl;
				if (chess_or_grid == 1)
				{
					//cout << "Chessboard" << endl;
					found = findChessboardCorners(timg, boardSize, curCornerPoints[k],
						CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
				}
				else if (chess_or_grid == 2)
				{
					//cout << "Grid" << endl;
					found = findCirclesGrid(timg, boardSize, curCornerPoints[k],
					CALIB_CB_SYMMETRIC_GRID);// +CALIB_CB_CLUSTERING);
				}
				if (found)
				{
					if (scale > 1)
					{
						Mat cornersMat(curCornerPoints[k]);
						cornersMat *= 1. / scale;
					}
					break;
				}
			}
			if (!found){
				sprintf_s(bugInfo, "第%d个相机第%d张图片未找到角点!!\n", k+1, i+1);
				matched = false;
				bug_info(bugInfo);
				break;
			}
			if (showProcess)
			{
				Mat cimg, cimg1;
				img.copyTo(cimg);
				drawChessboardCorners(cimg, boardSize, curCornerPoints[k], found);
				double sf = 640. / MAX(img.rows, img.cols);
				resize(cimg, cimg1, Size(), sf, sf);
				imshow("corners", cimg1);
				waitKey(10);
			}
			
			Mat timg;
			if (img.channels() == 3) cvtColor(img, timg, COLOR_BGR2GRAY);
			else img.copyTo(timg);
			cornerSubPix(timg, curCornerPoints[k], Size(11, 11), Size(-1, -1),
				TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS,
				30, 0.01));
		}
		if (matched) {
			//两张图片都成功找到角点，则添加入cornerPoints中
			for (int t = 0; t < 2; t++){
				cornerPoints[t][matchedCount] = curCornerPoints[t];
			}
			matchedCount++;
		}

		else {
			sprintf_s(bugInfo, "第%d对图片未能成功匹配!!\n", i+1);
			bug_info(bugInfo);
		}
	}
	destroyWindow("corners");
	if (showProcess) cout << "成功匹配图片数量为" + to_string(matchedCount) + "对！"<< endl;
	
	if (matchedCount < 2)
	{
		sprintf_s(bugInfo, "成功匹配图片对数数量过少！！");
		bug_info(bugInfo);
		return -1;
	}

	cornerPoints[0].resize(matchedCount);
	cornerPoints[1].resize(matchedCount);
	objectPoints.resize(matchedCount);

	for (i = 0; i < matchedCount; i++)
	{
		for (j = 0; j < boardSize.height; j++)
			for (k = 0; k < boardSize.width; k++)
				objectPoints[i].push_back(Point3f(k*squareSize, j*squareSize, 0));
	}

	if (showProcess) cout << "开始标定！！\n" << endl;

	Mat cameraMatrix[2], distCoeffs[2];
	cameraMatrix[0] = Mat::eye(3, 3, CV_64F);
	cameraMatrix[1] = Mat::eye(3, 3, CV_64F);
	Mat R, T, E, F; //R.size() = (3,3)

	double err = stereoCalibrate(objectPoints, cornerPoints[0], cornerPoints[1],
		cameraMatrix[0], distCoeffs[0],
		cameraMatrix[1], distCoeffs[1],
		imageSize, R, T, E, F,

		/*TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 100, 1e-5),
		//CV_CALIB_FIX_ASPECT_RATIO 
		//+CV_CALIB_ZERO_TANGENT_DIST 
		CV_CALIB_SAME_FOCAL_LENGTH 
		+CV_CALIB_RATIONAL_MODEL 
		//+CV_CALIB_FIX_K3 + CV_CALIB_FIX_K4 + CV_CALIB_FIX_K5
		);*/	
		TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 100, 1e-5),
		CV_CALIB_FIX_ASPECT_RATIO 
		+CV_CALIB_ZERO_TANGENT_DIST 
		+CV_CALIB_SAME_FOCAL_LENGTH
		+ CV_CALIB_RATIONAL_MODEL
		+ CV_CALIB_FIX_K3 + CV_CALIB_FIX_K4 + CV_CALIB_FIX_K5 + CV_CALIB_FIX_K6
		);	

	_cameraMatrix.clear();
	_distCoeffs.clear();
	for (int i = 0; i < 2; i++){
		_cameraMatrix.push_back(cameraMatrix[i]);
		_distCoeffs.push_back(distCoeffs[i]);
	}

	Mat R1, R2, P1, P2, Q;
	R.copyTo(_R);
	T.copyTo(_T);
	Rect validRoi[2];

	if (showProcess) cout << "开始校正！！\n" << endl;
	stereoRectify(cameraMatrix[0], distCoeffs[0],
		cameraMatrix[1], distCoeffs[1],
		imageSize, R, T, R1, R2, P1, P2, Q,
		//CALIB_ZERO_DISPARITY,
		0,
		1, imageSize, &validRoi[0], &validRoi[1]);
	
	ROI.clear();
	for (int i = 0; i < 2; i++) ROI.push_back(validRoi[i]);

	R1.copyTo(_R1);
	R2.copyTo(_R2);
	P1.copyTo(_P1);
	P2.copyTo(_P2);
	Q.copyTo(_Q);
	if (showProcess) cout << "校正完成！！误差为" << err << endl;
	return err;
}


void saveStereoResults(const string fname, const double err, const vector<Mat> cameraMatrix, const vector<Mat> distCoeffs ,const Mat R, const Mat T, const Mat R1, const Mat R2, const Mat P1, const Mat P2, const Mat Q,const vector<Rect> Roi){
	
	FileStorage fs(fname, FileStorage::WRITE);
	if (!fs.isOpened()){
		bug_info(fname + "打开失败！！");
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
	for (int i = 0; i < cnum; i++){
		string s = "Roi" + to_string(i + 1);
		fs << s << Roi[i];
	}
	cout << "结果已保存至" << fname << endl;

	fs.release();
}

void loadStereoResults(const string fname, const int cnum, vector<Mat> &cameraMatrix, vector<Mat> &distCoeffs, Mat &R, Mat &T, Mat &R1, Mat &R2, Mat &P1, Mat &P2, Mat &Q, vector<Rect> &Roi){
	FileStorage fs(fname, FileStorage::READ);
	if (!fs.isOpened()){
		bug_info(fname + "打开失败！！");
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
	Roi.resize(2);
	for (int i = 0; i < cnum; i++){
		string s = "Roi" + to_string(i + 1);
		fs[s] >> Roi[i];
	}
	fs.release();

	cout << "已成功从" + fname + "读入数据" << endl;
}

double pp3d_distance(const double a[3], const double b[3]){
	double dist = 0;
	for (int i = 0; i < 3; i++){
		dist += (a[i] - b[i]) * (a[i] - b[i]);
	}
	return sqrt(dist);
}


//void lengthErrorMeasure(const Mat images[2], const Mat Q){
void lengthErrorMeasure(vector<Mat> images, const Mat Q, int num){
	cout << "开始通过Q矩阵估算两对角角点间距离" << endl;

	if (images[0].empty() || images[1].empty() || images[0].size() != images[1].size())
		bug_info("传入图片不合法！！");
	vector<Point2f>  cornerPoints[2];

	for (int i = 0; i < 2; i++){
		//imshow("aa", images[0]);
		//imshow("bb", images[1]);
		bool found = findChessboardCorners(images[i], boardSize, cornerPoints[i],
			CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);

		if (!found){
			sprintf_s(bugInfo, "第%d张图片寻找角点失败！", i+1);
			bug_info(bugInfo);
			return;
		}
	}

	int cornerNum = boardSize.area();

	double disp1[4] = { cornerPoints[0][0].x, cornerPoints[0][0].y, cornerPoints[0][0].x - cornerPoints[1][0].x, 1 };
	double disp2[4] = { cornerPoints[0][cornerNum-1].x, cornerPoints[0][cornerNum-1].y, cornerPoints[0][cornerNum-1].x - cornerPoints[1][cornerNum-1].x, 1 };


	Mat ulDisp = Mat(4, 1, CV_64FC1,disp1);
	Mat lrDisp = Mat(4, 1, CV_64FC1,disp2);
	
	Mat ul3d = Mat(4, 1, CV_64FC1);
	Mat lr3d = Mat(4, 1, CV_64FC1);

	ul3d = Q * ulDisp;
	lr3d = Q * lrDisp;

	double pA[3], pB[3];

	for (int i = 0; i < 3; i++){
		pA[i] = ul3d.at<double>(i, 0) / ul3d.at<double>(3, 0);
		pB[i] = lr3d.at<double>(i, 0) / lr3d.at<double>(3, 0);
	}

	double rA[3] = { 0, 0, 0 };
	double rB[3] = { (boardSize.width - 1) * squareSize, (boardSize.height - 1) * squareSize, 0 };
	// 计算A、B两点间的距离
	double measureLength = pp3d_distance(pA, pB);
	double realLength = pp3d_distance(rA, rB);

	printf("估计距离为%f,实际距离为%f，误差为%f\%\n", measureLength, realLength, fabs(realLength - measureLength) / realLength);

	Mat left = images[0].clone();
	Mat right = images[1].clone();
	Point2f leftA = cornerPoints[0][0], leftB = cornerPoints[0][cornerNum - 1];
	Point2f rightA = cornerPoints[1][0], rightB = cornerPoints[1][cornerNum - 1];

	line(left, leftA, leftB, Scalar(0, 0, 255), 2);
	circle(left, leftA, 4, Scalar(255, 0, 0), -1);
	putText(left, "A", leftA, 4, 1, Scalar(255, 0, 0));
	circle(left, leftB, 4, Scalar(0, 255, 0), -1);
	putText(left, "B", leftB, 4, 1, Scalar(0, 255, 0));

	line(right, rightA, rightB, Scalar(0, 0, 255), 2);
	circle(right, rightA, 4, Scalar(255, 0, 0), -1);
	putText(right, "A", rightA, 4, 1, Scalar(255, 0, 0));
	circle(right, rightB, 4, Scalar(0, 255, 0), -1);
	putText(right, "B", rightB, 4, 1, Scalar(0, 255, 0));
	stringstream ss1;
	ss1 << "pair " << num << " - left";
	stringstream ss2;
	ss2 << "pair " << num << " - right";
	imshow(ss1.str(), left);
	imshow(ss2.str(), right);
	waitKey(0);



}