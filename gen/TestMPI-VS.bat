@echo off
REM ============================================================================
REM  TestMPI-VS.bat
REM
REM  Параллельная MPI-версия Globalizer: сборка с MPI (Intel MPI из oneAPI или
REM  MS-MPI) в build_64_mpi, программы - в _bin_mpi (обычная сборка в _bin не
REM  затрагивается), затем модульные тесты и интеграционные тесты MPI-версии
REM  (ctest -L mpi: integration_tests\run_mpi_tests.py, references\mpi_tests.json).
REM
REM  Шаги:
REM    [1/6] MPI: Intel MPI (I_MPI_ROOT или oneAPI по умолчанию) или MS-MPI (/ms)
REM    [2/6] Visual Studio (vswhere) и генератор CMake
REM    [3/6] Python для раннеров тестов (build_64\Globalizer_env или PATH)
REM    [4/6] CMake-конфигурация (GLOBALIZER_USE_MPI=ON, без MPFR и Python-модуля)
REM    [5/6] сборка
REM    [6/6] тесты: all_tests (модульные), затем ctest -L mpi
REM
REM  Ключи:
REM    /clean   - удалить кэш CMake build_64_mpi перед конфигурацией
REM    /notest  - только собрать
REM    /all     - запустить все тесты CTest (а не только модульные и MPI)
REM    /debug   - конфигурация Debug (по умолчанию Release)
REM    /ms      - MS-MPI вместо Intel MPI (нужны MSMPI_INC, MSMPI_LIB64, MSMPI_BIN)
REM
REM  Запуск решателя на нескольких процессах после сборки - gen\RunMPI.bat.
REM
REM  setvars.bat oneAPI не вызывается (он ломает парсер cmd, см. BuildTestAndOpenVS.bat):
REM  достаточно mpi\latest\env\vars.bat - только переменные Intel MPI.
REM ============================================================================
setlocal EnableExtensions

set "BUILD_CONFIG=Release"
set "DO_CLEAN=0"
set "DO_TEST=1"
set "TEST_ALL=0"
set "MPI_IMPL=intel"
for %%A in (%*) do (
    if /i "%%~A"=="/clean"  set "DO_CLEAN=1"
    if /i "%%~A"=="/notest" set "DO_TEST=0"
    if /i "%%~A"=="/all"    set "TEST_ALL=1"
    if /i "%%~A"=="/debug"  set "BUILD_CONFIG=Debug"
    if /i "%%~A"=="/ms"     set "MPI_IMPL=ms"
)

set "START_DIR=%cd%"
for %%I in ("%~dp0..") do set "ROOT_DIR=%%~fI"
set "BUILD_DIR=%ROOT_DIR%\build_64_mpi"
set "BIN_DIR=%ROOT_DIR%\_bin_mpi"
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "TMP_OUT=%TEMP%\globalizer_mpi_%RANDOM%.txt"

echo.
echo Root dir : %ROOT_DIR%
echo Build dir: %BUILD_DIR%
echo Bin dir  : %BIN_DIR%
echo Config   : %BUILD_CONFIG%
echo MPI      : %MPI_IMPL%

