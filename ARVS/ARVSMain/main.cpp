#include "GlWndMain.h"
#include <QtWidgets/QApplication>
#include <Windows.h>
#include <QtOpenGL/qtopengl>
#include <QtOpenGL/qgl.h>
#include <QApplication>
#include "MainWidget.h"
#include <time.h>
#include <iostream>
#include <io.h>
using std::cout;
using std::endl;



int main(int argc, char *argv[])
{

	QApplication a(argc, argv);
	MainWidget w;
	w.show();
	return a.exec();
}
