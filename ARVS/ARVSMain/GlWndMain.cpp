/*
相机标定
根据投影关系设置GL投影矩阵
设置标记角点在世界坐标系下的坐标
计算图像坐标系下标记角点对应的坐标
根据世界坐标系下的坐标、图像坐标系下标记角点对应的坐标，以及相机内参，使用cv::solvePnP计算GL模型变换矩阵
分别设置GL的投影矩阵及模型视图矩阵
在当前矩阵下绘制立方体
对立方体的各个面设置贴图
*/
#include "GlWndMain.h"
#include "GeneralFunctions.h"
#include <QImage>
#include <math.h>
#include <string>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <time.h>
#include <io.h>
#include <QPoint>
#include <algorithm>

using namespace std;


float dARBoxWidth=0.4;

double GlWndMain::ComputeThumbAngle( )
{
	Vector3 mThumbDir(_vecAllHandPoints[4]-_vecAllHandPoints[2]);
	Vector3 mYDir(_vecAllHandPoints[9]-_vecAllHandPoints[0]);
	return mThumbDir.angle(mYDir);
}
std::string ComputeStateSignal(const vector<int>&vecState)
{
	std::string strResult;
	for (int a=0;a<vecState.size();a++)
	{
		strResult+=IntToStr(vecState[a]);
	}

	return strResult;
}

