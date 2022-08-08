#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

void main() 
{
	ifstream fin("Data\\calibdata.txt"); //Img data
	ofstream fout("Data\\result.txt");  //Redult data	
	//read every image
	cout<<"Start corner extraction......"<<endl;
	int image_count=0;
	Size image_size;  
	Size board_size = Size(7,7); //注意是黑白方块的交界点，如方块为8X8，则交点数为7X7Note that it is the intersection point of black and white squares. If the square is 8x8, the intersection point is 7x7
	vector<Point2f> image_points_buf;  
	vector<vector<Point2f>> image_points_seq;
	string filename;
	int count= -1 ;//corner count
	while (getline(fin,filename))
	{
		image_count++;		
		cout<<"image_count = "<<image_count<<endl;		
		cout<<"-->count = "<<count;		
		Mat imageInput=imread(filename);
		if (image_count == 1)  //read first image to get width and hight
		{
			image_size.width = imageInput.cols;
			image_size.height =imageInput.rows;			
			cout<<"image_size.width = "<<image_size.width<<endl;
			cout<<"image_size.height = "<<image_size.height<<endl;
		}

		/* extract corner */
		if (0 == findChessboardCorners(imageInput,board_size,image_points_buf))
		{			
			cout<<"can not find chessboard corners!\n"; //no corner
			system("pause");
			exit(1);
		} 
		else 
		{
			Mat view_gray;
			cvtColor(imageInput,view_gray,CV_RGB2GRAY);
			/* Subpixel precision */
			find4QuadCornerSubpix(view_gray,image_points_buf,Size(5,5)); 
			//cornerSubPix(view_gray,image_points_buf,Size(5,5),Size(-1,-1),TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER,30,0.1));
			image_points_seq.push_back(image_points_buf);
			/*Show corner position on image*/
			drawChessboardCorners(view_gray,board_size,image_points_buf,false); 
			//imshow("Camera Calibration",view_gray);//show image
			//waitKey(500);
		}
	}
	int total = image_points_seq.size();
	cout<<"total = "<<total<<endl;
	int CornerNum=board_size.width*board_size.height;  //total corners
	for (int ii=0 ; ii<total ;ii++)
	{
		if (0 == ii%CornerNum)
		{	
			int i = -1;
			i = ii/CornerNum;
			int j=i+1;
			cout<<"--> The "<<j <<"image data --> : "<<endl;
		}
		if (0 == ii%3)
		{
			cout<<endl;
		}
		else
		{
			cout.width(10);
		}
		//output all corner
		cout<<" -->"<<image_points_seq[ii][0].x;
		cout<<" -->"<<image_points_seq[ii][0].y;
	}	
	cout<<"Corner extraction completed!\n";

	cout<<"camera calibration";
	/*Chessboard 3D information*/
	Size square_size = Size(10,10);  /* Real size */
	vector<vector<Point3f>> object_points; /* 保存标定板上角点的三维坐标 */
	/*内外参数*/
	Mat cameraMatrix=Mat(3,3,CV_32FC1,Scalar::all(0)); /* 摄像机内参数矩阵 */
	vector<int> point_counts;  // 每幅图像中角点的数量
	Mat distCoeffs=Mat(1,5,CV_32FC1,Scalar::all(0)); /* 摄像机的5个畸变系数：k1,k2,p1,p2,k3 */
	vector<Mat> tvecsMat;  /* 每幅图像的旋转向量 */
	vector<Mat> rvecsMat; /* 每幅图像的平移向量 */
	/* 初始化标定板上角点的三维坐标 */
	int i,j,t;
	for (t=0;t<image_count;t++) 
	{
		vector<Point3f> tempPointSet;
		for (i=0;i<board_size.height;i++) 
		{
			for (j=0;j<board_size.width;j++) 
			{
				Point3f realPoint;
				/* Suppose the calibration plate is placed on the plane of Z = 0 in the world coordinate system */
				realPoint.x = i*square_size.width;
				realPoint.y = j*square_size.height;
				realPoint.z = 0;
				tempPointSet.push_back(realPoint);
			}
		}
		object_points.push_back(tempPointSet);
	}

	for (i=0;i<image_count;i++)
	{
		point_counts.push_back(board_size.width*board_size.height);
	}	
	/* Start */
	calibrateCamera(object_points,image_points_seq,image_size,cameraMatrix,distCoeffs,rvecsMat,tvecsMat,0);
	cout<<"calibrate OK\n";
	//estimate
	cout<<"estimate......\n";
	double total_err = 0.0; 
	double err = 0.0; /* average error of each image */
	vector<Point2f> image_points2; /* Save the recalculated projection points */
	cout<<"Calibration error of each image\n";
	fout<<"Calibration error of each image\n";
	for (i=0;i<image_count;i++)
	{
		vector<Point3f> tempPointSet=object_points[i];
		/* Through the obtained internal and external parameters of the camera, the three-dimensional points in the space are re projected and calculated to obtain new projection points */
		projectPoints(tempPointSet,rvecsMat[i],tvecsMat[i],cameraMatrix,distCoeffs,image_points2);
		/* Calculate the error between the new projection point and the old projection point*/
		vector<Point2f> tempImagePoint = image_points_seq[i];
		Mat tempImagePointMat = Mat(1,tempImagePoint.size(),CV_32FC2);
		Mat image_points2Mat = Mat(1,image_points2.size(), CV_32FC2);
		for (int j = 0 ; j < tempImagePoint.size(); j++)
		{
			image_points2Mat.at<Vec2f>(0,j) = Vec2f(image_points2[j].x, image_points2[j].y);
			tempImagePointMat.at<Vec2f>(0,j) = Vec2f(tempImagePoint[j].x, tempImagePoint[j].y);
		}
		err = norm(image_points2Mat, tempImagePointMat, NORM_L2);
		total_err += err/=  point_counts[i];   
		std::cout<<"The "<<i+1<<"image's error: "<<err<<"pixies"<<endl;   
		fout<<"The "<<i+1<<"image's error: "<<err<<"pixies"<<endl;   
	}   
	std::cout<<"Overall mean error: "<<total_err/image_count<<"pixies"<<endl;   
	fout<<"Overall mean error: "<<total_err/image_count<<"pixies"<<endl<<endl;   
	std::cout<<"Evaluation completed!"<<endl;  
	//save result	
	std::cout<<"save result......"<<endl;
	Mat rotation_matrix = Mat(3,3,CV_32FC1, Scalar::all(0));
	fout<<"Camera internal parameter matrix: "<<endl;   
	fout<<cameraMatrix<<endl<<endl;   
	fout<<"The distortion coefficient: \n";   
	fout<<distCoeffs<<endl<<endl<<endl;   
	for (int i=0; i<image_count; i++) 
	{ 
		fout<<"The "<<i+1<<"image's rotation vector"<<endl;   
		fout<<tvecsMat[i]<<endl;    

		Rodrigues(tvecsMat[i],rotation_matrix);   
		fout<<"The "<<i+1<<"image's rotation matrix"<<endl;   
		fout<<rotation_matrix<<endl;   
		fout<<"The "<<i+1<<"image's translation vector"<<endl;   
		fout<<rvecsMat[i]<<endl<<endl;   
	}   
	std::cout<<"saved"<<endl; 
	fout<<endl;
	/************************************************************************  
	show result  
	*************************************************************************/
	if (0)
	{
		Mat mapx = Mat(image_size,CV_32FC1);
		Mat mapy = Mat(image_size,CV_32FC1);
		Mat R = Mat::eye(3,3,CV_32F);
		std::cout<<"save correct image"<<endl;
		string imageFileName;
		std::stringstream StrStm;
		for (int i = 0 ; i != image_count ; i++)
		{
			std::cout<<"Frame #"<<i+1<<"..."<<endl;
			initUndistortRectifyMap(cameraMatrix,distCoeffs,R,cameraMatrix,image_size,CV_32FC1,mapx,mapy);		
			StrStm.clear();
			imageFileName.clear();
			string filePath="chess";
			StrStm<<i+1;
			StrStm>>imageFileName;
			filePath+=imageFileName;
			filePath+=".bmp";
			Mat imageSource = imread(filePath);
			Mat newimage = imageSource.clone();
			remap(imageSource,newimage,mapx, mapy, INTER_LINEAR);		
			StrStm.clear();
			filePath.clear();
			StrStm<<i+1;
			StrStm>>imageFileName;
			imageFileName += "_d.jpg";
			imwrite(imageFileName,newimage);
		}
		std::cout<<"ok"<<endl;	
	}

}










