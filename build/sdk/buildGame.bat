chdir %1 && make PROJECT=%2 && del *.rc && del *.o && chdir build && %2.exe 
 
 