GlWndMain::GlWndMain(QWidget *parent)
	: QGLWidget(parent)
	,_bRun(false)
	,_bPause(false)
	,_bKeepState(false)
	,_bHyaline(false)
	,_bFog(false)
	,_bOpenAR(true)
	,_CurState(OpState::Initial)
	,_bCreatedArBox(false)
	,_bShowHandPoints(true)
	,_bARVideoOK(true)
	,_iLastTime(0)
{
	
	videoOfAR0=VideoCapture("Data/video0.mp4");
	videoOfAR1=VideoCapture("Data/video1.mp4");
	videoOfAR2=VideoCapture("Data/video2.mp4");
	videoOfAR3=VideoCapture("Data/video3.mp4");
	videoOfAR4=VideoCapture("Data/video4.mp4");
	videoOfAR5=VideoCapture("Data/video5.mp4");
	if (videoOfAR0.isOpened())		_iFrameCount_Video0=videoOfAR0.get(CV_CAP_PROP_FRAME_COUNT);	else	_bARVideoOK=false;
	if (videoOfAR1.isOpened())		_iFrameCount_Video1=videoOfAR1.get(CV_CAP_PROP_FRAME_COUNT);	else	_bARVideoOK=false;
	if (videoOfAR2.isOpened())		_iFrameCount_Video2=videoOfAR2.get(CV_CAP_PROP_FRAME_COUNT);	else	_bARVideoOK=false;
	if (videoOfAR3.isOpened())		_iFrameCount_Video3=videoOfAR3.get(CV_CAP_PROP_FRAME_COUNT);	else	_bARVideoOK=false;
	if (videoOfAR4.isOpened())		_iFrameCount_Video4=videoOfAR4.get(CV_CAP_PROP_FRAME_COUNT);	else	_bARVideoOK=false;
	if (videoOfAR5.isOpened())		_iFrameCount_Video5=videoOfAR5.get(CV_CAP_PROP_FRAME_COUNT);	else	_bARVideoOK=false;


	_dRotX = _dRotY = _dRotZ = 0;

	//Correction value
	stepRotX=0;
	stepRotY=0;
	stepRotZ=0;

	_dMoveX=_dMoveY=0;
	_dZoom = -10.0;
	fullScreen = false;

	if (fullScreen)
	{
		showFullScreen();
	}
	startTimer(1);

	//hand point
	if (!_handPointsCls.Init())
	{
		cout<<"error on _handPointsCls.Init"<<endl;
		return ;
	}

	//current screen resolution
	_iScreenWidth = GetSystemMetrics (SM_CXSCREEN) ;  // wide
	_iScreenHight = GetSystemMetrics (SM_CYSCREEN) ;  // high

}
GlWndMain::~GlWndMain()
{
	_handPointsCls.Close();
}
void GlWndMain::initializeGL()
{

	_mMainCapture=VideoCapture(0);
	//_mMainCapture=VideoCapture(1);

	glEnable(GL_TEXTURE_2D);//启用纹理 use texture
	glEnable(GL_COLOR_MATERIAL);//可以用颜色纹理 can use pure color texture
	glShadeModel(GL_SMOOTH);
	glClearColor(1,1,1,0.5);// screen color

	glClearDepth(1.0);//设置深度缓存 depth cache
	glEnable(GL_DEPTH_TEST);//启用深度测试 DEPTH_TEST
	glDepthFunc(GL_LEQUAL);//所做深度测试的类型 DEPTH_TEST_Type

	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);//投影修正 projection correction
	glPolygonMode(GL_BACK,GL_FILL);//
	glPolygonMode(GL_FRONT,GL_FILL);//

	//light
	GLfloat lightAmbient[4] = {0.5,0.5,0.5,1.0};
	GLfloat lightDiffuse[4] = {1.0,1.0,1.0,1.0};
	GLfloat lightPosition[4] = {0.0,0.0,2.0,1.0};
	glLightfv(GL_LIGHT1,GL_AMBIENT,lightAmbient);
	glLightfv(GL_LIGHT1,GL_DIFFUSE,lightDiffuse);
	glLightfv(GL_LIGHT1,GL_POSITION,lightPosition);
	glEnable(GL_LIGHT1);

	//flog
	GLuint fogMode[3] = {GL_EXP,GL_EXP2,GL_LINEAR};
	GLfloat fogColor[4] = {1,1,1,0.3};
	glFogi(GL_FOG_MODE,fogMode[0]);
	glFogfv(GL_FOG_COLOR,fogColor);
	glFogf(GL_FOG_DENSITY,0.1);//雾的浓度
	glHint(GL_FOG_HINT,GL_FASTEST);//雾的渲染方式，GL_DONT_CARE不关心建议值，GL_NICEST极棒的，每一像素渲染，GL_FASTEST对每一顶点渲染，速度快
	glFogf(GL_FOG_START, 1);//雾离屏幕的距离
	glFogf(GL_FOG_END, 5.0);


	//marker corner
	_mMarkerCorners.push_back(Point3f(-0.5f, -0.5f, 0));
	_mMarkerCorners.push_back(Point3f(-0.5f,  0.5f, 0));
	_mMarkerCorners.push_back(Point3f( 0.5f,  0.5f, 0));
	_mMarkerCorners.push_back(Point3f( 0.5f, -0.5f, 0));


	////The parameters determined by camera calibration show that the cube deviates from the mark. Whether the uncertainty is caused by calibration data
	//float mCameraMatrix[] = 
	//{
	//	590.7319		,0						,292.9710,
	//	0					, 619.0881		,202.5625,
	//	0					,0						,1
	//};
	//float dist_coeff[] = {0.1095483732100013, 0.005921985694402154, -0.02522667923131416, -0.0171742783898786, -0.1891767195416431};
	
	////20220626 data
	//float mCameraMatrix[] = 
	//{
	//	621.6733		,0						,301.8697,
	//	0					, 596.7352		,223.5491,
	//	0					,0						,1
	//};
	//float dist_coeff[] = {0.2050844086865027, -1.253387945124429, -0.009926487596546369, -0.006799737561947785, 5.45488965637716};


	//camera data
	float mCameraMatrix[] = 
	{
		508.3018		,0						,300.1497,
		0					, 504.5175		, 264.5351,
		0					,0						,1
	};
	float dist_coeff[] = {-0.4172170641396942, -0.1135454666162299, -0.0009781100036345459, -0.006095536879777572, 0.7763703887603729};
	m_camera_matrix = Mat(3, 3, CV_32FC1, mCameraMatrix).clone();	
	m_dist_coeff = Mat(1, 5, CV_32FC1, dist_coeff).clone();

	//static image texture
	//loadGLTextures();
}
void GlWndMain::resizeGL(int w, int h)
{
	cout<<__FUNCTION__<<endl;

	if (h==0)
	{
		h=1;
	}
	glViewport(0,0,(GLint)w,(GLint)h);//reset viewpoint
	glMatrixMode(GL_PROJECTION);//projection matrix
	glLoadIdentity();//reset projection matrix
	gluPerspective(45, (GLfloat)w/(GLfloat)h,0.1,300);//perspective projection
	//gluOrtho2D(0, w, 0, h);
	glMatrixMode(GL_MODELVIEW);//MODELVIEW
	glLoadIdentity();//reset MODELVIEW


}
void GlWndMain::paintGL()
{
	int iStartTime=GetSysTime_number();

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	bool bAppearNewMarker=false;

	int width=this->width();
	int height=this->height();
		
	//if (_bShowHandPoints)
	//{
	//	for (int a=0;a<_vecAllHandPoints.size();a++)
	//	{
	//		int iEachRadius=_vecAllHandPoints[a].Z*0.1;
	//		if (iEachRadius<0)
	//			iEachRadius=-iEachRadius;
	//		iEachRadius+=3;
	//		cv::circle(_mFrameImage , Point(_vecAllHandPoints[a].X*2, _vecAllHandPoints[a].Y*2) , iEachRadius , cv::Scalar(200, 100, 100 ,0.5) , CV_FILLED);
	//	}
	//}

	QImage buf((const unsigned char*)_mFrameImage.data, _mFrameImage.cols, _mFrameImage.rows, _mFrameImage.cols * _mFrameImage.channels(), QImage::Format_RGB888);
	QImage mTex = QGLWidget::convertToGLFormat(buf);
	mTex=mTex.rgbSwapped();
	glLoadIdentity();
	glPixelZoom((GLfloat)width/mTex.width(),(GLfloat)height/mTex.height());
	glDrawPixels(mTex.width(),mTex.height(),GL_RGBA,GL_UNSIGNED_BYTE, mTex.bits());
	glClear(GL_DEPTH_BUFFER_BIT);

	if(_bOpenAR && _mFrameImage.data!=NULL)
	{
		if (m_recognizer.update(_mFrameImage, 100 , 10 )>0)
		{
			float width=640;float height=480;float near_plane=0.1;float far_plane=100;
			{
				float f_x = m_camera_matrix.at<float>(0,0);
				float f_y = m_camera_matrix.at<float>(1,1);

				float c_x = m_camera_matrix.at<float>(0,2);
				float c_y = m_camera_matrix.at<float>(1,2);
				/*
				w近剪裁面的宽度
				h近剪裁面的高度
				n近剪裁面距离摄像机的距离
				f远剪裁面距离摄像机的距离
				W width of near clipping face
				H height of near the clipping surface
				N distance from the near clipping plane to the camera
				F distance from the far clipping plane to the camera
				*/
				m_projection_matrix[0] = 2*f_x/width;
				m_projection_matrix[1] = 0.0f;
				m_projection_matrix[2] = 0.0f;
				m_projection_matrix[3] = 0.0f;
				m_projection_matrix[4] = 0.0f;
				m_projection_matrix[5] = 2*f_y/height;
				m_projection_matrix[6] = 0.0f;
				m_projection_matrix[7] = 0.0f;
				m_projection_matrix[8] = 1.0f - 2*c_x/width;
				m_projection_matrix[9] = 2*c_y/height - 1.0f;
				m_projection_matrix[10] = -(far_plane + near_plane)/(far_plane - near_plane);
				m_projection_matrix[11] = -1.0f;
				m_projection_matrix[12] = 0.0f;
				m_projection_matrix[13] = 0.0f;
				m_projection_matrix[14] = -2.0f*far_plane*near_plane/(far_plane - near_plane);
				m_projection_matrix[15] = 0.0f;

			}
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			//glMultMatrixf(m_projection_matrix);
			glLoadMatrixf(m_projection_matrix);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glEnable(GL_DEPTH_TEST);
			glShadeModel(GL_FLAT); //some model / light stuff  GL_SMOOTH
			vector<Marker>& markers = m_recognizer.getMarkers();
			Mat rotation, translation;
			for (int i = 0; i < markers.size(); ++i)
			{
				Mat rot_vec;
				bool res = solvePnP(_mMarkerCorners,		//i世界坐标系下的控制点的坐标			//I coordinates of control points in the world coordinate system
					markers[i].m_corners,								//i图像坐标系下对应的控制点的坐标	//I coordinates of corresponding control points in the image coordinate system
					m_camera_matrix,										//i相机内参											//I camera internal parameters
					m_dist_coeff,												//i相机畸变											//I camera distortion
					rot_vec,														//o旋转向量											//O rotation vector
					translation);												//o平移向量											//O translation vector

				Rodrigues(rot_vec, rotation);				//rotation vector to matrix
				//cout<<"translation..."<<endl<<translation<<endl;
				//cout<<"rot_vec..."<<endl<<rot_vec<<endl;
				//cout<<"rotation..."<<endl<<rotation<<endl;

				//绕X轴旋转180度，从OpenCV坐标系变换为OpenGL坐标系
				//Rotate 180 degrees around the X axis and transform from opencv coordinate system to OpenGL coordinate system
				static double d[] = 
				{
					1, 0, 0,
					0, -1, 0,
					0, 0, -1
				};
				Mat_<double> rx(3, 3, d);
				rotation = rx*rotation;
				translation = rx*translation;


				m_model_view_matrix[0] =		rotation.at<double>(0,0);
				m_model_view_matrix[1] =		rotation.at<double>(1,0);
				m_model_view_matrix[2] =		rotation.at<double>(2,0);
				m_model_view_matrix[3] =		0.0f;

				m_model_view_matrix[4] =		rotation.at<double>(0,1);
				m_model_view_matrix[5] =		rotation.at<double>(1,1);
				m_model_view_matrix[6] =		rotation.at<double>(2,1);
				m_model_view_matrix[7] =		0.0f;

				m_model_view_matrix[8] =		rotation.at<double>(0,2);
				m_model_view_matrix[9] =		rotation.at<double>(1,2);
				m_model_view_matrix[10] =		rotation.at<double>(2,2);
				m_model_view_matrix[11] =		0.0f;

				m_model_view_matrix[12] =		translation.at<double>(0, 0)+stepRotX;
				m_model_view_matrix[13] =		translation.at<double>(1, 0)+stepRotY;
				m_model_view_matrix[14] =		translation.at<double>(2, 0)+stepRotZ;
				m_model_view_matrix[15] =		1.0f;

					
				glLoadMatrixf(m_model_view_matrix);//set GL_MODELVIEW matrix

				DrawARBox();
				bAppearNewMarker=true;

				//test for output
				string strPicExportFolder=m_recognizer.GetExportPicFolder();
				if (strPicExportFolder!=""&&_access(strPicExportFolder.c_str() , 0)==0)
				{
					QPixmap::grabWindow(this->winId()).save((strPicExportFolder+"\\result.png").c_str(),"png");
				}
			}
		}
	}


	if (!_bPause)//Pause
	{
		_dRotX += stepRotX;
		_dRotY += stepRotY;
		_dRotZ += stepRotZ;
	}

	glLoadIdentity();
	glTranslatef(_dMoveX, _dMoveY, _dZoom);
	glRotatef(_dRotX,1,0,0);
	glRotatef(_dRotY,0,1,0);
	glRotatef(_dRotZ,0,0,1);


	if (_bHyaline)
	{
		glColor4f(1,1,0.8,0.8);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND );
		glDepthMask(FALSE);
	}
	else
	{
		glColor4f(1,1,1,1);
	}

	//如果已经创建过立方体&&此时没有标记，就沿用原来的位置 if there is no marker and the cube was created, the cube keep the latest position
	if (_bCreatedArBox && !bAppearNewMarker)
	{
		if (_CurState==OpState::Initial)
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glLoadMatrixf(m_projection_matrix);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glEnable(GL_DEPTH_TEST);
			glShadeModel(GL_SMOOTH); //some model / light stuff

			glLoadMatrixf(m_model_view_matrix);// set GL_MODELVIEW matrix
		}

		DrawARBox();
	}
	

	if (_bHyaline)
	{
		glDepthMask(TRUE);
		glDisable(GL_BLEND);
	}


}

