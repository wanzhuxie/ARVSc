#include "MarkerRecognizer.h"
#include "gl/glut.h"
#include <qgl.h>
#include <fstream>
#include <time.h>
#include <io.h>
#include <QPoint>
#include <QDir>
#include <QTimer>
#include <iostream>  
#include <QWidget>
using std::cout;
using std::endl;
#include <stdio.h>
#include <sys/timeb.h>

#define ADAPTIVE_THRESH_SIZE 35
#define APPROX_POLY_EPS 0.08
#define marker55 1

const int giCellSize=10;
const int giCellCount=5;
const int giCellCountWithFrame=giCellCount+2;
const int giMarkerSize=giCellCountWithFrame*giCellSize;





/*
标记可根据实际需求确定，这里使用5行5列的黑白方块的组合作为标记，如图1所示，白色代表1，黑色代表0，事实上，标记内容即为编码信息，为便于识别，使用黑色边框包围标记内容
为消除标记的旋转对称性以及某一行全部黑色，保证矩阵信息的唯一，对标记定义如下
	标记的左上角必须为孤立白色方块，其他的3个角不可有孤立白色方块；
	最后一行及最后一列均有白色方块，最后一行的数值之和不可为0，最后一列的数值之和不可为0

将原始图像灰度化
祛除噪点
使用cv::findContours查找角点，得到点组
对于每个点组，判断是否为凸四边形，若不是，则跳过
使用cv::getPerspectiveTransform得到透视变换矩阵
使用cv::warpPerspective对图像透视变换并截取标记所在的区域
将图像二值化，使用cv::countNonZero并设置阈值判断方块的颜色，得到当前矩阵
对照标记的定义，计算当前标记的旋转状态
至此，得到了标记的4个角点及角点顺序
对角点亚像素精确
*/
//========================================Class Marker=====================================
Marker::Marker()
{
	m_id = -1;
	m_corners.resize(4, Point2f(0.f,0.f));
}

Marker::Marker(int _id, cv::Point2f _c0, cv::Point2f _c1, cv::Point2f _c2, cv::Point2f _c3)
{
	m_id = _id;

	m_corners.reserve(4);
	m_corners.push_back(_c0);
	m_corners.push_back(_c1);
	m_corners.push_back(_c2);
	m_corners.push_back(_c3);
}

void Marker::drawCorners(cv::Mat& image, cv::Scalar color, float thickness)
{
	circle(image, m_corners[0], thickness*2, color, thickness);
	circle(image, m_corners[1], thickness*2, color, thickness);
	circle(image, m_corners[2], thickness*2, color, thickness);
	circle(image, m_corners[3], thickness*2, color, thickness);
	line(image, m_corners[0], m_corners[1], color, thickness, CV_AA);
    line(image, m_corners[1], m_corners[2], color, thickness, CV_AA);
    line(image, m_corners[2], m_corners[3], color, thickness, CV_AA);
    line(image, m_corners[3], m_corners[0], color, thickness, CV_AA);
	
	//Point text_point = m_corners[0] + m_corners[2];
	//text_point.x /= 2;
	//text_point.y /= 2;

	//stringstream ss;
	//ss << m_id;

	//putText(image, ss.str(), text_point, FONT_HERSHEY_SIMPLEX, 0.5, color);
}

void Marker::estimateTransformToCamera(vector<Point3f> corners_3d, cv::Mat& camera_matrix, cv::Mat& dist_coeff, cv::Mat& rmat, cv::Mat& tvec)
{
	Mat rot_vec;
	bool res = solvePnP(corners_3d,		//i世界坐标系下的控制点的坐标
		m_corners,									//i图像坐标系下对应的控制点的坐标
		camera_matrix,								//i相机内参
		dist_coeff,									//i相机畸变
		rot_vec,										//o旋转向量
		tvec);											//o平移向量

	Rodrigues(rot_vec, rmat);				//旋转向量变为旋转矩阵
}

string GetSysTime_string()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	std::stringstream ss;

	ss<<st.wYear;
	ss<<st.wMonth;
	ss<<st.wDay;
	ss<<st.wHour;
	ss<<st.wMinute;
	ss<<st.wSecond;
	ss<<st.wMilliseconds;

	string strTime=	ss.str();
	//cout<<strTime<<endl;

	return strTime;
}
int GetSysTime_number()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	int iTime=st.wHour*60*60*1000+st.wMinute*60*1000+st.wSecond*1000+st.wMilliseconds;
	return iTime;
}


