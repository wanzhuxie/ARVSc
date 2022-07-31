/*
����궨
����ͶӰ��ϵ����GLͶӰ����
���ñ�ǽǵ�����������ϵ�µ�����
����ͼ������ϵ�±�ǽǵ��Ӧ������
������������ϵ�µ����ꡢͼ������ϵ�±�ǽǵ��Ӧ�����꣬�Լ�����ڲΣ�ʹ��cv::solvePnP����GLģ�ͱ任����
�ֱ�����GL��ͶӰ����ģ����ͼ����
�ڵ�ǰ�����»���������
��������ĸ�����������ͼ
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

BOOL GestureEqual(const vector<int>& vecFingerState , const std::string & strTargetState)
{
	return FALSE;
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
	,_iLastSumZ(0)
	,_bShowHandPoints(true)
{
	
	bARVidioOK=FALSE;
	videoOfAR=VideoCapture("Data/video.mp4");
	if (videoOfAR.isOpened())
	{
		bARVidioOK=true;
		_iFrameCount_Video1=videoOfAR.get(CV_CAP_PROP_FRAME_COUNT);
	}
	else
	{
		bARVidioOK=false;
	}

	_dRotX = _dRotY = _dRotZ = 0;
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
	startTimer(5);

	//������
	if (!_handPointsCls.Init())
	{
		cout<<"error on _handPointsCls.Init"<<endl;
		return ;
	}

}
GlWndMain::~GlWndMain()
{
	_handPointsCls.Close();
}
void GlWndMain::initializeGL()
{

	_mMainCapture=VideoCapture(0);
	//_mMainCapture=VideoCapture(1);

	glEnable(GL_TEXTURE_2D);//��������
	glEnable(GL_COLOR_MATERIAL);//��������ɫ��������
	glShadeModel(GL_SMOOTH);//��Ӱƽ��
	glClearColor(1,1,1,0.5);//���������Ļʱ��ʹ�õ���ɫ

	glClearDepth(1.0);//������Ȼ���
	glEnable(GL_DEPTH_TEST);//������Ȳ���
	glDepthFunc(GL_LEQUAL);//������Ȳ��Ե�����

	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);//ͶӰ����
	glPolygonMode(GL_BACK,GL_FILL);//����
	glPolygonMode(GL_FRONT,GL_FILL);//ǰ��

	//���ù�Դ
	GLfloat lightAmbient[4] = {0.5,0.5,0.5,1.0};
	GLfloat lightDiffuse[4] = {1.0,1.0,1.0,1.0};
	GLfloat lightPosition[4] = {0.0,0.0,2.0,1.0};
	//����趨//�������Ч��,���εݽ�
	GLuint fogMode[3] = {GL_EXP,GL_EXP2,GL_LINEAR};
	GLfloat fogColor[4] = {1,1,1,0.3};

	//��
	glLightfv(GL_LIGHT1,GL_AMBIENT,lightAmbient);
	glLightfv(GL_LIGHT1,GL_DIFFUSE,lightDiffuse);
	glLightfv(GL_LIGHT1,GL_POSITION,lightPosition);
	glEnable(GL_LIGHT1);

	//��
	glFogi(GL_FOG_MODE,fogMode[0]);
	glFogfv(GL_FOG_COLOR,fogColor);
	glFogf(GL_FOG_DENSITY,0.1);//���Ũ��
	glHint(GL_FOG_HINT,GL_FASTEST);//�����Ⱦ��ʽ��GL_DONT_CARE�����Ľ���ֵ��GL_NICEST�����ģ�ÿһ������Ⱦ��GL_FASTEST��ÿһ������Ⱦ���ٶȿ�
	glFogf(GL_FOG_START, 1);//������Ļ�ľ���
	glFogf(GL_FOG_END, 5.0);


	//��ǽǵ�
	_mMarkerCorners.push_back(Point3f(-0.5f, -0.5f, 0));
	_mMarkerCorners.push_back(Point3f(-0.5f,  0.5f, 0));
	_mMarkerCorners.push_back(Point3f( 0.5f,  0.5f, 0));
	_mMarkerCorners.push_back(Point3f( 0.5f, -0.5f, 0));


	////ͨ������궨ȷ���Ĳ�����������������ƫ���ȷ���ǲ������ڱ궨��������
	//float mCameraMatrix[] = 
	//{
	//	590.7319		,0						,292.9710,
	//	0					, 619.0881		,202.5625,
	//	0					,0						,1
	//};
	//float dist_coeff[] = {0.1095483732100013, 0.005921985694402154, -0.02522667923131416, -0.0171742783898786, -0.1891767195416431};
	
	////20220626������
	//float mCameraMatrix[] = 
	//{
	//	621.6733		,0						,301.8697,
	//	0					, 596.7352		,223.5491,
	//	0					,0						,1
	//};
	//float dist_coeff[] = {0.2050844086865027, -1.253387945124429, -0.009926487596546369, -0.006799737561947785, 5.45488965637716};


	//�ⲿ����ͷ����
	float mCameraMatrix[] = 
	{
		508.3018		,0						,300.1497,
		0					, 504.5175		, 264.5351,
		0					,0						,1
	};
	float dist_coeff[] = {-0.4172170641396942, -0.1135454666162299, -0.0009781100036345459, -0.006095536879777572, 0.7763703887603729};


	//����3x3�е�mat
	m_camera_matrix = Mat(3, 3, CV_32FC1, mCameraMatrix).clone();	
	//����1x4�е�mat
	m_dist_coeff = Mat(1, 5, CV_32FC1, dist_coeff).clone();

	////������ͼ�������ڻ�䰵������
	//loadGLTextures();

}
void GlWndMain::resizeGL(int w, int h)
{
	cout<<__FUNCTION__<<endl;

	if (h==0)//��ֹ��Ϊ0
	{
		h=1;
	}
	glViewport(0,0,(GLint)w,(GLint)h);//���õ�ǰ���ӿ�
	glMatrixMode(GL_PROJECTION);//ѡ��ͶӰ����
	glLoadIdentity();//����ͶӰ����
	gluPerspective(45, (GLfloat)w/(GLfloat)h,0.1,300);//����͸��ͶӰ����
	//gluOrtho2D(0, w, 0, h);
	glMatrixMode(GL_MODELVIEW);//ѡ��ģ�͹۲����
	glLoadIdentity();//����ģ�͹۲����


}
void GlWndMain::paintGL()
{
	int iStartTime=GetSysTime_number();

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	bool bAppearNewMarker=false;
	if (1)
	{
		int width=this->width();
		int height=this->height();
		//_mMainCapture>>_mFrameImage ; 
		
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

		//cout<<"circle ��ʱms��"<<GetSysTime_number()-iStartTime<<endl;

		QImage buf((const unsigned char*)_mFrameImage.data, _mFrameImage.cols, _mFrameImage.rows, _mFrameImage.cols * _mFrameImage.channels(), QImage::Format_RGB888);
		QImage mTex = QGLWidget::convertToGLFormat(buf);
		mTex=mTex.rgbSwapped();
		glLoadIdentity();
		glPixelZoom((GLfloat)width/mTex.width(),(GLfloat)height/mTex.height());
		glDrawPixels(mTex.width(),mTex.height(),GL_RGBA,GL_UNSIGNED_BYTE, mTex.bits());
		//glTexImage2D(GL_TEXTURE_2D,0,3,mTex.width(),mTex.height(),0,GL_BGR_EXT,GL_UNSIGNED_BYTE,mTex.bits());
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
					w��������Ŀ��
					h��������ĸ߶�
					n�����������������ľ���
					fԶ���������������ľ���
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
				glLoadIdentity();//���õ�ǰָ���ľ���Ϊ��λ����
				//glMultMatrixf(m_projection_matrix);
				glLoadMatrixf(m_projection_matrix);

				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				glEnable(GL_DEPTH_TEST);
				glShadeModel(GL_SMOOTH); //some model / light stuff
				vector<Marker>& markers = m_recognizer.getMarkers();
				Mat rotation, translation;
				for (int i = 0; i < markers.size(); ++i)
				{
					//markers[i].estimateTransformToCamera(_mMarkerCorners, m_camera_matrix, m_dist_coeff, r, t);
					Mat rot_vec;
					bool res = solvePnP(_mMarkerCorners,		//i��������ϵ�µĿ��Ƶ������
						markers[i].m_corners,							//iͼ������ϵ�¶�Ӧ�Ŀ��Ƶ������
						m_camera_matrix,								//i����ڲ�
						m_dist_coeff,									//i�������
						rot_vec,										//o��ת����
						translation);											//oƽ������

					Rodrigues(rot_vec, rotation);				//��ת������Ϊ��ת����
					//cout<<"translation..."<<endl<<translation<<endl;
					//cout<<"rot_vec..."<<endl<<rot_vec<<endl;
					//cout<<"rotation..."<<endl<<rotation<<endl;

					//��X����ת180�ȣ���OpenCV����ϵ�任ΪOpenGL����ϵ
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

					
					glLoadMatrixf(m_model_view_matrix);////�ѵ�ǰ����GL_MODELVIEW��16��ֵ����Ϊָ����ֵ

					DrawARBox();
					
					bAppearNewMarker=true;

					cout<<"������ͼ��ʱms��"<<GetSysTime_number()-iStartTime<<endl;


					string strPicExportFolder=m_recognizer.GetExportPicFolder();
					if (strPicExportFolder!=""&&_access(strPicExportFolder.c_str() , 0)==0)
					{
						QPixmap::grabWindow(this->winId()).save((strPicExportFolder+"\\result.png").c_str(),"png");
					}
				}
			}
		}

	}


	//if (!_bRun){	return;}

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

	if (_bFog)
	{
		glEnable(GL_FOG);
	}
	else
	{
		glDisable(GL_FOG);
	}

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

	//DrawMainBox();

	//����Ѿ�������������&&��ʱû�б�ǣ�������ԭ����λ��
	if (_bCreatedArBox && !bAppearNewMarker)
	{
		if (_CurState==OpState::Initial)
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();//���õ�ǰָ���ľ���Ϊ��λ����
			glLoadMatrixf(m_projection_matrix);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glEnable(GL_DEPTH_TEST);
			glShadeModel(GL_SMOOTH); //some model / light stuff

			glLoadMatrixf(m_model_view_matrix);////�ѵ�ǰ����GL_MODELVIEW��16��ֵ����Ϊָ����ֵ
		}

		DrawARBox();
	}
	

	if (_bHyaline)
	{
		glDepthMask(TRUE);
		glDisable(GL_BLEND);
	}


}

//��ʱ�ص�
/*
void GlWndMain::timerEvent(QTimerEvent *)
{
	//��ȡ��ǰ��Ļ����
	int cxScreen = GetSystemMetrics (SM_CXSCREEN) ;  // wide
	int cyScreen = GetSystemMetrics (SM_CYSCREEN) ;  // high
	//����MediaPip��Python�汾����ֲ��ؼ���ʶ��
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
	//Sleep(100);
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
	std::string vecFingerState=_handPointsCls.GetFingerState(false);
	//cout<<"FingerState: "<<vecFingerState<<endl;
	Point3D mTipPos;
	{
		//ת������Ļ����
		//ͼ��[640,480]����Ч����[140,500][100,380]��ӳ�䵽��Ļ�ϵ�����[0,cxScreen][0,cyScreen]
		int iImgRange_Xmin=100;
		int iImgRange_Xmax=540;
		int iImgRange_Ymin=110;
		int iImgRange_Ymax=360;
		int iRangeWidth=iImgRange_Xmax-iImgRange_Xmin;
		int iRangeHight=iImgRange_Ymax-iImgRange_Ymin;
		mTipPos.X=(mTipPos_img.X-iImgRange_Xmin)*cxScreen/iRangeWidth;
		mTipPos.Y=(mTipPos_img.Y-iImgRange_Ymin)*cyScreen/iRangeHight;
		if(mTipPos.X <0)
			mTipPos.X=0;
		if (mTipPos.X>cxScreen)
			mTipPos.X=cxScreen;
		if(mTipPos.Y <0)
			mTipPos.Y=0;
		if (mTipPos.Y>cyScreen)
			mTipPos.Y=cyScreen;
	}
	Point3D mMousePos;
	mMousePos.X = _mLastPos.X + (mTipPos.X - _mLastPos.X) / 5;
	mMousePos.Y = _mLastPos.Y + (mTipPos.Y - _mLastPos.Y) / 5;
	//ɾ��΢С�ƶ����
	float fDiff=mMousePos.DistanceTo(_mLastPos);
	//cout<<"error Diff:"<<fDiff<<endl;
	if (fDiff<3)
	{
		return;
	}
	GLfloat dx = GLfloat(mMousePos.X-_mLastPos.X)/width();
	GLfloat dy = GLfloat(mMousePos.Y-_mLastPos.Y)/height();
	_mLastPos=mMousePos;
	cout<<"dx"<<dx<<endl;
	cout<<"dy"<<dy<<endl;
	int iSumZ=0;
	for(int a=0;a<_vecAllHandPoints.size();a++)
	{
		iSumZ+=_vecAllHandPoints[a].Z;
	}
	//if(_iLastSumZ!=0)
	//{
	//	double dZoomDiff=double(iSumZ-_iLastSumZ)/100;
	//	if (abs(dZoomDiff)>0.05)
	//	{
	//		_dZoom-=dZoomDiff;
	//		if(_dZoom>-2) _dZoom=-2;
	//		if(_dZoom<-12) _dZoom=-12;
	//	}
	//}
	//_iLastSumZ=iSumZ;
	//״̬��Ǩ
	if (vecFingerState=="00000")
	{
		if (_bPause==true)
		{
			return;
		}
		_bPause=true;
		if (_CurState==OpState::Initial)
		{
			_CurState=OpState::FrontView;
		}
		else if (_CurState==OpState::Adjustment)
		{
			_CurState=OpState::FrontView;
		}
		else if (_CurState==OpState::FrontView)
		{
			_CurState=OpState::Adjustment;
		}
		return;
	}
	else
	{
		_bPause=false;
		cout<<_CurState<<endl;
		//FrontView
		if (_CurState==OpState::Initial)
		{
			_CurState=OpState::FrontView;
		}
		if (_CurState==OpState::FrontView)
		{
			_dMoveX =0;
			_dMoveY =0;
			//thump, index finger
			if (vecFingerState=="01000")
			{
				_dRotX=0;
				_dRotY=0;
				_iFrontViewIndex=1;
			}
			else	if (vecFingerState=="01100")
			{
				_dRotX=90;
				_dRotY=0;
				_iFrontViewIndex=2;
			}
			else	if (vecFingerState=="01110")
			{
				_dRotX=180;
				_dRotY=0;
				_iFrontViewIndex=3;
			}
			else	if (vecFingerState=="01111")
			{
				_dRotX=270;
				_dRotY=0;
				_iFrontViewIndex=4;
			}
			else	if (vecFingerState=="11111")
			{
				_dRotX=-90;
				_dRotY=90;
				_iFrontViewIndex=5;
			}
			else	if (vecFingerState=="10001")
			{
				_dRotX=0;
				_dRotY=-90;
				_iFrontViewIndex=6;
			}
	
			//if (_iFrontViewIndex>=1&&_iFrontViewIndex<=6)
			//{
			//	_CurState=OpState::Adapting;
			//}
		}
		else	if (_CurState==OpState::Adjustment)
		{
			//��������
			_iFrontViewIndex=0;
			if (vecFingerState=="01100")
			{
				_dMoveX += dx*5;
				_dMoveY -= dy*5;
			}
			else	if (vecFingerState=="01110")
			{
				_dRotX -= dy*100;
				_dRotY += dx*100;
			}
			//else	if (vecFingerState=="01111")
			//{
			//	GLfloat zValue = dx+(-dy);
			//	_dZoom += zValue*2;
			//}
			//if (_dZoom<0.1)							_dZoom=0.1;
			//if (_dZoom>1.0)							_dZoom = 1.0;
			//if (_dMoveX<0)							_dMoveX=0;
			//if (_dMoveX>width())				_dMoveX=width();
			//if (_dMoveY<0)							_dMoveY=0;
			//if ((-_dMoveY)>height())		_dMoveY=- height();
			//if (_dRotX>360)							_dRotX=int(_dRotX)%360;
			//if (_dRotX<-360)						_dRotX=int(_dRotX)%360;
			//if (_dRotY>360)							_dRotY=int(_dRotY)%360;
			//if (_dRotY<-360)						_dRotY=int(_dRotY)%360;
		}
	}
	updateGL();
}
*/
//��ʱ�ص�
void GlWndMain::timerEvent(QTimerEvent *)
{
	//��ȡ��ǰ��Ļ����
	int cxScreen = GetSystemMetrics (SM_CXSCREEN) ;  // wide
	int cyScreen = GetSystemMetrics (SM_CYSCREEN) ;  // high

	//����MediaPip��Python�汾����ֲ��ؼ���ʶ��
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
	//Sleep(100);
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

	//״̬��Ǩ
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
			_dZoom=-5;
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

			if (vecFingerState[0]!=1
				&&vecFingerState[1]==1
				&&vecFingerState[2]!=1
				&&vecFingerState[3]!=1
				&&vecFingerState[4]!=1)
			{
				dDiff=dStep;
			}
			if (vecFingerState[0]==1
				&&vecFingerState[1]==1
				&&vecFingerState[2]!=1
				&&vecFingerState[3]!=1
				&&vecFingerState[4]!=1)
			{
				dDiff=-dStep;
			}

			_dZoom+=dDiff;
		}
	}

	updateGL();
}