//core callback
void GlWndMain::timerEvent(QTimerEvent *)
{
	////coutTimeSpan
	//int iCurTime=GetSysTime_number();
	//cout<<"回调 耗时(ms):"<<iCurTime-_iLastTime<<endl;
	//_iLastTime=iCurTime;

	//With the help of MediaPip in Python to get hand points
	_mMainCapture.read(_mFrameImage); 
	if (_access("Data\\CreatedImage" , 0)==0)
	{
		rename("Data\\CreatedImage" , "Data\\CreatingImage");
	}
	cv::flip(_mFrameImage,_mFrameImage,1);
	cv::Mat mTransBtidgeImage=_mFrameImage.clone();
	cv::resize(mTransBtidgeImage,mTransBtidgeImage,cv::Size(320,240));
	//cvtColor(mTransBtidgeImage,mTransBtidgeImage,CV_BGR2GRAY);
	imwrite("Data\\TempImg.jpg",mTransBtidgeImage);
	if (_access("Data\\CreatingImage" , 0)==0)
	{
		rename("Data\\CreatingImage" , "Data\\CreatedImage");
	}
	_vecAllHandPoints=_handPointsCls.GetAllHandPoints();
	if(_handPointsCls.GetHandCount()==0)
	{
		updateGL();
		return;
	}
	Point3D mTipPos_img=_handPointsCls.GetTipOfIndexFinger(false);
	if (mTipPos_img.X<0||mTipPos_img.Y<0)
	{
		return;
	}
	//5 number(0 or 1) :thumb, index finger, middle finger, ring finger, little finger
	vector<int> vecFingerState=_handPointsCls.GetFingerState(false);
	if (vecFingerState.size()!=5)
	{
		return;
	}

	std::string strCurFingereState=ComputeStateSignal(vecFingerState);

	cout<<vecFingerState[0];
	cout<<vecFingerState[1];
	cout<<vecFingerState[2];
	cout<<vecFingerState[3];
	cout<<vecFingerState[4];
	cout<<endl;

	//state change
	if (strCurFingereState=="00000" || strCurFingereState=="11111")
	{
		if (strCurFingereState=="11111"&&_strLastFingerState=="00000")
		{
			_bKeepState=true;

			if (_CurState==OpState::Initial)
			{
				cout<<"====Initial=FrontView===="<<endl;
				_CurState=OpState::FrontView;
			}
			else if (_CurState==OpState::FrontView)
			{
				cout<<"====FrontView=Move===="<<endl;
				_CurState=OpState::Move;
			}
			else if (_CurState==OpState::Move)
			{
				cout<<"====Move=Rotate===="<<endl;
				_CurState=OpState::Rotate;
			}
			else if (_CurState==OpState::Rotate)
			{
				cout<<"====Rotate=Zoom===="<<endl;
				_CurState=OpState::Zoom;
			}
			else if (_CurState==OpState::Zoom)
			{
				cout<<"====Zoom=FrontView===="<<endl;
				_CurState=OpState::FrontView;
			}
		}
		else		if (strCurFingereState=="11111"&&_CurState==OpState::FrontView)
		{
			if (!_bKeepState)
			{
				_dRotX=0;
				_dRotY=90;
				_dRotZ=0;
			}
		}
		_strLastFingerState=strCurFingereState;
	}
	else
	{
		_bKeepState=false;

		//cout<<_CurState<<endl;

		//FrontView
		if (_CurState==OpState::Initial)
		{
			_CurState=OpState::FrontView;
		}
		if (_CurState==OpState::FrontView)
		{
			if (vecFingerState[0]!=1
				&&vecFingerState[1]==1
				&&vecFingerState[2]!=1
				&&vecFingerState[3]!=1
				&&vecFingerState[4]!=1)
			{
				_dRotX=0;
				_dRotY=0;
				_dRotZ=0;
			}
			if (vecFingerState[0]!=1
				&&vecFingerState[1]==1
				&&vecFingerState[2]==1
				&&vecFingerState[3]!=1
				&&vecFingerState[4]!=1)
			{
				_dRotX=180;
				_dRotY=0;
				_dRotZ=0;
			}
			if (vecFingerState[0]!=1
				&&vecFingerState[1]==1
				&&vecFingerState[2]==1
				&&vecFingerState[3]==1
				&&vecFingerState[4]!=1)
			{
				_dRotX=90;
				_dRotY=0;
				_dRotZ=0;
			}
			if (vecFingerState[0]!=1
				&&vecFingerState[1]==1
				&&vecFingerState[2]==1
				&&vecFingerState[3]==1
				&&vecFingerState[4]==1)
			{
				_dRotX=-90;
				_dRotY=0;
				_dRotZ=0;
			}
			if (vecFingerState[0]==1
				&&vecFingerState[1]==1
				&&vecFingerState[2]==1
				&&vecFingerState[3]==1
				&&vecFingerState[4]==1)
			{
				_dRotX=0;
				_dRotY=90;
				_dRotZ=0;
			}
			if (vecFingerState[0]==1
				&&vecFingerState[1]!=1
				&&vecFingerState[2]!=1
				&&vecFingerState[3]!=1
				&&vecFingerState[4]==1)
			{
				_dRotX=0;
				_dRotY=-90;
				_dRotZ=0;
			}
			_dZoom=-3;
		}
		else	if (_CurState==OpState::Move)
		{
			float dStep=0.05;
			float dDiffX=0;
			float dDiffY=0;

			if (vecFingerState[0]!=1
				&&vecFingerState[1]==1
				&&vecFingerState[2]!=1
				&&vecFingerState[3]!=1
				&&vecFingerState[4]!=1)
			{
				dDiffX=-dStep;
				dDiffY=0;
			}
			if (vecFingerState[0]!=1
				&&vecFingerState[1]==1
				&&vecFingerState[2]==1
				&&vecFingerState[3]!=1
				&&vecFingerState[4]!=1)
			{
				dDiffX=dStep;
				dDiffY=0;
			}
			if (vecFingerState[0]!=1
				&&vecFingerState[1]==1
				&&vecFingerState[2]==1
				&&vecFingerState[3]==1
				&&vecFingerState[4]!=1)
			{
				dDiffX=0;
				dDiffY=-dStep;
			}
			if (vecFingerState[0]!=1
				&&vecFingerState[1]==1
				&&vecFingerState[2]==1
				&&vecFingerState[3]==1
				&&vecFingerState[4]==1)
			{
				dDiffX=0;
				dDiffY=dStep;
			}

			_dMoveX+=dDiffX;
			_dMoveY+=dDiffY;
		}
		else	if (_CurState==OpState::Rotate)
		{
			float dStep=2;
			float dDiffX=0;
			float dDiffY=0;
			float dDiffZ=0;
			if (vecFingerState[0]!=1)
			{
				if (vecFingerState[1]==1)
				{
					dDiffX+=dStep;
				}
				if (vecFingerState[2]==1)
				{
					dDiffY+=dStep;
				}
				if (vecFingerState[3]==1)
				{
					dDiffZ+=dStep;
				}
			}
			else if (vecFingerState[0]==1)
			{
				if (vecFingerState[1]==1)
				{
					dDiffX+=-dStep;
				}
				if (vecFingerState[2]==1)
				{
					dDiffY+=-dStep;
				}
				if (vecFingerState[3]==1)
				{
					dDiffZ+=-dStep;
				}
			}

			_dRotX+=dDiffX;
			_dRotY+=dDiffY;
			_dRotZ+=dDiffZ;
		}
		else	if (_CurState==OpState::Zoom)
		{
			float dStep=0.1;
			float dDiff=0;
			//float dMaxStep=dStep*3;
			//float dMinStep=dStep*0.5;
			//double dAngle=ComputeThumbAngle();
			//float dDiffTemp=dAngle/90*dMaxStep;
			//if(dDiffTemp<dMinStep) dDiffTemp= dMinStep;
			//if(dDiffTemp>dMaxStep) dDiffTemp= dMaxStep;

			if (vecFingerState[0]!=1&&
				vecFingerState[1]==1
				&&vecFingerState[2]!=1
				&&vecFingerState[3]!=1
				&&vecFingerState[4]!=1)
			{
				dDiff=-dStep;//-dDiffTemp;
			}
			if (vecFingerState[0]==1&&
				vecFingerState[1]==1
				&&vecFingerState[2]!=1
				&&vecFingerState[3]!=1
				&&vecFingerState[4]!=1)
			{
				dDiff=dStep;//dDiffTemp;
			}

			_dZoom+=dDiff;
			if (_dZoom>-1.5)
			{
				_dZoom=-1.5;
			}
			if (_dZoom<-15)
			{
				_dZoom=-15;
			}
		}
	}

	updateGL();
}
/*
void GlWndMain::timerEvent(QTimerEvent *)
{
	int iCurTime=GetSysTime_number();
	cout<<"回调 耗时(ms):"<<iCurTime-_iLastTime<<endl;
	_iLastTime=iCurTime;

	//借助MediaPip的Python版本完成手部关键点识别
	_mMainCapture.read(_mFrameImage); 
	if (_access("Data\\CreatedImage" , 0)==0)
	{
		rename("Data\\CreatedImage" , "Data\\CreatingImage");
	}
	cv::flip(_mFrameImage,_mFrameImage,1);
	cv::Mat mTransBtidgeImage=_mFrameImage.clone();
	cv::resize(mTransBtidgeImage,mTransBtidgeImage,cv::Size(320,240));
	//cvtColor(mTransBtidgeImage,mTransBtidgeImage,CV_BGR2GRAY);
	imwrite("Data\\TempImg.jpg",mTransBtidgeImage);
	if (_access("Data\\CreatingImage" , 0)==0)
	{
		rename("Data\\CreatingImage" , "Data\\CreatedImage");
	}
	_vecAllHandPoints=_handPointsCls.GetAllHandPoints();
	if(_handPointsCls.GetHandCount()==0)
	{
		updateGL();
		return;
	}
	Point3D mTipPos_img=_handPointsCls.GetTipOfIndexFinger(false);
	if (mTipPos_img.X<0||mTipPos_img.Y<0)
	{
		return;
	}
	//5 number(0 or 1) :thumb, index finger, middle finger, ring finger, little finger
	vector<int> vecFingerState=_handPointsCls.GetFingerState(false);
	if (vecFingerState.size()!=5)
	{
		return;
	}

	std::string strCurFingereState=ComputeStateSignal(vecFingerState);

	cout<<vecFingerState[0];
	cout<<vecFingerState[1];
	cout<<vecFingerState[2];
	cout<<vecFingerState[3];
	cout<<vecFingerState[4];
	cout<<endl;


	Point3D mTipPos;
	{
		//转换到屏幕坐标
		//图像[640,480]中有效区域[iImgRange_Xmin,iImgRange_Xmax][iImgRange_Ymin,iImgRange_Ymax]，映射到屏幕上的区域[0,_iScreenWidth][0,_iScreenHight]
		int iImgRange_Xmin=100;
		int iImgRange_Xmax=_iScreenWidth-100;
		int iImgRange_Ymin=0;
		int iImgRange_Ymax=_iScreenHight-80;
		int iRangeWidth=iImgRange_Xmax-iImgRange_Xmin;
		int iRangeHight=iImgRange_Ymax-iImgRange_Ymin;
		mTipPos.X=(mTipPos_img.X-iImgRange_Xmin)*_iScreenWidth/iRangeWidth;
		mTipPos.Y=(mTipPos_img.Y-iImgRange_Ymin)*_iScreenHight/iRangeHight;
		if(mTipPos.X <0)
			mTipPos.X=0;
		if (mTipPos.X>_iScreenWidth)
			mTipPos.X=_iScreenWidth;
		if(mTipPos.Y <0)
			mTipPos.Y=0;
		if (mTipPos.Y>_iScreenHight)
			mTipPos.Y=_iScreenHight;
	}

	Point3D mMousePos;
	mMousePos.X = _mLastPos.X + (mTipPos.X - _mLastPos.X) / 5;
	mMousePos.Y = _mLastPos.Y + (mTipPos.Y - _mLastPos.Y) / 5;
	//删除微小移动误差
	float fDiff=mMousePos.DistanceTo(_mLastPos);
	//cout<<"error Diff:"<<fDiff<<endl;
	if (fDiff<3)
	{
		return;
	}
	GLfloat dx = GLfloat(mMousePos.X-_mLastPos.X)/width();
	GLfloat dy = GLfloat(mMousePos.Y-_mLastPos.Y)/height();
	_mLastPos=mMousePos;
	//cout<<"dx"<<dx<<endl;
	//cout<<"dy"<<dy<<endl;


	//状态变迁
	if (strCurFingereState=="00000" || strCurFingereState=="11111")
	{
		if (strCurFingereState=="11111"&&_strLastFingerState=="00000")
		{
			if (_CurState==OpState::Initial)
			{
				cout<<"====Initial=FrontView===="<<endl;
				_CurState=OpState::FrontView;
			}
			else if (_CurState==OpState::FrontView)
			{
				cout<<"====FrontView=Move===="<<endl;
				_CurState=OpState::Move;
			}
			else if (_CurState==OpState::Move)
			{
				cout<<"====Move=Rotate===="<<endl;
				_CurState=OpState::Rotate;
			}
			else if (_CurState==OpState::Rotate)
			{
				cout<<"====Rotate=Zoom===="<<endl;
				_CurState=OpState::Zoom;
			}
			else if (_CurState==OpState::Zoom)
			{
				cout<<"====Zoom=FrontView===="<<endl;
				_CurState=OpState::FrontView;
			}
		}
		else		if (strCurFingereState=="11111"&&_CurState==OpState::FrontView)
		{
			_dRotX=0;
			_dRotY=90;
			_dRotZ=0;
			_iFrontViewIndex=5;
		}
		_strLastFingerState=strCurFingereState;
	}
	else
	{
		_bKeepState=false;

		//cout<<_CurState<<endl;

		//FrontView
		if (_CurState==OpState::Initial)
		{
			_CurState=OpState::FrontView;
		}
		if (_CurState==OpState::FrontView)
		{
			if (vecFingerState[0]!=1
				&&vecFingerState[1]==1
				&&vecFingerState[2]!=1
				&&vecFingerState[3]!=1
				&&vecFingerState[4]!=1)
			{
				_dRotX=0;
				_dRotY=0;
				_dRotZ=0;
				_iFrontViewIndex=1;
			}
			if (vecFingerState[0]!=1
				&&vecFingerState[1]==1
				&&vecFingerState[2]==1
				&&vecFingerState[3]!=1
				&&vecFingerState[4]!=1)
			{
				_dRotX=180;
				_dRotY=0;
				_dRotZ=0;
				_iFrontViewIndex=2;
			}
			if (vecFingerState[0]!=1
				&&vecFingerState[1]==1
				&&vecFingerState[2]==1
				&&vecFingerState[3]==1
				&&vecFingerState[4]!=1)
			{
				_dRotX=90;
				_dRotY=0;
				_dRotZ=0;
				_iFrontViewIndex=3;
			}
			if (vecFingerState[0]!=1
				&&vecFingerState[1]==1
				&&vecFingerState[2]==1
				&&vecFingerState[3]==1
				&&vecFingerState[4]==1)
			{
				_dRotX=-90;
				_dRotY=0;
				_dRotZ=0;
				_iFrontViewIndex=4;
			}
			if (vecFingerState[0]==1
				&&vecFingerState[1]==1
				&&vecFingerState[2]==1
				&&vecFingerState[3]==1
				&&vecFingerState[4]==1)
			{
				_dRotX=0;
				_dRotY=90;
				_dRotZ=0;
				_iFrontViewIndex=5;
			}
			if (vecFingerState[0]==1
				&&vecFingerState[1]!=1
				&&vecFingerState[2]!=1
				&&vecFingerState[3]!=1
				&&vecFingerState[4]==1)
			{
				_dRotX=0;
				_dRotY=-90;
				_dRotZ=0;
				_iFrontViewIndex=6;
			}
			_dZoom=-3;
			//if (_iFrontViewIndex>=1&&_iFrontViewIndex<=6)
			//{
			//	_CurState=OpState::Adapting;
			//}
		}
		else	if (_CurState==OpState::Move)
		{

			float dStep=0.05;
			float dDiffX=0;
			float dDiffY=0;

			dDiffX=dx/1;
			dDiffY=-dy/1;

			_dMoveX+=dDiffX;
			_dMoveY+=dDiffY;
		}
		else	if (_CurState==OpState::Rotate)
		{
			float dStep=2;
			float dDiffX=0;
			float dDiffY=0;
			float dDiffZ=0;
			if (vecFingerState[0]!=1)
			{
				if (vecFingerState[1]==1)
				{
					dDiffX+=dStep;
				}
				if (vecFingerState[2]==1)
				{
					dDiffY+=dStep;
				}
				if (vecFingerState[3]==1)
				{
					dDiffZ+=dStep;
				}
			}
			else if (vecFingerState[0]==1)
			{
				if (vecFingerState[1]==1)
				{
					dDiffX+=-dStep;
				}
				if (vecFingerState[2]==1)
				{
					dDiffY+=-dStep;
				}
				if (vecFingerState[3]==1)
				{
					dDiffZ+=-dStep;
				}
			}

			_dRotX+=dDiffX;
			_dRotY+=dDiffY;
			_dRotZ+=dDiffZ;
		}
		else	if (_CurState==OpState::Zoom)
		{
			float dStep=0.1;
			float dDiff=0;
			//float dMaxStep=dStep*3;
			//float dMinStep=dStep*0.5;
			//double dAngle=ComputeThumbAngle();
			//float dDiffTemp=dAngle/90*dMaxStep;
			//if(dDiffTemp<dMinStep) dDiffTemp= dMinStep;
			//if(dDiffTemp>dMaxStep) dDiffTemp= dMaxStep;

			if (vecFingerState[0]!=1&&
				vecFingerState[1]==1
				&&vecFingerState[2]!=1
				&&vecFingerState[3]!=1
				&&vecFingerState[4]!=1)
			{
				dDiff=-dStep;//-dDiffTemp;
			}
			if (vecFingerState[0]==1&&
				vecFingerState[1]==1
				&&vecFingerState[2]!=1
				&&vecFingerState[3]!=1
				&&vecFingerState[4]!=1)
			{
				dDiff=dStep;//dDiffTemp;
			}

			_dZoom+=dDiff;
			if (_dZoom>-1.5)
			{
				_dZoom=-1.5;
			}
			if (_dZoom<-15)
			{
				_dZoom=-15;
			}
		}
	}

	updateGL();
}
*/
bool ImageCVToGL(const Mat & mImageCV , GLubyte * & pixels)
{
	//int iTime1=GetSysTime_number();

	int imageWidth=mImageCV.cols;
	int imageHeight=mImageCV.rows;
	int iSize=imageWidth*imageHeight*3;
	pixels=new GLubyte[iSize];
	memcpy(pixels , mImageCV.data , iSize*sizeof(unsigned char));

	//int iTime2=GetSysTime_number();
	//cout<<"ImageCVToGL time taken(ms):"<<iTime2-iTime1<<endl;

	return true;
}


