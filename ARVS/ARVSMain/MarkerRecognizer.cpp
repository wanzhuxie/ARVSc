#include "MarkerRecognizer.h"
#include "GeneralFunctions.h"
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

#define ADAPTIVE_THRESH_SIZE 35
#define APPROX_POLY_EPS 0.08

const int giCellSize=10;
const int giCellCount=5;
const int giCellCountWithFrame=giCellCount+2;
const int giMarkerSize=giCellCountWithFrame*giCellSize;





/*
��ǿɸ���ʵ������ȷ��������ʹ��5��5�еĺڰ׷���������Ϊ��ǣ���ͼ1��ʾ����ɫ����1����ɫ����0����ʵ�ϣ�������ݼ�Ϊ������Ϣ��Ϊ����ʶ��ʹ�ú�ɫ�߿��Χ�������
Ϊ������ǵ���ת�Գ����Լ�ĳһ��ȫ����ɫ����֤������Ϣ��Ψһ���Ա�Ƕ�������
	��ǵ����ϽǱ���Ϊ������ɫ���飬������3���ǲ����й�����ɫ���飻
	���һ�м����һ�о��а�ɫ���飬���һ�е���ֵ֮�Ͳ���Ϊ0�����һ�е���ֵ֮�Ͳ���Ϊ0

��ԭʼͼ��ҶȻ�
������
ʹ��cv::findContours���ҽǵ㣬�õ�����
����ÿ�����飬�ж��Ƿ�Ϊ͹�ı��Σ������ǣ�������
ʹ��cv::getPerspectiveTransform�õ�͸�ӱ任����
ʹ��cv::warpPerspective��ͼ��͸�ӱ任����ȡ������ڵ�����
��ͼ���ֵ����ʹ��cv::countNonZero��������ֵ�жϷ������ɫ���õ���ǰ����
���ձ�ǵĶ��壬���㵱ǰ��ǵ���ת״̬
���ˣ��õ��˱�ǵ�4���ǵ㼰�ǵ�˳��
�Խǵ������ؾ�ȷ
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
	bool res = solvePnP(corners_3d,		//i��������ϵ�µĿ��Ƶ������
		m_corners,									//iͼ������ϵ�¶�Ӧ�Ŀ��Ƶ������
		camera_matrix,								//i����ڲ�
		dist_coeff,									//i�������
		rot_vec,										//o��ת����
		tvec);											//oƽ������

	Rodrigues(rot_vec, rmat);				//��ת������Ϊ��ת����
}



//====================================Class MarkerRecognizer================================
MarkerRecognizer::MarkerRecognizer()
{
	//��׼Marker���꣬��ʱ��
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

	//�ҿ��ܵı��
	vector<Marker> possible_markers;
	{
		Mat img_Threshold;
		Mat img_Open;
		Mat img_Close;
		Mat img_erode;

		int thresh_size = (min_size/4)*2 + 1;
		//���ɱ䣨����Ӧ����ֵӦ����ͼ��
		adaptiveThreshold(image_gray, image_adaptiveThreshold, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV , thresh_size, thresh_size*0.5);

		//��ͼ��Ӧ�ø߼���̬ѧ����(������)
		morphologyEx(image_adaptiveThreshold, image_morphologyEx, MORPH_OPEN, Mat());	//use open operator to eliminate small patch

		//���ҽǵ�
		vector<vector<Point>> all_contours;
		vector<vector<Point>> contours;
		findContours(image_morphologyEx, all_contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

		//���ÿ�����������������Ҫ����һ����ֵ100
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
			approxPolyDP(contours[i], //����ĵ㼯
				approx_poly, //����㼯
				eps, //���ȣ�ԭʼ�������������֮���������
				true);//�պ�

			if (approx_poly.size() != 4)
				continue;
			//����Ƿ�͹�� opencv�Դ�
			if (!isContourConvex(approx_poly))
				continue;

			//�ı��θ�����֮�����̾���
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
			if (v1.cross(v2) < 0)	//������Ŵ�����ʱ��
			{
				swap(marker.m_corners[1], marker.m_corners[3]);
			}
			possible_markers.push_back(marker);
		}

	}

	//͸�ӱ任����Ƿ�����Ǳ��
	m_markers.clear();
	Mat bit_matrix(giCellCount, giCellCount, CV_8UC1);
	for (int i = 0; i < possible_markers.size(); ++i)
	{
		//����3x3͸�ӱ任����	//Դͼ���ĸ���������//Ŀ��ͼ���ĸ���������
		Mat M = cv::getPerspectiveTransform(possible_markers[i].m_corners, m_marker_coords);

		//͸�ӱ任 �ɱ任������ͼ���ȡƽ��ͼ�����ͼ���������size����
		warpPerspective(image_gray, //i
			image_marker_gray, //o
			M, //matrix
			Size(giMarkerSize, giMarkerSize));

		cv::threshold(image_marker_gray, image_marker_threshold, 125, 255, THRESH_BINARY|THRESH_OTSU); //OTSU ���ɶ�ֵ��

		//�ڿ�
		for (int y = 0; y < giCellCountWithFrame; ++y)
		{
			//��һ�м����һ��7�о��ڣ�������ֻ�е�1�м���7�к�ɫ������ȡcell����ʱ��һ�м����һ�е�����Ծ�������ӵ�һ���������һ��
			int iJump = (y == 0 || y == giCellCountWithFrame-1) ? 1 : giCellCountWithFrame-1;
			int cell_y = y*giCellSize;

			for (int x = 0; x < giCellCountWithFrame; x += iJump)
			{
				int cell_x = x*giCellSize;
				int none_zero_count = countNonZero(image_marker_threshold(Rect(cell_x, cell_y, giCellSize, giCellSize)));
				//��ʱ�ҶȲ�Ϊ0����Ӧ��giCellSize*giCellSize�����Ƿ������/4
				if (none_zero_count > giCellSize*giCellSize/4)
					goto __wrongMarker;
			}
		}

		//Decode the marker
		for (int y = 0; y < giCellCount; ++y)
		{
			//Ҫ�ų��߿�����Ҫx+1 y+1
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
		int rotation_idx=0;	//��ʱ����ת�Ĵ���
		//����
		if (bit_matrix.at<uchar>(0, 0)==1 &&bit_matrix.at<uchar>(0, 1)==0 &&bit_matrix.at<uchar>(1, 0)==0 &&bit_matrix.at<uchar>(1, 1)==0)
		{
			rotation_idx = 0;
			good_marker = true;
		}
		//����
		else	if (bit_matrix.at<uchar>(giCellCount-1, 0)==1 &&bit_matrix.at<uchar>(giCellCount-2, 0)==0 &&bit_matrix.at<uchar>(giCellCount-1, 1)==0 &&bit_matrix.at<uchar>(giCellCount-2, 1)==0)
		{
			rotation_idx = 1;
			good_marker = true;
		}
		//����
		else	if (bit_matrix.at<uchar>(giCellCount-1, giCellCount-1)==1 &&bit_matrix.at<uchar>(giCellCount-2, giCellCount-1)==0 &&bit_matrix.at<uchar>(giCellCount-1, giCellCount-2)==0 &&bit_matrix.at<uchar>(giCellCount-2, giCellCount-2)==0)
		{
			rotation_idx = 2;
			good_marker = true;
		}
		//����
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


__wrongMarker:
		continue;
	}

	//�����ص㾫ȷ
	for (int i = 0; i < m_markers.size(); ++i)
	{
		vector<Point2f>& corners = m_markers[i].m_corners;
		cornerSubPix(image_gray, corners, Size(5,5), Size(-1,-1), TermCriteria(CV_TERMCRIT_ITER, 30, 0.1));
	}

	//��ʱ
	if (m_markers.size()>0)
	{
		cout<<"ʶ���Ǻ�ʱms��"<<GetSysTime_number()-iStartTime<<endl;
	}

	//�����м����ͼƬ
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


