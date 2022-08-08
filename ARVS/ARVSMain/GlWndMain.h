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

	void initializeGL();
	void paintGL();
	void resizeGL(int w, int h);
	void timerEvent(QTimerEvent *);

	double ComputeThumbAngle( );


	////Mouse Keyboard
	//void keyPressEvent(QKeyEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent *e);

	bool fullScreen;
	GLfloat _dRotX,_dRotY,_dRotZ;
	GLfloat _dMoveX, _dMoveY;
	GLfloat _dZoom;

	GLfloat stepRotX,stepRotY,stepRotZ;//correction value

	//6 static texture
	GLuint texture[6];
	void loadGLTextures();

	QPoint lastPos;//mouse position

	bool _bKeepState;
	bool _bRun;//start end
	bool _bPause;//pause continue
	bool _bHyaline;//
	bool _bFog;//
	bool _bOpenAR;//ÊÇ·ñ¿ªÆôÉãÏñÍ·

	//WebCamera
	VideoCapture _mMainCapture;
	Mat _mFrameImage;

	//AR video
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

	//marker corner
	vector<cv::Point3f> _mMarkerCorners;

	//camera data
	cv::Mat m_camera_matrix;
	cv::Mat m_dist_coeff;

	//Main matrix
	float m_projection_matrix[16];
	float m_model_view_matrix[16];

	MarkerRecognizer m_recognizer;//marker Recognizer

	Point3D _mLastPos;//index finger tip position

	//hand points
	HandPointsProvider _handPointsCls;
	vector<Point3D> _vecAllHandPoints;
	
	enum OpState
	{
		Initial//
		,FrontView//
		,Adapting//not used
		,Adjustment//not used
		,Move//
		,Rotate//
		,Zoom//

	};
	OpState _CurState;
	std::string _strLastFingerState;
	bool _bCreatedArBox;


	int _iScreenWidth;
	int _iScreenHight;


	//for test
	int _iLastTime;
	BOOL _bShowHandPoints;

};

#endif // GlWndMain_H