//mouse callback
void GlWndMain::mousePressEvent(QMouseEvent *e)
{
	setCursor(Qt::OpenHandCursor);
	lastPos = e->pos();
}
void GlWndMain::mouseReleaseEvent(QMouseEvent *e)
{
	setCursor(Qt::ArrowCursor);
	lastPos = e->pos();
}
void GlWndMain::mouseMoveEvent(QMouseEvent *e)
{
	GLfloat dx = GLfloat(e->x()-lastPos.x())/width();
	GLfloat dy = GLfloat(e->y()-lastPos.y())/height();
	lastPos = e->pos();
	if (e->buttons()&Qt::LeftButton)
	{
		_dRotX -= dy*50;
		_dRotY += dx*50;
		updateGL();
	}
	else if (e->buttons()&Qt::RightButton)
	{
		_dRotX -= dy*200;
		_dRotY += dx*200;
		updateGL();
	}
	else if (e->buttons()&Qt::MiddleButton)
	{
		_dMoveX += dx*5;
		_dMoveY -= dy*5;
		updateGL();
	}
}
void GlWndMain::wheelEvent(QWheelEvent *e)
{
	GLfloat zValue = e->delta();
	_dZoom += zValue*0.005;
	if (_dZoom>1.0)
	{
		_dZoom = 1.0;
	}
	updateGL();
}

