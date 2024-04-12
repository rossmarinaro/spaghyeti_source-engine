cd ../src/core && echo "compiling core..." && mingw32-make.exe DEVELOPMENT="1" USE_JSON="1" STANDALONE="0" DLL="../../build/spaghyeti_source_runtime-editor.dll" && cd ../editor && ^ 
echo "compiling editor..." && mingw32-make.exe && echo "compile success! compressing..." && cd ../../build && ^
xcopy "%~dp0\spaghyeti_source_runtime-editor.dll" "%systemdrive%C:/software/upx-4.1.0-win64/upx-4.1.0-win64" && ^
xcopy "%~dp0\spaghyeti_source-engine-full.exe" "%systemdrive%C:/software/upx-4.1.0-win64/upx-4.1.0-win64" && ./compress.bat && ^
xcopy "%~dp0\spaghyeti_source_runtime-editor.dll" "%systemdrive%C:/project_data/projects/c++/spaghyeti_source_engine/build" && ^
xcopy "%~dp0\spaghyeti_source-engine-full.exe" "%systemdrive%C:/project_data/projects/c++/spaghyeti_source_engine/build" 
