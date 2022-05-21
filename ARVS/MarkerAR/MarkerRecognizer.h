#ifndef __MARKER_RECOGNIZER_H__
#define __MARKER_RECOGNIZER_H__

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <math.h>
#include <string>
#include <sstream>
using namespace std;
using namespace cv;

class Marker
{
public:
	int m_id;
	std::vector<cv::Point2f> m_corners;
	// 0-----3
	// |			|
	// |			|
	// 1-----2

public:
	Marker();
	Marker(int _id, cv::Point2f _c0, cv::Point2f _c1, cv::Point2f _c2, cv::Point2f _c3);

	void estimateTransformToCamera(std::vector<cv::Point3f> corners_3d, cv::Mat& camera_matrix, cv::Mat& dist_coeff, cv::Mat& rmat, cv::Mat& tvec);
	void drawCorners(cv::Mat& image, cv::Scalar color, float thickness);
};

class MarkerRecognizer
{
private:
	std::vector<cv::Point2f> m_marker_coords;
	std::vector<Marker> m_markers;

private:
	void markerDetect(cv::Mat& img_gray, std::vector<Marker>& possible_markers, int min_size, int min_side_length);
	void markerRecognize(cv::Mat& img_gray, std::vector<Marker>& possible_markers, std::vector<Marker>& final_markers);
	void markerRefine(cv::Mat& img_gray, std::vector<Marker>& final_markers);
	cv::Mat bitMatrixRotate(cv::Mat& bit_matrix);

	string _picFileDir;//中间如果输出，输出的位置

public:
	MarkerRecognizer();
	string GetExportPicFolder(){return _picFileDir;}
	int update(cv::Mat& image_gray, int min_size, int min_side_length = 10);
	std::vector<Marker>& getMarkers();
	void drawCorners(cv::Mat& image, cv::Scalar color, float thickness);
};

#endif