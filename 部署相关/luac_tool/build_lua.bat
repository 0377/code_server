@echo off
rem -----------------------------------------------
rem ����lua�ű�
rem ��Ҫ�� bat�ļ���luac.exe�ļ��ŵ�lua�ű���Ŀ��֮��
rem ִ��bat�ļ�����lua�ű���Ŀ��֮�´���һ�����ܺ���ļ���
rem ���ܺ���ļ���������bat���趨
rem -----------------------------------------------
rem lastedit 2017-8-19

rem ������Զ���Ŀ���ļ���
set OUTDIR=script_out


rem �ж�luac.exe�Ƿ����
if not exist luac.exe  (
echo luac.exe ���ڵ�ǰĿ¼�����ʵ
goto l_end 
)


rem �����ӳٱ���
setlocal enabledelayedexpansion 


rem ɾ��ԭ�е�Ŀ���ļ���
if exist %OUTDIR% (
rd /s /q %OUTDIR%
)

rem ����Ŀ���ļ��µ�lua�ļ�
set /a num=0
for /f %%x in ('dir /a-d /b /s  *.lua') do (
	echo ���ڼ����ļ�...!num!
    set /a num+=1
	rem ��ȡĿ���ļ�ȫ���ַ���
	set tempstr_full_out=%%x
	set "tempstr_full_out=!tempstr_full_out:%cd%=%OUTDIR%!"
	
	rem ��ȡԴ�ļ�ȫ���ַ���
	set tempstr_full_src=%%x
	
	rem ��ȡĿ���ļ��������ַ���
	set "tempstr_path=%%~dpx"
	set "tempstr_path=!tempstr_path:%cd%=%OUTDIR%!"
	rem ����Ŀ���ļ���
	if not exist !tempstr_path! (
	md !tempstr_path!
	)
	rem ����Ŀ��lua�ļ�
	cd.>!tempstr_full_out!

	rem ִ��luac����
	luac.exe -o !tempstr_full_out! !tempstr_full_src! 
	
)

echo һ�� %num% ���ļ�������

:l_end

