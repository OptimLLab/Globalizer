@echo off
REM ============================================================================
REM  RunMPI.bat - запуск Globalizer на нескольких процессах MPI
REM
REM    gen\RunMPI.bat <процессов> <схема> [параметры Globalizer...]
REM
REM  схема:
REM    MPI_calc  - синхронная: процесс 0 ведёт метод и на каждой итерации раздаёт
REM                по MpiBlockSize (-mbs) точек каждому из (процессов - 1) рабочих
REM                процессов, NumPoints = (процессов - 1) * MpiBlockSize;
REM    AsyncMPI  - асинхронная: освободившийся рабочий процесс сразу получает
REM                новую точку, не дожидаясь остальных;
REM    OMP       - без распараллеливания по процессам (для сравнения).
REM  Нужно хотя бы 2 процесса (процесс 0 только ведёт метод); на одном процессе
REM  MPI-схемы переключаются на обычный вычислитель.
REM
REM  Программа - _bin_mpi\GlobalizerMain.exe (сборка gen\TestMPI-VS.bat);
REM  другую можно задать переменной GLOBALIZER_EXE (например GlobalizerSampleMain.exe).
REM  Дополнительные ключи mpiexec - переменная GLOBALIZER_MPIEXEC_ARGS.
REM
REM  Примеры:
REM    gen\RunMPI.bat 4 MPI_calc -lib rastrigin.dll -N 2 -r 3.5 -E 0.01
REM    gen\RunMPI.bat 4 MPI_calc -lib rastrigin.dll -N 2 -r 3.5 -E 0.01 -mbs 2
REM    gen\RunMPI.bat 8 AsyncMPI -lib stronginc3.dll -N 2 -r 4 -E 0.01
REM ============================================================================
setlocal EnableExtensions

if "%~2"=="" goto usage
set "NP=%~1"
set "MODE=%~2"
set "ARGS="
:collect
if "%~3"=="" goto args_done
set ARGS=%ARGS% %3
shift /3
goto collect
:args_done

for %%I in ("%~dp0..") do set "ROOT_DIR=%%~fI"
set "BIN_DIR=%ROOT_DIR%\_bin_mpi"
if not defined GLOBALIZER_EXE set "GLOBALIZER_EXE=GlobalizerMain.exe"
if not exist "%BIN_DIR%\%GLOBALIZER_EXE%" (
    echo %BIN_DIR%\%GLOBALIZER_EXE% not found - build the MPI version: gen\TestMPI-VS.bat
    exit /b 1
)

REM mpiexec: Intel MPI (I_MPI_ROOT или oneAPI по умолчанию), иначе MS-MPI, иначе PATH
set "MPIEXEC="
if not defined I_MPI_ROOT if exist "%ProgramFiles(x86)%\Intel\oneAPI\mpi\latest\bin\mpiexec.exe" set "I_MPI_ROOT=%ProgramFiles(x86)%\Intel\oneAPI\mpi\latest"
if defined I_MPI_ROOT if exist "%I_MPI_ROOT%\bin\mpiexec.exe" set "MPIEXEC=%I_MPI_ROOT%\bin\mpiexec.exe"
if defined MPIEXEC set "PATH=%I_MPI_ROOT%\bin;%PATH%"
if not defined MPIEXEC if defined MSMPI_BIN set "MPIEXEC=%MSMPI_BIN%mpiexec.exe"
if not defined MPIEXEC set "MPIEXEC=mpiexec"

cd /d "%BIN_DIR%"
echo "%MPIEXEC%" -n %NP% %GLOBALIZER_MPIEXEC_ARGS% %GLOBALIZER_EXE%%ARGS% -TypeCalculation %MODE%
"%MPIEXEC%" -n %NP% %GLOBALIZER_MPIEXEC_ARGS% "%BIN_DIR%\%GLOBALIZER_EXE%"%ARGS% -TypeCalculation %MODE%
set "RC=%errorlevel%"
endlocal & exit /b %RC%

:usage
echo Usage: gen\RunMPI.bat ^<processes^> ^<MPI_calc^|AsyncMPI^|OMP^> [Globalizer parameters...]
echo Example: gen\RunMPI.bat 4 MPI_calc -lib rastrigin.dll -N 2 -r 3.5 -E 0.01
exit /b 1
