@REM FOR /f %%i IN (../../games/currentGame.txt) DO set game=%%i
@REM to use: %game%
cd %1 && mingw32-make.exe && %2.exe 

