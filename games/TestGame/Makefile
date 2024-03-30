
OBJS = \
	$(wildcard ./src/*.cpp) \
	$(wildcard ./src/**/*.cpp) \
	$(wildcard ./resources/scripts/*.cpp) \
	./build/spaghyeti_source_runtime-core.dll 

all : $(OBJS)/
	g++ -g -std=c++17 $(OBJS) -w -lmingw32 -lopengl32 -lglfw3 -lgdi32 -luser32 -lkernel32 ./resources/icon/icon.o -o ./build/$(PROJECT).exe 