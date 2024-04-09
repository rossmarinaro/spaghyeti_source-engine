
#OBJS specifies which files to compile as part of the project

OBJS = \
	$(wildcard src/*.cpp) \
	$(wildcard src/**/*.cpp) \
	$(wildcard src/**/**/*.cpp) \
	$(wildcard src/**/**/**/*.cpp) \
	$(wildcard src/**/**/**/**/*.cpp) \
	$(wildcard ../window/*.cpp) \
	$(wildcard ../../vendors/box2d/src/**/*.cpp) \
	$(wildcard ../../vendors/glad/src/*.c) \
	$(wildcard ../../vendors/glm/*.cpp) \
	$(wildcard ../../vendors/GLFW/*.cpp) 

#CC specifies which compiler we're using

CC = g++ -g -std=c++17 

#INCLUDE_PATHS specifies the additional include paths we'll need

INCLUDE_PATHS = -IC:\libs\glad\include\glad 

#LIBRARY_PATHS specifies the additional library paths we'll need

LIBRARY_PATHS = -LC:\libs\glad  #-LC:\libs\openAL\Win32

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
# -Wl,-subsystem,windows gets rid of the console window

COMPILER_FLAGS = \
	-DSPAGHYETI_CORE \
	-DDEVELOPMENT=$(DEVELOPMENT) \
	-DUSE_JSON=$(USE_JSON) \
	-DSTANDALONE=$(STANDALONE) \
	-w #-Wl,-subsystem,windows

#LINKER_FLAGS specifies the libraries we're linking against

LINKER_FLAGS = -lmingw32 -lopengl32 -lglfw3 -lfreetype -lgdi32 -luser32 -lkernel32 -mwindows -lm  

#OBJ_NAME specifies the name of our exectuable

OBJ_NAME = -o $(DLL) -shared  

   
#This is the target that compiles our executable

all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) ../../icon/icon.o $(OBJ_NAME) 