void GlWndMain::loadGLTextures()
{
	int iStartTime=GetSysTime_number();
	cout<<__FUNCTION__<<endl;
	glGenTextures(6, texture);//创建6个纹理
	for (int i=0;i<6;i++)
	{
		string strTitle=IntToStr(i);
		string sTexPath="Data\\"+strTitle+".jpg";
		QString qsTexPath = QString::fromStdString(sTexPath);
		QImage mTex,mBuf;
		if (!mBuf.load(qsTexPath))
		{
			cout<<"cannot open image : "<<qsTexPath.toStdString()<<endl;
			QImage dummy(128,128,QImage::Format_ARGB32);
			dummy.fill(Qt::lightGray);
			mBuf = dummy;//如果载入不成功，自动生成颜色图片
		}
		mTex = QGLWidget::convertToGLFormat(mBuf);//QGLWidget提供的专门转换图片的静态函数
		mTex=mTex.rgbSwapped();
		glBindTexture(GL_TEXTURE_2D,texture[i]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D,0,3,mTex.width(),mTex.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,mTex.bits());
		//glTexSubImage2D(GL_TEXTURE_2D,0,3,mTex.width(),mTex.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,mTex.bits());
		cout<<"更新视图耗时ms："<<GetSysTime_number()-iStartTime<<endl;
		
	}
	glEnable(GL_TEXTURE_2D);
}