//����¼�
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
	glGenTextures(6, texture);//����6������
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
			mBuf = dummy;//������벻�ɹ����Զ�������ɫͼƬ
		}
		mTex = QGLWidget::convertToGLFormat(mBuf);//QGLWidget�ṩ��ר��ת��ͼƬ�ľ�̬����
		mTex=mTex.rgbSwapped();
		glBindTexture(GL_TEXTURE_2D,texture[i]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D,0,3,mTex.width(),mTex.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,mTex.bits());
		//glTexSubImage2D(GL_TEXTURE_2D,0,3,mTex.width(),mTex.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,mTex.bits());
		cout<<"������ͼ��ʱms��"<<GetSysTime_number()-iStartTime<<endl;
		
	}
	glEnable(GL_TEXTURE_2D);
}
void GlWndMain::DrawARBox()
{
	//glTexImage2D�ڴ���ͼƬ����ʱ�������úܰ���������ȫ��ʹ����Ƶ��ͼ
	//cout<<__FUNCTION__<<endl;

	int iStartTime=GetSysTime_number();
	//���������Ƶ�ɹ����Ͱ���Ƶ��ʾ�ڷ����ϣ�������ʾͼƬ����
	if(bARVidioOK)
	{
		GLuint videoTextur;
		glGenTextures(1, &videoTextur);

		//���Խ�ͼ��Ҫ�Ļ���
		QImage mBuf, mTex;
		if (1)
		{
			if(videoOfAR.get(CV_CAP_PROP_POS_FRAMES) == _iFrameCount_Video1)
			{
				videoOfAR.set(CV_CAP_PROP_POS_FRAMES, 0);
			}
			int iTime1=GetSysTime_number();
			videoOfAR>>mFrameForBox ; 
			int iTime2=GetSysTime_number();
			cout<<"videoOfAR>>mFrameForBox ��ʱms:"<<iTime2-iTime1<<endl;

			//cvtColor(mFrameForBox, mFrameForBox, CV_BGR2RGB);
			//��Mat����ת����QImage
			mBuf = QImage((const unsigned char*)mFrameForBox.data, mFrameForBox.cols, mFrameForBox.rows, mFrameForBox.cols * mFrameForBox.channels(), QImage::Format_RGB888);
			int iTime3=GetSysTime_number();
			cout<<"��Mat����ת����QImage ��ʱms:"<<iTime3-iTime2<<endl;
			if (mBuf.isNull())
			{
				return;
			}
			mTex = QGLWidget::convertToGLFormat(mBuf);
			int iTime4=GetSysTime_number();
			cout<<"convertToGLFormat ��ʱms:"<<iTime4-iTime3<<endl;
		}
		else
		{
			string sTexPath="Data\\2.jpg";
			QString qsTexPath = QString::fromStdString(sTexPath);
			if (!mBuf.load(qsTexPath))
			{
				cout<<"cannot open image : "<<qsTexPath.toStdString()<<endl;
				QImage dummy(128,128,QImage::Format_ARGB32);
				dummy.fill(Qt::lightGray);
				mBuf = dummy;//������벻�ɹ����Զ�������ɫͼƬ
			}
			mTex = QGLWidget::convertToGLFormat(mBuf);//QGLWidget�ṩ��ר��ת��ͼƬ�ľ�̬����
			mTex=mTex.rgbSwapped();

		}
		glBindTexture(GL_TEXTURE_2D, videoTextur);//����һ���󶨵�Ŀ�����������
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexImage2D(GL_TEXTURE_2D, 0, 3, mTex.width(), mTex.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		int iTime5=GetSysTime_number();
		glTexImage2D(GL_TEXTURE_2D, 0, 3, mTex.width(), mTex.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, mTex.bits());
		int iTime6=GetSysTime_number();
		cout<<"convertToGLFormat ��ʱms:"<<iTime6-iTime5<<endl;

		glBindTexture(GL_TEXTURE_2D, videoTextur);
		glBegin(GL_QUADS);
		// ǰ��
		glNormal3f(0,0,1);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 1.0f); glVertex3f( dARBoxWidth, dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glEnd();
		//glDeleteTextures(1, &videoTextur);

		// ����
		glBindTexture(GL_TEXTURE_2D,videoTextur);
		glBegin(GL_QUADS);
		glNormal3f(0,0,-1);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 1.0f); glVertex3f( dARBoxWidth, dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glEnd();

		// ����
		glBindTexture(GL_TEXTURE_2D, videoTextur);
		glBegin(GL_QUADS);
		glNormal3f(0,1,0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 1.0f); glVertex3f( dARBoxWidth, dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 0.0f); glVertex3f( dARBoxWidth, dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glEnd();

		// ����
		glBindTexture(GL_TEXTURE_2D, videoTextur);
		glBegin(GL_QUADS);
		glNormal3f(0,-1,0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 1.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glEnd();

		// ����
		glBindTexture(GL_TEXTURE_2D, videoTextur);
		glBegin(GL_QUADS);
		glNormal3f(1,0,0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 0.0f); glVertex3f( dARBoxWidth, dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 1.0f); glVertex3f( dARBoxWidth, dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 1.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glEnd();

		// ����
		glBindTexture(GL_TEXTURE_2D, videoTextur);
		glBegin(GL_QUADS);
		glNormal3f(-1,0,0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glEnd();
		glDeleteTextures(1, &videoTextur);
	}
	else
	{
		// ǰ��
		glBindTexture(GL_TEXTURE_2D,texture[0]);
		glBegin(GL_QUADS);
		glNormal3f(0,0,1);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 1.0f); glVertex3f( dARBoxWidth, dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glEnd();

		// ����
		glBindTexture(GL_TEXTURE_2D,texture[1]);
		glBegin(GL_QUADS);
		glNormal3f(0,0,-1);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 1.0f); glVertex3f( dARBoxWidth, dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glEnd();

		// ����
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glBegin(GL_QUADS);
		glNormal3f(0,1,0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 1.0f); glVertex3f( dARBoxWidth, dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 0.0f); glVertex3f( dARBoxWidth, dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glEnd();

		// ����
		glBindTexture(GL_TEXTURE_2D, texture[3]);
		glBegin(GL_QUADS);
		glNormal3f(0,-1,0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 1.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glEnd();

		// ����
		glBindTexture(GL_TEXTURE_2D, texture[4]);
		glBegin(GL_QUADS);
		glNormal3f(1,0,0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 0.0f); glVertex3f( dARBoxWidth, dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 1.0f); glVertex3f( dARBoxWidth, dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 1.0f); glVertex3f( dARBoxWidth, -dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glEnd();

		// ����
		glBindTexture(GL_TEXTURE_2D, texture[5]);
		glBegin(GL_QUADS);
		glNormal3f(-1,0,0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-dARBoxWidth, -dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, dARBoxWidth); // ������ı��ε�����
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-dARBoxWidth, dARBoxWidth, -dARBoxWidth); // ������ı��ε�����
		glEnd();
	}

	_bCreatedArBox=true;
}
void GlWndMain::DrawMainBox()
{
	cout<<__FUNCTION__<<endl;
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glBegin(GL_QUADS);
	glNormal3f(0,0,1);
	glTexCoord2f(0,0); glVertex3f(-1,		-1,	1);
	glTexCoord2f(1,0); glVertex3f(1,		-1,	1);
	glTexCoord2f(1,1); glVertex3f(1,		1,		1);
	glTexCoord2f(0,1); glVertex3f(-1,		1,		1);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glBegin(GL_QUADS);
	glNormal3f(0,0,-1);
	glTexCoord2f(0,0); glVertex3f(1,		-1,	-1);
	glTexCoord2f(1,0); glVertex3f(-1,		-1,	-1);
	glTexCoord2f(1,1); glVertex3f(-1,		1,		-1);
	glTexCoord2f(0,1); glVertex3f(1,		1,		-1);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glBegin(GL_QUADS);
	glNormal3f(-1,0,0);
	glTexCoord2f(0,0); glVertex3f(-1,		-1,	-1);
	glTexCoord2f(1,0); glVertex3f(-1,		-1,	1);
	glTexCoord2f(1,1); glVertex3f(-1,		1,		1);
	glTexCoord2f(0,1); glVertex3f(-1,		1,		-1);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glBegin(GL_QUADS);
	glNormal3f(1,0,0);
	glTexCoord2f(0,0); glVertex3f(1,		-1,	1);
	glTexCoord2f(1,0); glVertex3f(1,		-1,	-1);
	glTexCoord2f(1,1); glVertex3f(1,		1,		-1);
	glTexCoord2f(0,1); glVertex3f(1,		1,		1);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[4]);
	glBegin(GL_QUADS);
	glNormal3f(0,1,0);
	glTexCoord2f(0,0); glVertex3f(-1,		1,		1);
	glTexCoord2f(1,0); glVertex3f(1,		1,		1);
	glTexCoord2f(1,1); glVertex3f(1,		1,		-1);
	glTexCoord2f(0,1); glVertex3f(-1,		1,		-1);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[5]);
	glBegin(GL_QUADS);
	glNormal3f(0,-1,0);
	glTexCoord2f(0,0); glVertex3f(-1,		-1,		-1);
	glTexCoord2f(1,0); glVertex3f(1,		-1,		-1);
	glTexCoord2f(1,1); glVertex3f(1,		-1,		1);
	glTexCoord2f(0,1); glVertex3f(-1,		-1,		1);
	glEnd();

}



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
