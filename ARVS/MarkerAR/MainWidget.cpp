#include "MainWidget.h"
#include <QPushButton>
#include <QPushButton>


MainWidget::MainWidget(QWidget *parent)
	: QWidget(parent)
	,_IsRunning(false)
	,pARTool(0)
	,pPushButton1(0)
	,pPushButton2(0)
	,pPushButton3(0)
	,pPushButton4(0)
	,pPushButton5(0)
	,pPushButton6(0)
{
	pARTool=new GlWndMain(this);
	pPushButton1=new QPushButton("Start", this);
	pPushButton2=new QPushButton("Fog:On", this);		//pPushButton2->setVisible(false);
	pPushButton3=new QPushButton("Pause", this);
	pPushButton4=new QPushButton("Hyaline:On", this);		pPushButton4->setFixedWidth(120);
	pPushButton5=new QPushButton("AR:Off", this);

	pSlider1=new QSlider(this);		pSlider1->setRange(0, 100);
	pSlider2=new QSlider(this);		pSlider2->setRange(0, 100);
	pSlider3=new QSlider(this);		pSlider3->setRange(0, 100);

	QHBoxLayout*layoutOnSlider=new QHBoxLayout;
	layoutOnSlider->addWidget(pSlider1);
	layoutOnSlider->addWidget(pSlider2);
	layoutOnSlider->addWidget(pSlider3);

	QVBoxLayout*layoutOnPushButton=new QVBoxLayout;
	layoutOnPushButton->addWidget(pPushButton1);
	layoutOnPushButton->addWidget(pPushButton3);
	layoutOnPushButton->addWidget(pPushButton4);
	layoutOnPushButton->addWidget(pPushButton2);
	layoutOnPushButton->addWidget(pPushButton5);
	layoutOnPushButton->addLayout(layoutOnSlider);

	QSpacerItem *spacerItem=new QSpacerItem(0,0,QSizePolicy::Fixed, QSizePolicy::Expanding);
	layoutOnPushButton->addSpacerItem(spacerItem);
	
	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(pARTool);
	layout->addLayout(layoutOnPushButton);
	setLayout(layout);

	ui.setupUi(this);

	connect(pPushButton1, &QPushButton::pressed, this, &MainWidget::ActionStart);
	connect(pPushButton2, &QPushButton::pressed, this, &MainWidget::ActionFog);
	connect(pPushButton3, &QPushButton::pressed, this, &MainWidget::ActionPause);
	connect(pPushButton4, &QPushButton::pressed, this, &MainWidget::ActionHyaline);
	connect(pPushButton5, &QPushButton::pressed, this, &MainWidget::ActionVideo);
	connect(pSlider1, &QSlider::valueChanged, this, &MainWidget::ActionSlider1);
	connect(pSlider2, &QSlider::valueChanged, this, &MainWidget::ActionSlider2);
	connect(pSlider3, &QSlider::valueChanged, this, &MainWidget::ActionSlider3);
}

MainWidget::~MainWidget()
{

}

void MainWidget::ActionStart()
{
	//if (!_IsRunning)
	{
		if (pPushButton1->text()=="Start")
		{
			pPushButton1->setText("End");
			pARTool->Run();
			_IsRunning=true;
		}
		else
		{
			pPushButton1->setText("Start");
			pARTool->Stop();
			_IsRunning=false;
		}
	}
}
void MainWidget::ActionPause()
{
	if (_IsRunning)
	{
		if (pPushButton3->text()=="Pause")
		{
			pPushButton3->setText("Continue");
			pARTool->Pause();
		}
		else
		{
			pPushButton3->setText("Pause");
			pARTool->Continue();
		}
	}
}
void MainWidget::ActionHyaline()
{
	if (_IsRunning)
	{
		if (pPushButton4->text()=="Hyaline:On")
		{
			pPushButton4->setText("Hyaline:Off");
			pARTool->OpenHyaline();
		}
		else
		{
			pPushButton4->setText("Hyaline:On");
			pARTool->CloseHyaline();
		}
	}
}
void MainWidget::ActionFog()
{
	if (_IsRunning)
	{
		if (pPushButton2->text()=="Fog:On")
		{
			pPushButton2->setText("Fog:Off");
			pARTool->OpenFog();
		}
		else
		{
			pPushButton2->setText("Fog:On");
			pARTool->CloseFog();
		}
	}
}
void MainWidget::ActionVideo()
{
	QString onOffValue=pPushButton5->text();
	if (onOffValue=="AR:Off")
	{
		pPushButton5->setText("AR:On");
		pARTool->CloseVideo();
	}
	else	if (onOffValue=="AR:On")
	{
		pPushButton5->setText("AR:Off");
		pARTool->OpenVideo();
	}
}

void MainWidget::ActionSlider1()
{
	int iSlider=pSlider1->value();	pARTool->SetRotateStepX((GLfloat)iSlider/50);

}
void MainWidget::ActionSlider2()
{
	int iSlider=pSlider2->value();	pARTool->SetRotateStepY((GLfloat)iSlider/50);

}
void MainWidget::ActionSlider3()
{
	int iSlider=pSlider3->value();	pARTool->SetRotateStepZ((GLfloat)iSlider/50);

}