//====================================Class MarkerRecognizer================================
MarkerRecognizer::MarkerRecognizer()
{
	//标准Marker坐标，逆时针
	m_marker_coords.push_back(Point2f(0,0));
	m_marker_coords.push_back(Point2f(0, giMarkerSize-1));
	m_marker_coords.push_back(Point2f(giMarkerSize-1, giMarkerSize-1));
	m_marker_coords.push_back(Point2f(giMarkerSize-1, 0));
}

int MarkerRecognizer::update(Mat& image, int min_size, int min_side_length)
{
	_picFileDir="";

	CV_Assert(!image.empty());
	//CV_Assert(image.type() == CV_8UC1);

	int iStartTime=GetSysTime_number();

	Mat image_ori;
	Mat image_gray;
	Mat image_adaptiveThreshold;
	Mat image_morphologyEx;
	Mat image_marker_gray;
	Mat image_marker_threshold;
	Mat image_drawCorners;
	
	image_ori=image;
	image_drawCorners = image_ori.clone();

	cvtColor( image, image_gray, CV_BGR2GRAY );
	//equalizeHist(image_gray, image_gray);

	//找可能的标记
	vector<Marker> possible_markers;
	{
		Mat img_Threshold;
		Mat img_Open;
		Mat img_Close;
		Mat img_erode;

		int thresh_size = (min_size/4)*2 + 1;
		//将可变（自适应）阈值应用于图像
		adaptiveThreshold(image_gray, image_adaptiveThreshold, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV , thresh_size, thresh_size*0.5);

		//对图像应用高级形态学操作(开运算)
		morphologyEx(image_adaptiveThreshold, image_morphologyEx, MORPH_OPEN, Mat());	//use open operator to eliminate small patch

		//查找角点
		vector<vector<Point>> all_contours;
		vector<vector<Point>> contours;
		findContours(image_morphologyEx, all_contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

		//检查每个轮廓点的数量，需要大于一定的值100
		for (int i = 0; i < all_contours.size(); ++i)
		{
			if (all_contours[i].size() > min_size)
			{
				contours.push_back(all_contours[i]);
			}
		}

		vector<Point> approx_poly;
		for (int i = 0; i < contours.size(); ++i)
		{
			double eps = contours[i].size()*APPROX_POLY_EPS;
			approxPolyDP(contours[i], //输入的点集
				approx_poly, //输出点集
				eps, //精度，原始曲线与近似曲线之间的最大距离
				true);//闭合

			if (approx_poly.size() != 4)
				continue;
			//检查是否凸包 opencv自带
			if (!isContourConvex(approx_poly))
				continue;

			//四边形各顶点之间的最短距离
			float min_side = FLT_MAX;
			for (int j = 0; j < 4; ++j)
			{
				Point side = approx_poly[j] - approx_poly[(j+1)%4];
				min_side = min(min_size, side.dot(side));
			}
			if (min_side < min_side_length*min_side_length)
				continue;

			//Sort the points in anti-clockwise
			Marker marker = Marker(0, approx_poly[0], approx_poly[1], approx_poly[2], approx_poly[3]);
			Point2f v1 = marker.m_corners[1] - marker.m_corners[0];
			Point2f v2 = marker.m_corners[2] - marker.m_corners[0];
			if (v1.cross(v2) < 0)	//大于零才代表逆时针
			{
				swap(marker.m_corners[1], marker.m_corners[3]);
			}
			possible_markers.push_back(marker);
		}

	}

	//透视变换检查是否真的是标记
	m_markers.clear();
	Mat bit_matrix(giCellCount, giCellCount, CV_8UC1);
	for (int i = 0; i < possible_markers.size(); ++i)
	{
		//返回3x3透视变换矩阵	//源图像四个顶点坐标//目标图像四个顶点坐标
		Mat M = cv::getPerspectiveTransform(possible_markers[i].m_corners, m_marker_coords);

		//透视变换 由变换矩阵后的图像获取平面图像，输出图像的区域由size控制
		warpPerspective(image_gray, //i
			image_marker_gray, //o
			M, //matrix
			Size(giMarkerSize, giMarkerSize));

		cv::threshold(image_marker_gray, image_marker_threshold, 125, 255, THRESH_BINARY|THRESH_OTSU); //OTSU 大律二值化

		//黑框
		for (int y = 0; y < giCellCountWithFrame; ++y)
		{
			//第一列及最后一列7行均黑，其他列只有第1行及第7行黑色，所以取cell像素时第一列及最后一列单步跳跃，其他从第一行跳到最后一行
			int iJump = (y == 0 || y == giCellCountWithFrame-1) ? 1 : giCellCountWithFrame-1;
			int cell_y = y*giCellSize;

			for (int x = 0; x < giCellCountWithFrame; x += iJump)
			{
				int cell_x = x*giCellSize;
				int none_zero_count = countNonZero(image_marker_threshold(Rect(cell_x, cell_y, giCellSize, giCellSize)));
				//此时灰度不为0像素应该giCellSize*giCellSize，考虑反光和误差，/4
				if (none_zero_count > giCellSize*giCellSize/4)
					goto __wrongMarker;
			}
		}

		//Decode the marker
		for (int y = 0; y < giCellCount; ++y)
		{
			//要排除边框，所以要x+1 y+1
			int cell_y = (y+1)*giCellSize;

			for (int x = 0; x < giCellCount; ++x)
			{
				int cell_x = (x+1)*giCellSize;
				int none_zero_count = countNonZero(image_marker_threshold(Rect(cell_x, cell_y, giCellSize, giCellSize)));
				if (none_zero_count > giCellSize*giCellSize/2)
					bit_matrix.at<uchar>(y, x) = 1;
				else
					bit_matrix.at<uchar>(y, x) = 0;
			}
		}

		bool good_marker = false;
		int rotation_idx=0;	//逆时针旋转的次数
		//使用hammingmarker or marker55
#ifdef marker55
		//左上
		if (bit_matrix.at<uchar>(0, 0)==1 &&bit_matrix.at<uchar>(0, 1)==0 &&bit_matrix.at<uchar>(1, 0)==0 &&bit_matrix.at<uchar>(1, 1)==0)
		{
			rotation_idx = 0;
			good_marker = true;
		}
		//左下
		else	if (bit_matrix.at<uchar>(giCellCount-1, 0)==1 &&bit_matrix.at<uchar>(giCellCount-2, 0)==0 &&bit_matrix.at<uchar>(giCellCount-1, 1)==0 &&bit_matrix.at<uchar>(giCellCount-2, 1)==0)
		{
			rotation_idx = 1;
			good_marker = true;
		}
		//右下
		else	if (bit_matrix.at<uchar>(giCellCount-1, giCellCount-1)==1 &&bit_matrix.at<uchar>(giCellCount-2, giCellCount-1)==0 &&bit_matrix.at<uchar>(giCellCount-1, giCellCount-2)==0 &&bit_matrix.at<uchar>(giCellCount-2, giCellCount-2)==0)
		{
			rotation_idx = 2;
			good_marker = true;
		}
		//右上
		else	if (bit_matrix.at<uchar>(0, giCellCount-1)==1 &&bit_matrix.at<uchar>(0, giCellCount-2)==0 &&bit_matrix.at<uchar>(1, giCellCount-2)==0 &&bit_matrix.at<uchar>(1, giCellCount-1)==0)
		{
			rotation_idx = 3;
			good_marker = true;
		}

		if (!good_marker) goto __wrongMarker;

		//Store the final marker
		Marker& final_marker = possible_markers[i];
		std::rotate(final_marker.m_corners.begin(), final_marker.m_corners.begin() + rotation_idx, final_marker.m_corners.end());

		m_markers.push_back(final_marker);
		drawCorners(image_drawCorners , cv::Scalar(255,0,0) , 1);
#else
		for (rotation_idx = 0; rotation_idx < 4; ++rotation_idx)
		{
			if (hammingDistance(bit_matrix) == 0)
			{
				good_marker = true;
				break;
			}
			bit_matrix = bitMatrixRotate(bit_matrix);
		}

		if (!good_marker) goto __wrongMarker;

		//Store the final marker
		Marker& final_marker = possible_markers[i];

		final_marker.m_id = bitMatrixToId(bit_matrix);
		std::rotate(final_marker.m_corners.begin(), final_marker.m_corners.begin() + rotation_idx, final_marker.m_corners.end());

		m_markers.push_back(final_marker);

#endif // myselfmarker



__wrongMarker:
		continue;
	}

	//亚像素点精确
	for (int i = 0; i < m_markers.size(); ++i)
	{
		vector<Point2f>& corners = m_markers[i].m_corners;
		cornerSubPix(image_gray, corners, Size(5,5), Size(-1,-1), TermCriteria(CV_TERMCRIT_ITER, 30, 0.1));
	}

	//耗时
	if (m_markers.size()>0)
	{
		cout<<"识别标记耗时ms："<<GetSysTime_number()-iStartTime<<endl;
	}

	//保存中间过程图片
	if (0 && m_markers.size()>0)
	{
		string strCurTime=GetSysTime_string();
		string picFileDir="data\\process\\"+strCurTime;
		QString picFolder=QString::fromStdString(picFileDir);
		QDir dirmaker(picFolder);
		if (! dirmaker.exists())
		{
			dirmaker.mkdir(dirmaker.absolutePath());
		}
		imwrite(picFileDir+"\\image_ori.jpg" , image_ori);
		imwrite(picFileDir+"\\image_gray.jpg" , image_gray);
		imwrite(picFileDir+"\\image_adaptiveThreshold.jpg" , image_adaptiveThreshold);
		imwrite(picFileDir+"\\image_morphologyEx.jpg" , image_morphologyEx);
		imwrite(picFileDir+"\\image_warpPerspective.jpg" , image_marker_gray);
		imwrite(picFileDir+"\\image_threshold.jpg" , image_marker_threshold);
		imwrite(picFileDir+"\\image_drawCorners.jpg" , image_drawCorners);

		_picFileDir=picFileDir;
	}


	//if(image_ori.rows>0&&image_ori.cols>0)	imshow("\\image_ori.jpg" , image_ori);
	//if(image_gray.rows>0&&image_gray.cols>0)	imshow("\\image_gray.jpg" , image_gray);
	//if(image_adaptiveThreshold.rows>0&&image_adaptiveThreshold.cols>0)	imshow("\\image_adaptiveThreshold.jpg" , image_adaptiveThreshold);
	//if(image_morphologyEx.rows>0&&image_morphologyEx.cols>0)	imshow("\\image_morphologyEx.jpg" , image_morphologyEx);
	//if(image_marker_gray.rows>0&&image_marker_gray.cols>0)	imshow("\\image_warpPerspective.jpg" , image_marker_gray);
	//if(image_marker_threshold.rows>0&&image_marker_threshold.cols>0)	imshow("\\image_threshold.jpg" , image_marker_threshold);
	//if(image_drawCorners.rows>0&&image_drawCorners.cols>0)	imshow("\\image_drawCorners.jpg" , image_drawCorners);

	return m_markers.size();
}

Mat MarkerRecognizer::bitMatrixRotate(cv::Mat& bit_matrix)
{
	//Rotate the bitMatrix by anti-clockwise way
	Mat out = bit_matrix.clone();
	int rows = bit_matrix.rows;
	int cols = bit_matrix.cols;

	for (int i=0; i<rows; ++i)
	{
		for (int j=0; j<cols; j++)
		{
			out.at<uchar>(i,j) = bit_matrix.at<uchar>(cols-j-1, i);
		}
	}
	return out;
}

int MarkerRecognizer::hammingDistance(Mat& bit_matrix)
{
	const int ids[4][5]=
	{
		{1,0,0,0,0},	// 00
		{1,0,1,1,1},	// 01
		{0,1,0,0,1},	// 10
		{0,1,1,1,0}	// 11
	};
  
	int dist=0;

	for (int y=0; y<5; ++y)
	{
		int minSum = INT_MAX; //hamming distance to each possible word
    
		for (int p=0; p<4; ++p)
		{
			int sum=0;
			//now, count
			for (int x=0; x<5; ++x)
			{
				sum += !(bit_matrix.at<uchar>(y, x) == ids[p][x]);
			}
			minSum = min(minSum, sum);
		}
    
		//do the and
		dist += minSum;
	}
  
	return dist;
}

int MarkerRecognizer::bitMatrixToId(Mat& bit_matrix)
{
	int id = 0;
	for (int y=0; y<5; ++y)
	{
		id <<= 1;
		id |= bit_matrix.at<uchar>(y,1);

		id <<= 1;
		id |= bit_matrix.at<uchar>(y,3);
	}
	return id;
}

vector<Marker>& MarkerRecognizer::getMarkers()
{
	return m_markers;
}

void MarkerRecognizer::drawCorners(cv::Mat& image, cv::Scalar color, float thickness)
{
	for (int i = 0; i < m_markers.size(); ++i)
	{
		m_markers[i].drawCorners(image, color, thickness);
	}
}


/*void MarkerRecognizer::markerDetect(Mat& img_gray, vector<Marker>& possible_markers, int min_size, int min_side_length)
{
Mat img_Threshold;
Mat img_Open;
Mat img_Close;
Mat img_erode;

int thresh_size = (min_size/4)*2 + 1;
//将可变（自适应）阈值应用于图像
adaptiveThreshold(img_gray, img_Threshold, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, thresh_size, thresh_size/3);

if(_picFileDir!="")
imwrite(_picFileDir+"\\adaptiveThreshold.jpg" , img_Threshold);

//对图像应用高级形态学操作(开运算)
morphologyEx(img_Threshold, img_Open, MORPH_OPEN, Mat());	//use open operator to eliminate small patch

if(_picFileDir!="")
imwrite(_picFileDir+"\\morphologyEx.jpg" , img_Open);

////边缘检测 不能提取边缘
//Mat img_Canny;
//Canny(img_Open, img_Canny, 120, 200);
////imshow("img_Canny", img_Canny);

//查找角点
vector<vector<Point>> all_contours;
vector<vector<Point>> contours;
findContours(img_Open, all_contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

//检查每个轮廓点的数量，需要大于一定的值100
for (int i = 0; i < all_contours.size(); ++i)
{
if (all_contours[i].size() > min_size)
{
contours.push_back(all_contours[i]);
}
}

vector<Point> approx_poly;
for (int i = 0; i < contours.size(); ++i)
{
double eps = contours[i].size()*APPROX_POLY_EPS;
approxPolyDP(contours[i], //输入的点集
approx_poly, //输出点集
eps, //精度，原始曲线与近似曲线之间的最大距离
true);//闭合

if (approx_poly.size() != 4)
continue;
//检查是否凸包 opencv自带
if (!isContourConvex(approx_poly))
continue;

//四边形各顶点之间的最短距离
float min_side = FLT_MAX;
for (int j = 0; j < 4; ++j)
{
Point side = approx_poly[j] - approx_poly[(j+1)%4];
min_side = min(min_size, side.dot(side));
}
if (min_side < min_side_length*min_side_length)
continue;

//Sort the points in anti-clockwise
Marker marker = Marker(0, approx_poly[0], approx_poly[1], approx_poly[2], approx_poly[3]);
Point2f v1 = marker.m_corners[1] - marker.m_corners[0];
Point2f v2 = marker.m_corners[2] - marker.m_corners[0];
if (v1.cross(v2) < 0)	//大于零才代表逆时针
{
swap(marker.m_corners[1], marker.m_corners[3]);
}
possible_markers.push_back(marker);
}
}

void MarkerRecognizer::markerRecognize(cv::Mat& img_gray, vector<Marker>& possible_markers, vector<Marker>& final_markers)
{
final_markers.clear();

Mat bit_matrix(giCellCount, giCellCount, CV_8UC1);
for (int i = 0; i < possible_markers.size(); ++i)
{
//返回3x3透视变换矩阵	//源图像四个顶点坐标//目标图像四个顶点坐标
Mat M = cv::getPerspectiveTransform(possible_markers[i].m_corners, m_marker_coords);

//透视变换 由变换矩阵后的图像获取平面图像，输出图像的区域由size控制
Mat img_marker;
warpPerspective(img_gray, //i
img_marker, //o
M, //matrix
Size(giMarkerSize, giMarkerSize));

if(_picFileDir!="")
imwrite(_picFileDir+"\\warpPerspective.jpg" , img_marker);

threshold(img_marker, img_marker, 125, 255, THRESH_BINARY|THRESH_OTSU); //OTSU 大律二值化

if(_picFileDir!="")
imwrite(_picFileDir+"\\threshold.jpg" , img_marker);

//黑框
for (int y = 0; y < giCellCountWithFrame; ++y)
{
//第一列及最后一列7行均黑，其他列只有第1行及第7行黑色，所以取cell像素时第一列及最后一列单步跳跃，其他从第一行跳到最后一行
int iJump = (y == 0 || y == giCellCountWithFrame-1) ? 1 : giCellCountWithFrame-1;
int cell_y = y*giCellSize;

for (int x = 0; x < giCellCountWithFrame; x += iJump)
{
int cell_x = x*giCellSize;
int none_zero_count = countNonZero(img_marker(Rect(cell_x, cell_y, giCellSize, giCellSize)));
//此时灰度不为0像素应该giCellSize*giCellSize，考虑反光和误差，/4
if (none_zero_count > giCellSize*giCellSize/4)
goto __wrongMarker;
}
}

//Decode the marker
for (int y = 0; y < giCellCount; ++y)
{
//要排除边框，所以要x+1 y+1
int cell_y = (y+1)*giCellSize;

for (int x = 0; x < giCellCount; ++x)
{
int cell_x = (x+1)*giCellSize;
int none_zero_count = countNonZero(img_marker(Rect(cell_x, cell_y, giCellSize, giCellSize)));
if (none_zero_count > giCellSize*giCellSize/2)
bit_matrix.at<uchar>(y, x) = 1;
else
bit_matrix.at<uchar>(y, x) = 0;
}
}

bool good_marker = false;
int rotation_idx=0;	//逆时针旋转的次数
//使用hammingmarker or myselfmarker
#ifdef myselfmarker
//左上
if (bit_matrix.at<uchar>(0, 0)==1 &&bit_matrix.at<uchar>(0, 1)==0 &&bit_matrix.at<uchar>(1, 0)==0 &&bit_matrix.at<uchar>(1, 1)==0)
{
rotation_idx = 0;
good_marker = true;
}
//左下
else	if (bit_matrix.at<uchar>(giCellCount-1, 0)==1 &&bit_matrix.at<uchar>(giCellCount-2, 0)==0 &&bit_matrix.at<uchar>(giCellCount-1, 1)==0 &&bit_matrix.at<uchar>(giCellCount-2, 1)==0)
{
rotation_idx = 1;
good_marker = true;
}
//右下
else	if (bit_matrix.at<uchar>(giCellCount-1, giCellCount-1)==1 &&bit_matrix.at<uchar>(giCellCount-2, giCellCount-1)==0 &&bit_matrix.at<uchar>(giCellCount-1, giCellCount-2)==0 &&bit_matrix.at<uchar>(giCellCount-2, giCellCount-2)==0)
{
rotation_idx = 2;
good_marker = true;
}
//右上
else	if (bit_matrix.at<uchar>(0, giCellCount-1)==1 &&bit_matrix.at<uchar>(0, giCellCount-2)==0 &&bit_matrix.at<uchar>(1, giCellCount-2)==0 &&bit_matrix.at<uchar>(1, giCellCount-1)==0)
{
rotation_idx = 3;
good_marker = true;
}

if (!good_marker) goto __wrongMarker;

//Store the final marker
Marker& final_marker = possible_markers[i];
std::rotate(final_marker.m_corners.begin(), final_marker.m_corners.begin() + rotation_idx, final_marker.m_corners.end());

final_markers.push_back(final_marker);


drawCorners(img_gray , cv::Scalar(255,0,0) , 1);

if(_picFileDir!="")
imwrite(_picFileDir+"\\drawCorners.jpg" , img_gray);

#else
for (rotation_idx = 0; rotation_idx < 4; ++rotation_idx)
{
if (hammingDistance(bit_matrix) == 0)
{
good_marker = true;
break;
}
bit_matrix = bitMatrixRotate(bit_matrix);
}

if (!good_marker) goto __wrongMarker;

//Store the final marker
Marker& final_marker = possible_markers[i];

final_marker.m_id = bitMatrixToId(bit_matrix);
std::rotate(final_marker.m_corners.begin(), final_marker.m_corners.begin() + rotation_idx, final_marker.m_corners.end());

final_markers.push_back(final_marker);

#endif // myselfmarker



__wrongMarker:
continue;
}
}

//角点亚像素精确化
void MarkerRecognizer::markerRefine(cv::Mat& img_gray, vector<Marker>& final_markers)
{
for (int i = 0; i < final_markers.size(); ++i)
{
vector<Point2f>& corners = final_markers[i].m_corners;
cornerSubPix(img_gray, corners, Size(5,5), Size(-1,-1), TermCriteria(CV_TERMCRIT_ITER, 30, 0.1));
}
}
*/