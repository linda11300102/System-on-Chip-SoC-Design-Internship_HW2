@echo off
chcp 65001 >nul
set PYTHONUTF8=1
set VITIS=C:\AMDDesignTools\2026.1\Vitis\bin\vitis.bat
set ROOT=C:\vitis_irq
set SRC=%~dp0

rem ---- copy xsa + scripts into C:\vitis_irq ----
if not exist "%ROOT%" mkdir "%ROOT%"
if exist "%SRC%design_1_wrapper.xsa" copy /Y "%SRC%design_1_wrapper.xsa" "%ROOT%\" >nul
if exist "%SRC%make_irq.py" copy /Y "%SRC%make_irq.py" "%ROOT%\" >nul
if exist "%SRC%main_irq.c"  copy /Y "%SRC%main_irq.c"  "%ROOT%\" >nul
cd /d "%ROOT%"

for %%F in (design_1_wrapper.xsa make_irq.py main_irq.c) do (
  if not exist %%F (
    echo [ERROR] %ROOT%\%%F not found.
    echo         Put run_irq.bat, make_irq.py, main_irq.c in the SAME folder
    echo         as design_1_wrapper.xsa ^(Desktop\LED_Interruption^) and run again.
    pause
    exit /b
  )
)

echo ==================================================
echo  Building platform + app in %ROOT%\ws (3-10 min)
echo ==================================================
call "%VITIS%" -s make_irq.py
echo.
echo ==================================================
echo  Check above for ALL DONE. Press any key to open Vitis.
echo ==================================================
pause >nul
call "%VITIS%" -w %ROOT%\ws
