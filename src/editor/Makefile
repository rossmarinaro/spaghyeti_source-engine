
OBJS = \
	main.cpp \
	$(wildcard src/*.cpp) \
	$(wildcard src/**/*.cpp) \
	$(wildcard src/**/**/*.cpp) \
	$(wildcard src/**/**/**/*.cpp) \
	$(wildcard ../window/*.cpp) \
	$(wildcard ../../vendors/imgui/*.cpp) \
	../../build/spaghyeti_source_runtime-editor.dll

CC = g++ -g -std=c++17 

LINKER_FLAGS = -lmingw32 -lopengl32 -lglfw3 -lgdi32 -luser32 -lkernel32 -mconsole -mwindows -lm

OBJ_NAME = -o ../../build/spaghyeti_source-engine-full.exe  


all : $(OBJS)
	$(CC) $(OBJS) -DUSE_JSON=1 -DDEVELOPMENT=0 -w $(LINKER_FLAGS) ../../icon/icon.o $(OBJ_NAME) 


  