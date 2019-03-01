运行环境需求：
1.	Visual Studio 2013
2.	OpenCV 2.4.13
3.	ZBar

运行所需的额外组件：
1.	OpenCV项目属性表OpenCV_Debug.props
2.	ZBar项目属性表ZBar_Debug.props

运行所需的器材
1.	720p摄像头两个

使用步骤：
1.	新建工程
2.	向工程/头文件分支下添加PNPSolver.h
3.	向工程/源文件分支下添加PNPSolver.cpp、ZB3.cpp
4.	向属性管理器中的Debug|Win32分支下添加OpenCV_Debug.prop、ZBar_Debug.props
5.	运行
