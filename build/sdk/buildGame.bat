chdir %1 && make -j%NUMBER_OF_PROCESSORS% PROJECT=%2 && del *.rc && del *.o && chdir build && %2.exe 

 