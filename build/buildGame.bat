@REM FOR /f %%i IN (../../games/currentGame.txt) DO set game=%%i
@REM to use: %game%
chdir %1 && mingw32-make.exe PROJECT=%2 && %2.exe 
 
 