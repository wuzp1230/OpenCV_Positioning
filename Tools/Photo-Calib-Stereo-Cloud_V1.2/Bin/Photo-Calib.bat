:: ���ڽ�ȡ��Ӹ�ʽ %date:~x,y%����ʾ�ӵ�xλ��ʼ����ȡy������(x,y����ʼֵΪ0)  
:: windows��DOS����date�Ľ�� 2016/09/03 ���� 
@echo off
set CURRENT_DATE="%date:~0,4%_%date:~5,2%_%date:~8,2%"
::echo %CURRENT_DATE% 
:: ʱ���ȡ��Ӹ�ʽ %time:~x,y%����ʾ�ӵ�xλ��ʼ����ȡy������(x,y����ʼֵΪ0)  
:: windows��DOS����time�Ľ�� 12:05:49.02   
set CURRENT_TIME="%time:~0,2%_%time:~3,2%"
::echo %CURRENT_TIME%


:CHO
CLS
ECHO. ======================================================================================
ECHO.       ��ѡ����Ĳ���
ECHO.                    1.�����ǰ������cameraXX�ļ���
ECHO.                    2.��ʼ����
ECHO.                    3.���ݱ궨ͼƬ
ECHO.                    4.�궨
ECHO.                    5.���ݱ궨����
ECHO.                    6.�����Ӳ�ͼ
ECHO.                    7.��Matlab���ɵ� stereoParams.mat �ļ�ת��OpenCV�궨����
ECHO.                    0.�˳�
ECHO.                                    wuzp1230 ����
ECHO. ======================================================================================
set choice=
set /p choice= �����Ӧ���֣�Ȼ�󰴻س���:
if /i "%choice%"=="1" goto CLEAR_CAM
if /i "%choice%"=="2" goto PHOTO
if /i "%choice%"=="3" goto BACKUP_CALIB
if /i "%choice%"=="4" goto CALIB
if /i "%choice%"=="5" goto BACKUP_CALIB_DATA
if /i "%choice%"=="6" goto BACKUP_DISP
if /i "%choice%"=="7" goto CONVERT_MATLAB
if /i "%choice%"=="0" goto QUIT
echo ѡ����Ч������������
echo.
GOTO CHO


::goto aaa

:CLEAR_CAM
::�����ǰ��camera
set /a rmdir_tmp = 1
:rmdir_loop_1
::echo %rmdir_tmp%
rd /s /q "%~dp0camera0%rmdir_tmp%"
set /a rmdir_tmp=%rmdir_tmp% + 1
if exist "%~dp0camera0%rmdir_tmp%\left" goto rmdir_loop_1
PAUSE
GOTO CHO

:PHOTO
::����1
.\TakePhotos.exe
GOTO CHO

:BACKUP_CALIB
::���camera���������ݱ궨ͼƬ
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
::��ʵ������Ϊloop_tmp - 1
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
::����2
::echo %loop_tmp_calib%
Stereo_camera_calib_new.exe %loop_tmp_calib%
PAUSE
GOTO CHO

::ɾ���궨��ͼƬ�����ݱ궨���ݣ�Ϊ��һ��takephotos����׼��
::�����ǰ��camera
set /a rmdir_tmp = 1
:rmdir_loop_2
::echo %rmdir_tmp%
rd /s /q "%~dp0camera0%rmdir_tmp%"
set /a rmdir_tmp=%rmdir_tmp% + 1
if exist "%~dp0camera0%rmdir_tmp%\left" goto rmdir_loop_2

:BACKUP_CALIB_DATA
::���ݱ궨����
@copy "%~dp0camera0?_results.xml" "%~dp0Backup\%CURRENT_DATE%\%CURRENT_TIME%\"
PAUSE
GOTO CHO

:CONVERT_MATLAB
::��Matlab���ɵ� .mat �ļ�ת��OpenCV�궨����
cd %~dp0
::����result.xml
matlab -nojvm -nodesktop  -r createXml('stereoParams');exit
::��result.xmlת��camera01_results.xml
readMatlabXml.exe
echo ת�����
PAUSE
GOTO CHO

:BACKUP_DISP
::���camera�����������Ӳ�ͼ
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
::��ʵ������Ϊloop_tmp - 1
:loop_2_over
echo ִ�����
PAUSE
GOTO CHO
PAUSE
:QUIT