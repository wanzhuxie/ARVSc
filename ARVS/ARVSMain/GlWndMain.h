#ifndef GlWndMain_H
#define GlWndMain_H

#include "MarkerRecognizer.h"
#include <QtWidgets/QWidget>
#include "ui_GlWndMain.h"
#include <QGLWidget>
#include <QTimer>
#include "gl/glu.h"
#include "gl/glut.h"
#include <QWidget>
#include <qgl.h>
#include <QKeyevent>
#include <QtOpenGL>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "HandPointsProvider.h"
using namespace cv;


class GlWndMain : public QGLWidget
{
	Q_OBJECT

public:
	GlWndMain(QWidget *parent = 0);
	~GlWndMain();
	void DrawARBox();
	void DrawARBox2();
	void DrawMainBox();
	void Run();
	void Stop();
	void Pause();
	void Continue();
	void OpenHyaline();
	void CloseHyaline();
	void OpenFog();
	void CloseFog();
	void OpenVideo();
	void CloseVideo();
	void SetRotateStepX(GLfloat fStep);
	void SetRotateStepY(GLfloat fStep);
	void SetRotateStepZ(GLfloat fStep);



private:
	Ui::GlWndMainClass ui;

	void initializeGL();//初始化QpenGL窗口部件
	void paintGL();//绘制QPenGL窗口,有更新发生，函数就会被调用
	void resizeGL(int w, int h);//处理窗口大小变化，w和h是新状态下的宽和高，完成后自动刷新屏幕
	void timerEvent(QTimerEvent *);//实现窗口部件的定时操作


	void loadGLTextures();//载入纹理

	//void keyPressEvent(QKeyEvent *e);//键盘按下事件处理函数
	void mousePressEvent(QMouseEvent *e);//鼠标单击事件
	void mouseMoveEvent(QMouseEvent *e);//鼠标移动事件
	void mouseReleaseEvent(QMouseEvent *e);//鼠标释放事件
	void wheelEvent(QWheelEvent *e);//鼠标滚轮事件

	bool fullScreen;
	GLfloat _dRotX,_dRotY,_dRotZ;
	GLfloat _dMoveX, _dMoveY;
	GLfloat _dZoom;

	GLfloat stepRotX,stepRotY,stepRotZ;//纠错值

	GLuint texture[6];//6个纹理

	QPoint lastPos;//鼠标位置

	bool _bKeepState;
	bool _bRun;//开始结束
	bool _bPause;//暂停继续
	bool _bHyaline;//是否开启透明
	bool _bFog;//是否开启透明
	bool _bOpenAR;//是否开启摄像头

	VideoCapture _mMainCapture;
	Mat _mFrameImage;

	//AR方块视频
	VideoCapture videoOfAR0;
	VideoCapture videoOfAR1;
	VideoCapture videoOfAR2;
	VideoCapture videoOfAR3;
	VideoCapture videoOfAR4;
	VideoCapture videoOfAR5;
	int _iFrameCount_Video0;
	int _iFrameCount_Video1;
	int _iFrameCount_Video2;
	int _iFrameCount_Video3;
	int _iFrameCount_Video4;
	int _iFrameCount_Video5;
	Mat _mFaceFrame0;
	Mat _mFaceFrame1;
	Mat _mFaceFrame2;
	Mat _mFaceFrame3;
	Mat _mFaceFrame4;
	Mat _mFaceFrame5;

	BOOL _bARVideoOK;

	//标记角点
	vector<cv::Point3f> _mMarkerCorners;

	cv::Mat m_camera_matrix;
	cv::Mat m_dist_coeff;
	float m_projection_matrix[16];
	float m_model_view_matrix[16];

	MarkerRecognizer m_recognizer;//标记识别器

	Point3D _mLastPos;//手势位置
	int _iLastSumZ;
	HandPointsProvider _handPointsCls;
	vector<Point3D> _vecAllHandPoints;
	int _iFrontViewIndex;//123456
	BOOL _bShowHandPoints;
	enum OpState
	{
		Initial//尚未通过手势控制
		//,Pause
		,FrontView//正视
		,Adapting//正在调整到正视中NG
		,Adjustment//度量调整NG
		,Move//手调
		,Rotate//手调
		,Zoom//手调

	};
	OpState _CurState;
	std::string _strLastFingerState;
	bool _bCreatedArBox;
};

#endif // GlWndMain_H



