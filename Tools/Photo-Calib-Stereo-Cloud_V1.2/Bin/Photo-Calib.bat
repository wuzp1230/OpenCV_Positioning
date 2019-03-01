:: 日期截取遵从格式 %date:~x,y%，表示从第x位开始，截取y个长度(x,y的起始值为0)  
:: windows下DOS窗口date的结果 2016/09/03 周六 
@echo off
set CURRENT_DATE="%date:~0,4%_%date:~5,2%_%date:~8,2%"
::echo %CURRENT_DATE% 
:: 时间截取遵从格式 %time:~x,y%，表示从第x位开始，截取y个长度(x,y的起始值为0)  
:: windows下DOS窗口time的结果 12:05:49.02   
set CURRENT_TIME="%time:~0,2%_%time:~3,2%"
::echo %CURRENT_TIME%


:CHO
CLS
ECHO. ======================================================================================
ECHO.       请选择你的操作
ECHO.                    1.清除先前的所有cameraXX文件夹
ECHO.                    2.开始拍照
ECHO.                    3.备份标定图片
ECHO.                    4.标定
ECHO.                    5.备份标定数据
ECHO.                    6.备份视差图
ECHO.                    7.将Matlab生成的 stereoParams.mat 文件转至OpenCV标定数据
ECHO.                    0.退出
ECHO.                                    wuzp1230 制作
ECHO. ======================================================================================
set choice=
set /p choice= 输入对应数字，然后按回车键:
if /i "%choice%"=="1" goto CLEAR_CAM
if /i "%choice%"=="2" goto PHOTO
if /i "%choice%"=="3" goto BACKUP_CALIB
if /i "%choice%"=="4" goto CALIB
if /i "%choice%"=="5" goto BACKUP_CALIB_DATA
if /i "%choice%"=="6" goto BACKUP_DISP
if /i "%choice%"=="7" goto CONVERT_MATLAB
if /i "%choice%"=="0" goto QUIT
echo 选择无效，请重新输入
echo.
GOTO CHO


::goto aaa

:CLEAR_CAM
::清除先前的camera
set /a rmdir_tmp = 1
:rmdir_loop_1
::echo %rmdir_tmp%
rd /s /q "%~dp0camera0%rmdir_tmp%"
set /a rmdir_tmp=%rmdir_tmp% + 1
if exist "%~dp0camera0%rmdir_tmp%\left" goto rmdir_loop_1
PAUSE
GOTO CHO

:PHOTO
::运行1
.\TakePhotos.exe
GOTO CHO

:BACKUP_CALIB
::检测camera数量，备份标定图片
set /a loop_tmp = 0
:loop_1
set /a loop_tmp=%loop_tmp% + 1
if not exist "%~dp0camera0%loop_tmp%\left" goto loop_1_over
::mk backup dir
@mkdir "%~dp0Backup\%CURRENT_DATE%\%CURRENT_TIME%\Calib\camera0%loop_tmp%\left"
@mkdir "%~dp0Backup\%CURRENT_DATE%\%CURRENT_TIME%\Calib\camera0%loop_tmp%\right"
::Backup
@copy "%~dp0camera0%loop_tmp%\left\*.png" "%~dp0Backup\%CURRENT_DATE%\%CURRENT_TIME%\Calib\camera0%loop_tmp%\left\"
::echo %loop_tmp%
if exist "%~dp0camera0%loop_tmp%\left" goto loop_1
::真实的数量为loop_tmp - 1
:loop_1_over
set /a loop_tmp=%loop_tmp% - 1
PAUSE
GOTO CHO

:CALIB
set /a loop_tmp_calib = 0
:loop_calib
set /a loop_tmp_calib=%loop_tmp_calib% + 1
if not exist "%~dp0camera0%loop_tmp_calib%\left" goto loop_calib_over
if exist "%~dp0camera0%loop_tmp_calib%\left" goto loop_calib
:loop_calib_over
set /a loop_tmp_calib=%loop_tmp_calib% - 1
::运行2
::echo %loop_tmp_calib%
Stereo_camera_calib_new.exe %loop_tmp_calib%
PAUSE
GOTO CHO

::删除标定板图片并备份标定数据，为下一步takephotos做好准备
::清除先前的camera
set /a rmdir_tmp = 1
:rmdir_loop_2
::echo %rmdir_tmp%
rd /s /q "%~dp0camera0%rmdir_tmp%"
set /a rmdir_tmp=%rmdir_tmp% + 1
if exist "%~dp0camera0%rmdir_tmp%\left" goto rmdir_loop_2

:BACKUP_CALIB_DATA
::备份标定数据
@copy "%~dp0camera0?_results.xml" "%~dp0Backup\%CURRENT_DATE%\%CURRENT_TIME%\"
PAUSE
GOTO CHO

:CONVERT_MATLAB
::将Matlab生成的 .mat 文件转至OpenCV标定数据
cd %~dp0
::生成result.xml
matlab -nojvm -nodesktop  -r createXml('stereoParams');exit
::将result.xml转至camera01_results.xml
readMatlabXml.exe
echo 转换完毕
PAUSE
GOTO CHO

:BACKUP_DISP
::检测camera数量，备份视差图
set /a loop2_tmp = 0
:loop_2
set /a loop2_tmp=%loop2_tmp% + 1
if not exist "%~dp0camera0%loop2_tmp%\left" goto loop_2_over
::mk backup dir
@mkdir "%~dp0Backup\%CURRENT_DATE%\%CURRENT_TIME%\Disps\camera0%loop2_tmp%\left"
@mkdir "%~dp0Backup\%CURRENT_DATE%\%CURRENT_TIME%\Disps\camera0%loop2_tmp%\right"
::Backup
@copy "%~dp0camera0%loop2_tmp%\left\*.png" "%~dp0Backup\%CURRENT_DATE%\%CURRENT_TIME%\Disps\camera0%loop2_tmp%\left\"
@copy "%~dp0camera0%loop2_tmp%\right\*.png" "%~dp0Backup\%CURRENT_DATE%\%CURRENT_TIME%\Disps\camera0%loop2_tmp%\right\"
::echo %loop2_tmp%
if exist "%~dp0camera0%loop2_tmp%\left" goto loop_2
::真实的数量为loop_tmp - 1
:loop_2_over
echo 执行完毕
PAUSE
GOTO CHO
PAUSE
:QUIT