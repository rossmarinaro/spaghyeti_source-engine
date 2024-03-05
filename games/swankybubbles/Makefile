

#OBJS specifies which files to compile as part of the project

OBJS = \
	$(wildcard *.cpp) \
	$(wildcard **/*.cpp) \
	$(wildcard ../../src/core/*.cpp) \
	$(wildcard ../../src/core/src/*.cpp) \
	$(wildcard ../../src/core/src/**/*.cpp) \
	$(wildcard ../../src/core/src/**/**/*.cpp) \
	$(wildcard ../../src/core/src/**/**/**/*.cpp) \
	$(wildcard ../../vendors/box2d/src/**/*.cpp) \
	$(wildcard ../../vendors/glad/src/*.c) \
	$(wildcard ../../vendors/glm/*.cpp) \
	$(wildcard ../../vendors/GLFW/*.cpp) 


#CC specifies which compiler we're using

CC = g++ -g -std=c++17 

#INCLUDE_PATHS specifies the additional include paths we'll need

INCLUDE_PATHS = -IC:\libs\glad\include\glad -IC:\libs\glfw-3.3.8\include\GLFW #-IC:\libs\openAL\include\AL
#LIBRARY_PATHS specifies the additional library paths we'll need

LIBRARY_PATHS = -LC:\libs\glad #-LC:\libs\openAL\Win32

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
# -Wl,-subsystem,windows gets rid of the console window

COMPILER_FLAGS = -DDEVELOPMENT=0 -DSTANDALONE=1 -D_JSON=1 -w #-Wl,-subsystem,windows

#LINKER_FLAGS specifies the libraries we're linking against

LINKER_FLAGS = -lmingw32 -lopengl32 -lglfw3 -lgdi32 -luser32 -lkernel32  #-static-libgcc -static-libstdc++ -mwindows -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -static-libgcc

#OBJ_NAME specifies the name of our exectuable

OBJ_NAME = -o swankybubbles.exe  # opengl-engine.dll -shared


#This is the target that compiles our executable

all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) ./resources/icon/icon.o $(OBJ_NAME) 


