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


	bool _bRun;//��ʼ����
	bool _bPause;//��ͣ����
	bool _bHyaline;//�Ƿ���͸��
	bool _bFog;//�Ƿ���͸��
	bool _bOpenAR;//�Ƿ�������ͷ

	VideoCapture m_videoFrame;
	Mat mFrame;
	int _iFrameCount_Video1;
	//AR������Ƶ
	VideoCapture videoOfAR;
	BOOL bARVidioOK;
	Mat mFrameForBox;

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

	int _iFrontViewIndex;//123456

	enum OpState
	{
		Initial,//��δͨ�����ƿ���
		FrontView,//����
		Adapting,//���ڵ�����������
		Adjustment//�ֵ�
	};
	OpState _CurState;

	bool _bCreatedArBox;
};

#endif // GlWndMain_H



