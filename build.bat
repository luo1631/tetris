@echo off
REM Save with UTF-8 encoding
REM ======================================
REM Build script for Tetris game
REM Requires: GCC compiler (MinGW)
REM ======================================

echo Building Tetris game...
echo.

REM Check for GCC
echo Checking for GCC compiler...
where gcc >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Error: GCC compiler not found!
    echo Please install MinGW and add it to system PATH.
    echo Download from: https://www.mingw-w64.org/
    pause
    exit /b 1
)

REM Compile audio converter
echo Compiling audio converter...
gcc audio_converter.c -o audio_converter.exe
if %ERRORLEVEL% neq 0 (
    echo Error: Failed to compile audio converter!
    echo Please check if audio_converter.c exists and has no errors.
    pause
    exit /b 1
)

REM Create sounds directory if it doesn't exist
if not exist sounds mkdir sounds

REM Check for audio files
echo Checking audio files...
set AUDIO_FILES=rotate.wav move.wav drop.wav clear.wav gameover.wav theme1.mp3 theme2.mp3
set MISSING_FILES=0

for %%f in (%AUDIO_FILES%) do (
    if not exist sounds\%%f (
        echo Missing: %%f
        set /a MISSING_FILES+=1
    )
)

if %MISSING_FILES% neq 0 (
    echo.
    echo Error: Missing audio files!
    echo Required files in sounds directory:
    for %%f in (%AUDIO_FILES%) do echo - %%f
    echo Run create_sample_audio.bat to create sample files
    pause
    exit /b 1
)

REM Convert audio files
echo Converting audio files to C code...
for %%f in (%AUDIO_FILES%) do (
    echo Processing: %%f
    audio_converter.exe sounds/%%f audio_temp_%%~nf.c
    if errorlevel 1 (
        echo Error: Failed to convert %%f
        goto cleanup
    )
)

REM Compile main program
echo.
echo Compiling main program...
gcc -o eluos.exe eluos.c audio_temp_rotate.c audio_temp_move.c audio_temp_drop.c audio_temp_clear.c ^
    audio_temp_gameover.c audio_temp_theme1.c audio_temp_theme2.c -lwinmm
if errorlevel 1 (
    echo Error: Failed to compile main program!
    goto cleanup
)

:cleanup
REM Clean up temporary files
echo Cleaning up temporary files...
del audio_converter.exe 2>nul
del audio_temp_*.c 2>nul

echo.
echo Build completed successfully!
echo Run eluos.exe to start the game
echo.
echo Troubleshooting tips:
echo 1. Make sure all audio files exist
echo 2. Verify GCC compiler installation
echo 3. Check for sufficient disk space
echo.
pause 