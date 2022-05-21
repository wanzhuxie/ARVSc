# ARVS
Video Display System Based on Augmented Reality
基于增强现实的视频展示系统

##Method of use
Compile and run the program to display the predefined mark (5x5 black-and-white checkerboard) in front of the camera. After the program detects the mark, create a cube on the mark and display the preset pictures or videos in the data folder on the cube
编译运行程序，在摄像头前展示预定义标记(5x5黑白棋盘格)，程序检测到标记后在标记上创建立方体，在立方体上展示Data文件夹中预置的图片或视频
##Marker design
(1) The four corners of the tag have and only one corner is an isolated cell, that is, the surrounding color is black. The marker with the isolated cell in the upper left corner is regarded as the standard marker. When the isolated cell is at the other three corners, it is considered that the marker is the result of plane rotation on the basis of the standard marker;
(2) The last row and the last column have at least one white cell, that is, the sum of the values of the last row and the last column cannot be 0 (black 0 and white 1).
(1)标记的四个角处有且只有一个角为孤立单元格，即其周围颜色都是黑色。将左上角为孤立单元格的标记视为标准标记，当孤立单元格在其他的三个角处时，认为该标记是在标准标记的基础上平面旋转的结果；
(2)最后一行及最后一列均有白色单元格，即最后一行及最后一列的数值之和不可为0(黑0白1)。
##Examples
See the result folder
见result文件夹