//draw AR cube
void GlWndMain::DrawARBox()
{
	//glTexImage2D在创建图片纹理时，整体变得很暗，所以先全部使用视频贴图
	//cout<<__FUNCTION__<<endl;

	int iStartTime=GetSysTime_number();

	if(_bARVideoOK)
	{
		int iTime1=GetSysTime_number();
		if(videoOfAR0.get(CV_CAP_PROP_POS_FRAMES) == _iFrameCount_Video0)	videoOfAR0.set(CV_CAP_PROP_POS_FRAMES, 0);		videoOfAR0>>_mFaceFrame0 ; 
		if(videoOfAR1.get(CV_CAP_PROP_POS_FRAMES) == _iFrameCount_Video1)	videoOfAR1.set(CV_CAP_PROP_POS_FRAMES, 0);		videoOfAR1>>_mFaceFrame1 ; 
		if(videoOfAR2.get(CV_CAP_PROP_POS_FRAMES) == _iFrameCount_Video2)	videoOfAR2.set(CV_CAP_PROP_POS_FRAMES, 0);		videoOfAR2>>_mFaceFrame2 ; 
		if(videoOfAR3.get(CV_CAP_PROP_POS_FRAMES) == _iFrameCount_Video3)	videoOfAR3.set(CV_CAP_PROP_POS_FRAMES, 0);		videoOfAR3>>_mFaceFrame3 ; 
		if(videoOfAR4.get(CV_CAP_PROP_POS_FRAMES) == _iFrameCount_Video4)	videoOfAR4.set(CV_CAP_PROP_POS_FRAMES, 0);		videoOfAR4>>_mFaceFrame4 ; 
		if(videoOfAR5.get(CV_CAP_PROP_POS_FRAMES) == _iFrameCount_Video5)	videoOfAR5.set(CV_CAP_PROP_POS_FRAMES, 0);		videoOfAR5>>_mFaceFrame5 ; 
		//int iTime2=GetSysTime_number();
		//cout<<"读取帧数据 耗时(ms):"<<iTime2-iTime1<<endl;

		GLubyte*imageData0=NULL;ImageCVToGL(_mFaceFrame0,imageData0);
		GLubyte*imageData1=NULL;ImageCVToGL(_mFaceFrame1,imageData1);
		GLubyte*imageData2=NULL;ImageCVToGL(_mFaceFrame2,imageData2);
		GLubyte*imageData3=NULL;ImageCVToGL(_mFaceFrame3,imageData3);
		GLubyte*imageData4=NULL;ImageCVToGL(_mFaceFrame4,imageData4);
		GLubyte*imageData5=NULL;ImageCVToGL(_mFaceFrame5,imageData5);
		//int iTime21=GetSysTime_number();
		//cout<<"ImageCVToGL 耗时(ms):"<<iTime21-iTime2<<endl;

		GLuint videoTextur0;		glGenTextures(1, &videoTextur0);
		GLuint videoTextur1;		glGenTextures(1, &videoTextur1);
		GLuint videoTextur2;		glGenTextures(1, &videoTextur2);
		GLuint videoTextur3;		glGenTextures(1, &videoTextur3);
		GLuint videoTextur4;		glGenTextures(1, &videoTextur4);
		GLuint videoTextur5;		glGenTextures(1, &videoTextur5);
		//int iTime22=GetSysTime_number();
		//cout<<"glGenTextures 耗时(ms):"<<iTime22-iTime21<<endl;

		// 前面
		{
			glBindTexture(GL_TEXTURE_2D, videoTextur0);//建立一个绑定到目标纹理的纹理
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//int iTime5=GetSysTime_number();
			glTexImage2D(GL_TEXTURE_2D, 0, 3, _mFaceFrame0.cols, _mFaceFrame0.rows, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, imageData0);
			//int iTime6=GetSysTime_number();
			//cout<<"glTexImage2D 耗时ms:"<<iTime6-iTime5<<endl;
		}

		glBindTexture(GL_TEXTURE_2D, videoTextur0);
		glBegin(GL_QUADS);
		glNormal3f(0,0,1);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的左下
		glTexCoord2f(1.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的右下
		glTexCoord2f(1.0f, 1.0f); glVertex3f( dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的右上
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的左上
		glEnd();

		//int iTime3=GetSysTime_number();
		//cout<<"面0 耗时(ms):"<<iTime3-iTime22<<endl;

		// 后面
		{
			glBindTexture(GL_TEXTURE_2D, videoTextur1);//建立一个绑定到目标纹理的纹理
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//int iTime5=GetSysTime_number();
			glTexImage2D(GL_TEXTURE_2D, 0, 3, _mFaceFrame1.cols, _mFaceFrame1.rows, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, imageData1);
			//int iTime6=GetSysTime_number();
			//cout<<"glTexImage2D 耗时ms:"<<iTime6-iTime5<<endl;
		}

		glBindTexture(GL_TEXTURE_2D,videoTextur1);
		glBegin(GL_QUADS);
		glNormal3f(0,0,-1);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右下
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右上
		glTexCoord2f(0.0f, 1.0f); glVertex3f( dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左上
		glTexCoord2f(0.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左下
		glEnd();

		//int iTime4=GetSysTime_number();
		//cout<<"面1 耗时(ms):"<<iTime4-iTime3<<endl;

		// 顶面
		{
			glBindTexture(GL_TEXTURE_2D, videoTextur2);//建立一个绑定到目标纹理的纹理
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//int iTime5=GetSysTime_number();
			glTexImage2D(GL_TEXTURE_2D, 0, 3, _mFaceFrame2.cols, _mFaceFrame2.rows, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, imageData2);
			//int iTime6=GetSysTime_number();
			//cout<<"glTexImage2D 耗时ms:"<<iTime6-iTime5<<endl;
		}

		glBindTexture(GL_TEXTURE_2D, videoTextur2);
		glBegin(GL_QUADS);
		glNormal3f(0,1,0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左上
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的左下
		glTexCoord2f(0.0f, 1.0f); glVertex3f( dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的右下
		glTexCoord2f(0.0f, 0.0f); glVertex3f( dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右上
		glEnd();

		//int iTime5=GetSysTime_number();
		//cout<<"面2 耗时(ms):"<<iTime5-iTime4<<endl;

		// 底面
		{
			glBindTexture(GL_TEXTURE_2D, videoTextur3);//建立一个绑定到目标纹理的纹理
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//int iTime5=GetSysTime_number();
			glTexImage2D(GL_TEXTURE_2D, 0, 3, _mFaceFrame3.cols, _mFaceFrame3.rows, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, imageData3);
			//int iTime6=GetSysTime_number();
			//cout<<"glTexImage2D 耗时ms:"<<iTime6-iTime5<<endl;
		}

		glBindTexture(GL_TEXTURE_2D, videoTextur3);
		glBegin(GL_QUADS);
		glNormal3f(0,-1,0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右上
		glTexCoord2f(1.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左上
		glTexCoord2f(1.0f, 1.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的左下
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的右下
		glEnd();

		//int iTime6=GetSysTime_number();
		//cout<<"面3 耗时(ms):"<<iTime6-iTime5<<endl;

		// 右面
		{
			glBindTexture(GL_TEXTURE_2D, videoTextur4);//建立一个绑定到目标纹理的纹理
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//int iTime5=GetSysTime_number();
			glTexImage2D(GL_TEXTURE_2D, 0, 3, _mFaceFrame4.cols, _mFaceFrame4.rows, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, imageData4);
			//int iTime6=GetSysTime_number();
			//cout<<"glTexImage2D 耗时ms:"<<iTime6-iTime5<<endl;
		}

		glBindTexture(GL_TEXTURE_2D, videoTextur4);
		glBegin(GL_QUADS);
		glNormal3f(1,0,0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右下
		glTexCoord2f(1.0f, 0.0f); glVertex3f( dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右上
		glTexCoord2f(1.0f, 1.0f); glVertex3f( dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的左上
		glTexCoord2f(0.0f, 1.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的左下
		glEnd();

		//int iTime7=GetSysTime_number();
		//cout<<"面4 耗时(ms):"<<iTime7-iTime6<<endl;

		// 左面
		{
			glBindTexture(GL_TEXTURE_2D, videoTextur5);//建立一个绑定到目标纹理的纹理
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//int iTime5=GetSysTime_number();
			glTexImage2D(GL_TEXTURE_2D, 0, 3, _mFaceFrame5.cols, _mFaceFrame5.rows, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, imageData5);
			//int iTime6=GetSysTime_number();
			//cout<<"glTexImage2D 耗时ms:"<<iTime6-iTime5<<endl;
		}

		glBindTexture(GL_TEXTURE_2D, videoTextur5);
		glBegin(GL_QUADS);
		glNormal3f(-1,0,0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左下
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的右下
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的右上
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左上
		glEnd();

		//int iTime8=GetSysTime_number();
		//cout<<"面5 耗时(ms):"<<iTime8-iTime7<<endl;

		glDeleteTextures(1, &videoTextur0);
		glDeleteTextures(1, &videoTextur1);
		glDeleteTextures(1, &videoTextur2);
		glDeleteTextures(1, &videoTextur3);
		glDeleteTextures(1, &videoTextur4);
		glDeleteTextures(1, &videoTextur5);

		free(imageData0);
		free(imageData1);
		free(imageData2);
		free(imageData3);
		free(imageData4);
		free(imageData5);

		//int iTime9=GetSysTime_number();
		//cout<<"内存管理 耗时(ms):"<<iTime9-iTime8<<endl;

	}

	cout<<"DrawARBox 耗时(ms):"<<GetSysTime_number()-iStartTime<<endl;

	_bCreatedArBox=true;
}
void GlWndMain::DrawARBox2()
{
	//glTexImage2D在创建图片纹理时，整体变得很暗，所以先全部使用视频贴图
	//cout<<__FUNCTION__<<endl;

	int iStartTime=GetSysTime_number();
	//如果加载视频成功，就把视频显示在方块上，否则显示图片数据
	if(_bARVideoOK)
	{
		QImage mBuf0, mTex0;
		QImage mBuf1, mTex1;
		QImage mBuf2, mTex2;
		QImage mBuf3, mTex3;
		QImage mBuf4, mTex4;
		QImage mBuf5, mTex5;
		if (1)
		{
			if(videoOfAR0.get(CV_CAP_PROP_POS_FRAMES) == _iFrameCount_Video0)	videoOfAR0.set(CV_CAP_PROP_POS_FRAMES, 0);		videoOfAR0>>_mFaceFrame0 ; 
			if(videoOfAR1.get(CV_CAP_PROP_POS_FRAMES) == _iFrameCount_Video1)	videoOfAR1.set(CV_CAP_PROP_POS_FRAMES, 0);		videoOfAR1>>_mFaceFrame1 ; 
			if(videoOfAR2.get(CV_CAP_PROP_POS_FRAMES) == _iFrameCount_Video2)	videoOfAR2.set(CV_CAP_PROP_POS_FRAMES, 0);		videoOfAR2>>_mFaceFrame2 ; 
			if(videoOfAR3.get(CV_CAP_PROP_POS_FRAMES) == _iFrameCount_Video3)	videoOfAR3.set(CV_CAP_PROP_POS_FRAMES, 0);		videoOfAR3>>_mFaceFrame3 ; 
			if(videoOfAR4.get(CV_CAP_PROP_POS_FRAMES) == _iFrameCount_Video4)	videoOfAR4.set(CV_CAP_PROP_POS_FRAMES, 0);		videoOfAR4>>_mFaceFrame4 ; 
			if(videoOfAR5.get(CV_CAP_PROP_POS_FRAMES) == _iFrameCount_Video5)	videoOfAR5.set(CV_CAP_PROP_POS_FRAMES, 0);		videoOfAR5>>_mFaceFrame5 ; 

			//将Mat类型转换成QImage
			mBuf0 = QImage((const unsigned char*)_mFaceFrame0.data, _mFaceFrame0.cols, _mFaceFrame0.rows, _mFaceFrame0.cols * _mFaceFrame0.channels(), QImage::Format_RGB888);
			mBuf1 = QImage((const unsigned char*)_mFaceFrame1.data, _mFaceFrame1.cols, _mFaceFrame1.rows, _mFaceFrame1.cols * _mFaceFrame1.channels(), QImage::Format_RGB888);
			mBuf2 = QImage((const unsigned char*)_mFaceFrame2.data, _mFaceFrame2.cols, _mFaceFrame2.rows, _mFaceFrame2.cols * _mFaceFrame2.channels(), QImage::Format_RGB888);
			mBuf3 = QImage((const unsigned char*)_mFaceFrame3.data, _mFaceFrame3.cols, _mFaceFrame3.rows, _mFaceFrame3.cols * _mFaceFrame3.channels(), QImage::Format_RGB888);
			mBuf4 = QImage((const unsigned char*)_mFaceFrame4.data, _mFaceFrame4.cols, _mFaceFrame4.rows, _mFaceFrame4.cols * _mFaceFrame4.channels(), QImage::Format_RGB888);
			mBuf5 = QImage((const unsigned char*)_mFaceFrame5.data, _mFaceFrame5.cols, _mFaceFrame5.rows, _mFaceFrame5.cols * _mFaceFrame5.channels(), QImage::Format_RGB888);
			if (mBuf0.isNull())	return;
			if (mBuf1.isNull())	return;
			if (mBuf2.isNull())	return;
			if (mBuf3.isNull())	return;
			if (mBuf4.isNull())	return;
			if (mBuf5.isNull())	return;
			mTex0 = QGLWidget::convertToGLFormat(mBuf0);
			mTex1 = QGLWidget::convertToGLFormat(mBuf1);
			mTex2 = QGLWidget::convertToGLFormat(mBuf2);
			mTex3 = QGLWidget::convertToGLFormat(mBuf3);
			mTex4 = QGLWidget::convertToGLFormat(mBuf4);
			mTex5 = QGLWidget::convertToGLFormat(mBuf5);
		}
		//else
		//{
		//	string sTexPath="Data\\2.jpg";
		//	QString qsTexPath = QString::fromStdString(sTexPath);
		//	if (!mBuf.load(qsTexPath))
		//	{
		//		cout<<"cannot open image : "<<qsTexPath.toStdString()<<endl;
		//		QImage dummy(228,228,QImage::Format_ARGB32);
		//		dummy.fill(Qt::lightGray);
		//		mBuf = dummy;//如果载入不成功，自动生成颜色图片
		//	}
		//	mTex = QGLWidget::convertToGLFormat(mBuf);//QGLWidget提供的专门转换图片的静态函数
		//	mTex=mTex.rgbSwapped();
		//}

		GLuint videoTextur0;		glGenTextures(1, &videoTextur0);
		GLuint videoTextur1;		glGenTextures(1, &videoTextur1);
		GLuint videoTextur2;		glGenTextures(1, &videoTextur2);
		GLuint videoTextur3;		glGenTextures(1, &videoTextur3);
		GLuint videoTextur4;		glGenTextures(1, &videoTextur4);
		GLuint videoTextur5;		glGenTextures(1, &videoTextur5);

		// 前面
		{
			glBindTexture(GL_TEXTURE_2D, videoTextur0);//建立一个绑定到目标纹理的纹理
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, 3, mTex0.width(), mTex0.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, mTex0.bits());
		}

		glBindTexture(GL_TEXTURE_2D, videoTextur0);
		glBegin(GL_QUADS);
		glNormal3f(0,0,1);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的左下
		glTexCoord2f(1.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的右下
		glTexCoord2f(1.0f, 1.0f); glVertex3f( dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的右上
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的左上
		glEnd();

		// 后面

		{
			glBindTexture(GL_TEXTURE_2D, videoTextur1);//建立一个绑定到目标纹理的纹理
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, 3, mTex1.width(), mTex1.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, mTex1.bits());
		}

		glBindTexture(GL_TEXTURE_2D,videoTextur1);
		glBegin(GL_QUADS);
		glNormal3f(0,0,-1);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右下
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右上
		glTexCoord2f(0.0f, 1.0f); glVertex3f( dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左上
		glTexCoord2f(0.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左下
		glEnd();

		// 顶面

		{
			glBindTexture(GL_TEXTURE_2D, videoTextur2);//建立一个绑定到目标纹理的纹理
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, 3, mTex2.width(), mTex2.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, mTex2.bits());
		}

		glBindTexture(GL_TEXTURE_2D, videoTextur2);
		glBegin(GL_QUADS);
		glNormal3f(0,1,0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左上
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的左下
		glTexCoord2f(0.0f, 1.0f); glVertex3f( dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的右下
		glTexCoord2f(0.0f, 0.0f); glVertex3f( dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右上
		glEnd();

		// 底面

		{
			glBindTexture(GL_TEXTURE_2D, videoTextur3);//建立一个绑定到目标纹理的纹理
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, 3, mTex3.width(), mTex3.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, mTex3.bits());
		}

		glBindTexture(GL_TEXTURE_2D, videoTextur3);
		glBegin(GL_QUADS);
		glNormal3f(0,-1,0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右上
		glTexCoord2f(1.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左上
		glTexCoord2f(1.0f, 1.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的左下
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的右下
		glEnd();

		// 右面

		{
			glBindTexture(GL_TEXTURE_2D, videoTextur4);//建立一个绑定到目标纹理的纹理
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, 3, mTex4.width(), mTex4.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, mTex4.bits());
		}

		glBindTexture(GL_TEXTURE_2D, videoTextur4);
		glBegin(GL_QUADS);
		glNormal3f(1,0,0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右下
		glTexCoord2f(1.0f, 0.0f); glVertex3f( dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右上
		glTexCoord2f(1.0f, 1.0f); glVertex3f( dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的左上
		glTexCoord2f(0.0f, 1.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的左下
		glEnd();

		// 左面

		{
			glBindTexture(GL_TEXTURE_2D, videoTextur5);//建立一个绑定到目标纹理的纹理
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, 3, mTex5.width(), mTex5.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, mTex5.bits());
		}

		glBindTexture(GL_TEXTURE_2D, videoTextur5);
		glBegin(GL_QUADS);
		glNormal3f(-1,0,0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左下
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的右下
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的右上
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左上
		glEnd();

		glDeleteTextures(1, &videoTextur0);
		glDeleteTextures(1, &videoTextur1);
		glDeleteTextures(1, &videoTextur2);
		glDeleteTextures(1, &videoTextur3);
		glDeleteTextures(1, &videoTextur4);
		glDeleteTextures(1, &videoTextur5);
	}
	else
	{
		// 前面
		glBindTexture(GL_TEXTURE_2D,texture[0]);
		glBegin(GL_QUADS);
		glNormal3f(0,0,2);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的左下
		glTexCoord2f(2.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的右下
		glTexCoord2f(2.0f, 2.0f); glVertex3f( dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的右上
		glTexCoord2f(0.0f, 2.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的左上
		glEnd();

		// 后面
		glBindTexture(GL_TEXTURE_2D,texture[2]);
		glBegin(GL_QUADS);
		glNormal3f(0,0,-2);
		glTexCoord2f(2.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右下
		glTexCoord2f(2.0f, 2.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右上
		glTexCoord2f(0.0f, 2.0f); glVertex3f( dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左上
		glTexCoord2f(0.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左下
		glEnd();

		// 顶面
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glBegin(GL_QUADS);
		glNormal3f(0,2,0);
		glTexCoord2f(2.0f, 0.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左上
		glTexCoord2f(2.0f, 2.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的左下
		glTexCoord2f(0.0f, 2.0f); glVertex3f( dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的右下
		glTexCoord2f(0.0f, 0.0f); glVertex3f( dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右上
		glEnd();

		// 底面
		glBindTexture(GL_TEXTURE_2D, texture[3]);
		glBegin(GL_QUADS);
		glNormal3f(0,-2,0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右上
		glTexCoord2f(2.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左上
		glTexCoord2f(2.0f, 2.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的左下
		glTexCoord2f(0.0f, 2.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的右下
		glEnd();

		// 右面
		glBindTexture(GL_TEXTURE_2D, texture[4]);
		glBegin(GL_QUADS);
		glNormal3f(2,0,0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右下
		glTexCoord2f(2.0f, 0.0f); glVertex3f( dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的右上
		glTexCoord2f(2.0f, 2.0f); glVertex3f( dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的左上
		glTexCoord2f(0.0f, 2.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的左下
		glEnd();

		// 左面
		glBindTexture(GL_TEXTURE_2D, texture[5]);
		glBegin(GL_QUADS);
		glNormal3f(-2,0,0);
		glTexCoord2f(2.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左下
		glTexCoord2f(2.0f, 2.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, dARBoxWidth); // 纹理和四边形的右下
		glTexCoord2f(0.0f, 2.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, dARBoxWidth); // 纹理和四边形的右上
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, -dARBoxWidth); // 纹理和四边形的左上
		glEnd();
	}

	cout<<"DrawARBox 耗时(ms):"<<GetSysTime_number()-iStartTime<<endl;

	_bCreatedArBox=true;
}

//Action
void GlWndMain::Run()
{
	_bRun=true;
}
void GlWndMain::Stop()
{
	_bRun=false;
}
void GlWndMain::Pause()
{
	_bPause=true;
}
void GlWndMain::Continue()
{
	_bPause=false;
}
void GlWndMain::OpenHyaline()
{
	_bHyaline=true;
}
void GlWndMain::CloseHyaline()
{
	_bHyaline=false;
}
void GlWndMain::OpenFog()
{
	_bFog=true;
}
void GlWndMain::CloseFog()
{
	_bFog=false;
}
void GlWndMain::SetRotateStepX(GLfloat fStep)
{
	stepRotX=fStep;
}
void GlWndMain::SetRotateStepY(GLfloat fStep)
{
	stepRotY=fStep;
}
void GlWndMain::SetRotateStepZ(GLfloat fStep)
{
	stepRotZ=fStep;
}
void GlWndMain::OpenVideo()
{
	_bOpenAR=true;
}
void GlWndMain::CloseVideo()
{
	_bOpenAR=false;
}
