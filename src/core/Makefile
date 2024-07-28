OBJS = \
	$(wildcard src/*.cpp) \
	$(wildcard src/**/*.cpp) \
	$(wildcard src/**/**/*.cpp) \
	$(wildcard src/**/**/**/*.cpp) \
	$(wildcard src/**/**/**/**/*.cpp) \
	$(wildcard ../window/*.cpp) \
	$(wildcard ./vendors/box2d/src/**/*.cpp) \
	$(wildcard ./vendors/glad/src/*.c)



CC = g++ -g -O -std=c++17  

INCLUDE_PATHS = -IC:\libs\glad\include\glad 

LIBRARY_PATHS = -LC:\libs\glad  #-LC:\libs\openAL\Win32

#-w suppresses all warnings -Wl,-subsystem,windows gets rid of the console window

COMPILER_FLAGS = \
	-DSPAGHYETI_CORE \
	-DTHREAD_COUNT=$(THREAD_COUNT) \
	-DDEVELOPMENT=$(DEVELOPMENT) \
	-DUSE_JSON=$(USE_JSON) \
	-DSTANDALONE=$(STANDALONE) 

COMPILER_FLAGS_WEB = \
	-DSPAGHYETI_CORE \
	-DTHREAD_COUNT=5 \
	-DDEVELOPMENT=0 \
	-DUSE_JSON=0 \
	-DSTANDALONE=1

LINKER_FLAGS = -lmingw32 -lopengl32 -lglfw3 -lgdi32 -luser32 -lkernel32 -mwindows -lm  

#targets standard and web builds as static or dynamic libs

dynamic: compile

static: compile ar 

web: compile_web emar 

compile: $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) -w -Wl,-subsystem,windows $(LINKER_FLAGS) $(BUILDTYPE) $(BUILDFLAG) $(OUTFILE) 

compile_web: $(OBJS) 
	em++ -g -O3 -std=c++20 -w $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS_WEB) -pthread -c $(OUTFILE) 

ar: 
	ar rvs spaghyeti.a $(wildcard *.o)

emar:
	emar rvs spaghyeti-web.a $(wildcard *.o)



