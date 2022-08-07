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

	void initializeGL();//��ʼ��QpenGL���ڲ���
	void paintGL();//����QPenGL����,�и��·����������ͻᱻ����
	void resizeGL(int w, int h);//�����ڴ�С�仯��w��h����״̬�µĿ�͸ߣ���ɺ��Զ�ˢ����Ļ
	void timerEvent(QTimerEvent *);//ʵ�ִ��ڲ����Ķ�ʱ����

	double ComputeThumbAngle( );

	void loadGLTextures();//��������

	//void keyPressEvent(QKeyEvent *e);//���̰����¼�������
	void mousePressEvent(QMouseEvent *e);//��굥���¼�
	void mouseMoveEvent(QMouseEvent *e);//����ƶ��¼�
	void mouseReleaseEvent(QMouseEvent *e);//����ͷ��¼�
	void wheelEvent(QWheelEvent *e);//�������¼�

	bool fullScreen;
	GLfloat _dRotX,_dRotY,_dRotZ;
	GLfloat _dMoveX, _dMoveY;
	GLfloat _dZoom;

	GLfloat stepRotX,stepRotY,stepRotZ;//����ֵ

	GLuint texture[6];//6������

	QPoint lastPos;//���λ��

	bool _bKeepState;
	bool _bRun;//��ʼ����
	bool _bPause;//��ͣ����
	bool _bHyaline;//�Ƿ���͸��
	bool _bFog;//�Ƿ���͸��
	bool _bOpenAR;//�Ƿ�������ͷ

	VideoCapture _mMainCapture;
	Mat _mFrameImage;

	//AR������Ƶ
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

	//��ǽǵ�
	vector<cv::Point3f> _mMarkerCorners;

	cv::Mat m_camera_matrix;
	cv::Mat m_dist_coeff;
	float m_projection_matrix[16];
	float m_model_view_matrix[16];

	MarkerRecognizer m_recognizer;//���ʶ����

	Point3D _mLastPos;//����λ��
	int _iLastSumZ;
	HandPointsProvider _handPointsCls;
	vector<Point3D> _vecAllHandPoints;
	int _iFrontViewIndex;//123456
	BOOL _bShowHandPoints;
	enum OpState
	{
		Initial//��δͨ�����ƿ���
		//,Pause
		,FrontView//����
		,Adapting//���ڵ�����������NG
		,Adjustment//��������NG
		,Move//�ֵ�
		,Rotate//�ֵ�
		,Zoom//�ֵ�

	};
	OpState _CurState;
	std::string _strLastFingerState;
	bool _bCreatedArBox;

	int _iLastTime;

	int _iScreenWidth;
	int _iScreenHight;
};

#endif // GlWndMain_H



