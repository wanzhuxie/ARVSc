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

	void initializeGL();//初始化QpenGL窗口部件
	void paintGL();//绘制QPenGL窗口,有更新发生，函数就会被调用
	void resizeGL(int w, int h);//处理窗口大小变化，w和h是新状态下的宽和高，完成后自动刷新屏幕
	void timerEvent(QTimerEvent *);//实现窗口部件的定时操作


	void loadGLTextures();//载入纹理

	//void keyPressEvent(QKeyEvent *e);//鼠标按下事件处理函数
	void mousePressEvent(QMouseEvent *e);//鼠标单击事件
	void mouseMoveEvent(QMouseEvent *e);//鼠标移动事件
	void mouseReleaseEvent(QMouseEvent *e);//鼠标释放事件
	void wheelEvent(QWheelEvent *e);//鼠标滚轮事件
	void intrinsicMatrix2ProjectionMatrix(cv::Mat& camera_matrix, float width, float height, float near_plane, float far_plane, float* projection_matrix);
	void extrinsicMatrix2ModelViewMatrix(cv::Mat& rotation, cv::Mat& translation, float* model_view_matrix);

	bool fullScreen;
	GLfloat xrot,yrot,zrot;
	GLfloat stepRotX,stepRotY,stepRotZ;
	GLfloat moveX, moveY, zoom;
				 
	GLuint texture[6];//6个纹理
	GLuint texturFrame;

	GLfloat sphere_x,sphere_y;

	bool light;
	bool blend;


	GLuint fogFilter;//选择雾的类型
	GLuint filter;//储存纹理变量
	QPoint lastPos;


	bool sp;//空格键是否按下
	bool _bRun;//开始结束
	bool _bPause;//暂停继续
	bool _bHyaline;//是否开启透明
	bool _bFog;//是否开启透明
	bool _bOpenAR;//是否开启摄像头

	VideoCapture m_video;
	VideoCapture m_videoFrame;

	//AR方块视频
	VideoCapture videoOfAR;
	BOOL bARVidioOK;



	Mat mFrame, mFrameForBox;

	vector<cv::Point3f> m_corners_3d;

	cv::Mat m_camera_matrix;
	cv::Mat m_dist_coeff;
	float m_projection_matrix[16];
	float m_model_view_matrix[16];

	cv::Mat m_img_gray;
	cv::Mat m_img_color;
	cv::Mat mFrame_gray;
	MarkerRecognizer m_recognizer;

	vector<Point3f> g_bgVertices;

	vector<QPointF> m_vecHandPos;
	qreal m_HandDia;
};

#endif // GlWndMain_H



