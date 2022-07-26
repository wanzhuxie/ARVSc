# ARVS
Video Display System Based on Augmented Reality

## 简介
基于增强现实及手势识别的交互式视频播放系统，系统实时识别摄像头捕获的图像，当有预定义的正方形标记时，在标记处创建立方体，在立方体的六个面上播放已提前配置好的视频文件，可以通过手势操控立方体。可用于校园宣传、商场指引、景区景点介绍等。

## 标记定义
 1. 5x5黑底白单元格，标记的四个角处有且只有一个角为孤立单元格，即其周围颜色都是黑色，即最后一行及最后一列的数值之和不可为0(黑0白1)
 2. 将左上角为孤立单元格的标记视为标准标记，当孤立单元格在其他的三个角处时，认为该标记是在标准标记的基础上平面旋转的结果。最后一行及最后一列均有白色单元格。
## 软件库
 - QT5
 - OpenCV2413
 - OpenGL
 - Python3.7
 - MeadiaPipe	
## 软件架构
#### 通用方法库 GeneralFunctions
一些多模块常用的方法
#### 数学库 MathLibrary
下载于https://www.songho.ca/index.html，版权归Song Ho Ahn (song.ahn@gmail.com)所有，在原基础上做了小修改
#### 相机标定 CameraCalibration
对所使用的摄像头进行标定，项目中包含标定用的棋盘图
#### 手关键点提取 HandPointsProvider
使用MeadiaPipe为手部关键点识别的框架，虽然是基于C++编写的，但其C++案例较少，反而是Python的接口众多。对于C++的编译，可以按照官方说明（https://google.github.io/mediapipe/getting_started/cpp.html）进行编译，但很繁琐，框架很庞大，需要深入学习。本系统暂使用Python提取手部21个关键点的2.5D坐标。
#### 主程序 ARVSMain
###### 标记的识别
OpenCV处理图像，识别预定义标记
###### 虚拟图形的创建
识别标记后，计算标记在空间的姿态，使用OpenGL在标记处创建立方体，并在立方体表面创建摄像头实时捕获的图像贴图
###### 手势分析
对通过HandPointsProvider得到的手关键点进行分析，计算当前手势，进而对虚拟物体进行相应的操控。

- 手势状态
 - 初始化状态: 程序刚刚初始化，尚未检测到标记	
 - 正视操作状态: 检测到了标记，此时伸出对应手指可对虚拟立方体进行正视操作
 - 移动、旋转、缩放操作状态
- 状态切换
 - 握拳手势
- 正视操作
 - 正视面1:食指
 - 正视面2:食指+中指
 - 正视面3:食指+中指+无名指
 - 正视面4:食指+中指+无名指+小拇指
 - 正视面5:食指+中指+无名指+小拇指+大拇指
 - 正视面6:小拇指+大拇指
- 移动、旋转、缩放操作
 - 移动:食指+中指
 - 旋转:食指+中指+无名指
 - 缩放:以手部21个关键点与摄像头距离之和为基础
	
###### 图形界面
主要为QT的OpenGL窗体，用于展示摄像头获取的真实世界场景及虚实结合后的场景。窗体中嵌入了一些可以用鼠标操作的控件，功能与手势操作基本一致。不过使用鼠标会更精确更顺畅地操控。

## 使用说明
启动程序，此时窗口中显示摄像头实时捕获的真实世界场景，在摄像头前展示标记，或将摄像头对准固定的标记，此时窗口中出现虚拟立方体，立方体的位置及姿态随标记相对于摄像头的位置而定。此时可用手势介入，介入后标记不再跟随标记而变化，而是受控于手势的操作。
