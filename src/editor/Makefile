
OBJS = \
	main.cpp \
	$(wildcard src/*.cpp) \
	$(wildcard src/**/*.cpp) \
	$(wildcard src/**/**/*.cpp) \
	$(wildcard src/**/**/**/*.cpp) \
	$(wildcard ../shared/*.cpp) \
	$(wildcard ../vendors/**/*.cpp) \
	../../build/spaghyeditor.dll

CC = g++ -g -std=c++17 

LINKER_FLAGS = -lmingw32 -lopengl32 -lglfw3 -lgdi32 -luser32 -lkernel32 -mconsole -mwindows -lm

all : $(OBJS)
	$(CC) $(OBJS) -DUSE_JSON=1 -Wfatal-errors -w $(LINKER_FLAGS) ./res/res.o -o ../../build/spaghyeditor.exe


  