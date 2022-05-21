#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include "ui_MainWidget.h"
#include "GlWndMain.h"
#include <QPushButton>
#include <QHBoxLayout>


class MainWidget : public QWidget
{
	Q_OBJECT

public:
	MainWidget(QWidget *parent = 0);
	~MainWidget();
	void ActionStart();
	void ActionEnd();
	void ActionPause();
	void ActionHyaline();
	void ActionFog();
	void ActionVideo();

	void ActionSlider1();
	void ActionSlider2();
	void ActionSlider3();

private:
	bool _IsRunning;
	Ui::MainWidget ui;
	GlWndMain*pARTool;
	QPushButton*pPushButton1;
	QPushButton*pPushButton2;
	QPushButton*pPushButton3;
	QPushButton*pPushButton4;
	QPushButton*pPushButton5;
	QPushButton*pPushButton6;
	QSlider * pSlider1;
	QSlider * pSlider2;
	QSlider * pSlider3;

};

#endif // MAINWIDGET_H