REM ---------------------------------------------------------------------------
echo.
echo ========================================
echo [1/6] MPI
echo ========================================
if /i "%MPI_IMPL%"=="ms" goto mpi_ms
if not defined I_MPI_ROOT if exist "%ProgramFiles(x86)%\Intel\oneAPI\mpi\latest\env\vars.bat" set "I_MPI_ROOT=%ProgramFiles(x86)%\Intel\oneAPI\mpi\latest"
if not defined I_MPI_ROOT ( set "FAIL_MSG=Intel MPI not found: install Intel oneAPI (MPI Library) or set I_MPI_ROOT; for MS-MPI use /ms." & goto error )
REM путь - до vars.bat: он переопределяет I_MPI_ROOT как ...\env\..
set "MPIEXEC=%I_MPI_ROOT%\bin\mpiexec.exe"
if exist "%I_MPI_ROOT%\env\vars.bat" call "%I_MPI_ROOT%\env\vars.bat" >nul
goto mpi_check
:mpi_ms
if not defined MSMPI_INC ( set "FAIL_MSG=MS-MPI SDK not found (MSMPI_INC is not set)." & goto error )
if not defined MSMPI_LIB64 ( set "FAIL_MSG=MS-MPI SDK not found (MSMPI_LIB64 is not set)." & goto error )
set "MPIEXEC=%MSMPI_BIN%mpiexec.exe"
if not exist "%MPIEXEC%" set "MPIEXEC=%MSMPI_BIN%\mpiexec.exe"
:mpi_check
if not exist "%MPIEXEC%" ( set "FAIL_MSG=mpiexec not found: %MPIEXEC%" & goto error )
echo [Found] %MPIEXEC%
"%MPIEXEC%" -n 2 hostname
if errorlevel 1 ( set "FAIL_MSG=mpiexec -n 2 hostname failed - check the MPI installation." & goto error )

REM ---------------------------------------------------------------------------
echo.
echo ========================================
echo [2/6] Visual Studio
echo ========================================
call :detect_vs
if errorlevel 1 ( set "FAIL_MSG=Visual Studio with C++ tools was not found." & goto error )
echo [Found] Visual Studio %VS_VER%
echo [Using] %VS_GENERATOR%

REM ---------------------------------------------------------------------------
echo.
echo ========================================
echo [3/6] Python for the test runners
echo ========================================
set "PY_EXE="
if exist "%ROOT_DIR%\build_64\Globalizer_env\python.exe" set "PY_EXE=%ROOT_DIR%\build_64\Globalizer_env\python.exe"
if not defined PY_EXE for /f "delims=" %%P in ('where python 2^>nul') do if not defined PY_EXE set "PY_EXE=%%P"
set PY_FLAGS=
if not defined PY_EXE goto no_python
echo [Using] %PY_EXE%
set PY_FLAGS="-DPython3_EXECUTABLE=%PY_EXE%"
goto python_done
:no_python
echo [WARNING] Python not found - CTest integration tests need python in PATH.
:python_done

REM CMake: из PATH, иначе - входящий в Visual Studio
set "CMAKE_EXE=cmake"
where cmake >nul 2>&1
if not errorlevel 1 goto cmake_done
set "CMAKE_EXE=%VS_PATH%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
set "CTEST_EXE=%VS_PATH%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe"
if not exist "%CMAKE_EXE%" ( set "FAIL_MSG=CMake not found (neither in PATH nor in Visual Studio)." & goto error )
:cmake_done
if not defined CTEST_EXE set "CTEST_EXE=ctest"
echo [Using] %CMAKE_EXE%

REM ---------------------------------------------------------------------------
echo.
echo ========================================
echo [4/6] CMake configuration
echo ========================================
if "%DO_CLEAN%"=="1" (
    if exist "%BUILD_DIR%\CMakeCache.txt" del /q "%BUILD_DIR%\CMakeCache.txt"
    if exist "%BUILD_DIR%\CMakeFiles" rmdir /s /q "%BUILD_DIR%\CMakeFiles"
)
"%CMAKE_EXE%" -S "%ROOT_DIR%" -B "%BUILD_DIR%" -G "%VS_GENERATOR%" -A x64 ^
  -DGLOBALIZER_OUTPUT_DIRECTORY="%BIN_DIR%" ^
  -DGLOBALIZER_USE_MPI=ON ^
  -DGLOBALIZER_MPI=%MPI_IMPL% ^
  -DGLOBALIZER_USE_MP=OFF ^
  -DGLOBALIZER_USE_CUDA=OFF ^
  -DGLOBALIZER_BUILD_PROBLEMS=ON ^
  -DGLOBALIZER_BUILD_GCGEN=ON ^
  -DGLOBALIZER_BUILD_TESTS=ON ^
  -DGLOBALIZER_BUILD_INTEGRATION_TESTS=ON ^
  -DGLOBALIZER_PYBIND_VERSION=OFF ^
  -DGLOBALIZER_PYTHON=OFF ^
  -DGLOBALIZER_MAX_DIMENSION=130 ^
  -DGLOBALIZER_MAX_Number_Of_Function=70 ^
  -Drastrigin_build=ON ^
  -DrastriginInt_build=ON ^
  -DX2_build=ON ^
  -Dstronginc3_build=ON ^
  -DrastriginC1_build=ON ^
  %PY_FLAGS% ^
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5
if errorlevel 1 ( set "FAIL_MSG=CMake configuration failed. Rerun with /clean if the generator or MPI changed." & goto error )

