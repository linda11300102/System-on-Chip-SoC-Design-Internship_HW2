@echo off
chcp 65001 >nul
set PYTHONUTF8=1
set VITIS=C:\AMDDesignTools\2026.1\Vitis\bin\vitis.bat
set ROOT=C:\vitis_led
set SRC=%~dp0

rem ---- copy xsa + scripts into C:\vitis_led ----
if not exist "%ROOT%" mkdir "%ROOT%"
if exist "%SRC%design_1_wrapper.xsa" copy /Y "%SRC%design_1_wrapper.xsa" "%ROOT%\" >nul
if exist "%SRC%make_led.py" copy /Y "%SRC%make_led.py" "%ROOT%\" >nul
if exist "%SRC%main_led.c"  copy /Y "%SRC%main_led.c"  "%ROOT%\" >nul
cd /d "%ROOT%"

for %%F in (design_1_wrapper.xsa make_led.py main_led.c) do (
  if not exist %%F (
    echo [ERROR] %ROOT%\%%F not found.
    echo         Put run_led.bat, make_led.py, main_led.c in the SAME folder
    echo         as design_1_wrapper.xsa ^(Desktop\LED Blinking^) and run again.
    pause
    exit /b
  )
)

echo ==================================================
echo  Building platform + app in %ROOT%\ws (3-10 min)
echo ==================================================
call "%VITIS%" -s make_led.py
echo.
echo ==================================================
echo  Check above for ALL DONE. Press any key to open Vitis.
echo ==================================================
pause >nul
call "%VITIS%" -w %ROOT%\ws