REM ---------------------------------------------------------------------------
echo.
echo ========================================
echo [5/6] Build (%BUILD_CONFIG%)
echo ========================================
REM /MP - параллельная компиляция файлов внутри проекта (MSBuild без него компилирует по одному)
set "CL=/MP %CL%"
"%CMAKE_EXE%" --build "%BUILD_DIR%" --config %BUILD_CONFIG% --parallel
if errorlevel 1 ( set "FAIL_MSG=Build failed." & goto error )

if "%DO_TEST%"=="0" goto done

REM ---------------------------------------------------------------------------
echo.
echo ========================================
echo [6/6] Tests
echo ========================================
set "MPLBACKEND=Agg"
set "TEST_RC=0"
if "%TEST_ALL%"=="1" (
    "%CTEST_EXE%" --test-dir "%BUILD_DIR%" -C %BUILD_CONFIG% --output-on-failure
    if errorlevel 1 set "TEST_RC=1"
    goto tests_done
)
echo --- unit tests (all_tests) ---
"%CTEST_EXE%" --test-dir "%BUILD_DIR%" -C %BUILD_CONFIG% -R "^all_tests$" --output-on-failure
if errorlevel 1 set "TEST_RC=1"
echo --- MPI integration tests (ctest -L mpi) ---
"%CTEST_EXE%" --test-dir "%BUILD_DIR%" -C %BUILD_CONFIG% -L mpi --output-on-failure
if errorlevel 1 set "TEST_RC=1"
:tests_done
if not "%TEST_RC%"=="0" ( set "FAIL_MSG=Tests FAILED (see the output above)." & goto error )
echo [OK] Tests passed.

:done
echo.
echo ========================================
echo DONE
echo ========================================
echo Binaries : %BIN_DIR%
echo Parallel run (4 processes, synchronous scheme):
echo   gen\RunMPI.bat 4 MPI_calc -lib rastrigin.dll -N 2 -r 3.5 -E 0.01
echo Asynchronous scheme:
echo   gen\RunMPI.bat 4 AsyncMPI -lib rastrigin.dll -N 2 -r 3.5 -E 0.01
cd /d "%START_DIR%"
endlocal
exit /b 0

REM ===========================================================================
:detect_vs
set "VS_GENERATOR="
set "VS_VER="
if not exist "%VSWHERE%" exit /b 1
"%VSWHERE%" -latest -prerelease -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion > "%TMP_OUT%" 2>nul
set /p VS_VER=<"%TMP_OUT%"
"%VSWHERE%" -latest -prerelease -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath > "%TMP_OUT%" 2>nul
set /p VS_PATH=<"%TMP_OUT%"
del "%TMP_OUT%" >nul 2>&1
if not defined VS_VER exit /b 1
for /f "tokens=1 delims=." %%V in ("%VS_VER%") do set "VS_MAJOR=%%V"
if "%VS_MAJOR%"=="18" set "VS_GENERATOR=Visual Studio 18 2026"
if "%VS_MAJOR%"=="17" set "VS_GENERATOR=Visual Studio 17 2022"
if "%VS_MAJOR%"=="16" set "VS_GENERATOR=Visual Studio 16 2019"
if defined VS_GENERATOR exit /b 0
exit /b 1

:error
echo.
echo ========================================
echo  BUILD / TEST FAILED
echo  %FAIL_MSG%
echo ========================================
cd /d "%START_DIR%"
endlocal
exit /b 1
