#EMSCRIPTEN BUILD

OBJS = \
	$(wildcard ../src/*.cpp) \
	$(wildcard ../src/**/*.cpp) \
	$(wildcard ../resources/scripts/*.cpp) 

CC = em++ -std=c++20 --pre-js pre-js.js --preload-file assets --use-preload-plugins 

COMPILER_FLAGS = \
	-DDEVELOPMENT=0 \
	-DPRINT_LOGS=0 \
	-D_ISMOBILE=0 \
	-D_JSON=0

LINKER_FLAGS = \
	-sEXPORT_ALL=1 \
	-sWASM=1 \
	-sLEGACY_GL_EMULATION=0 \
	-sASSERTIONS \
	-sGL_ASSERTIONS=1 \
	-sMAX_WEBGL_VERSION=3 \
	-sMIN_WEBGL_VERSION=0 \
	-sUSE_WEBGL2=1 \
	-sFULL_ES3=1 \
	-sUSE_GLFW=3 \
	-sUSE_LIBPNG=1 \
	-sUSE_ZLIB \
	-sASYNCIFY \
	-sUSE_PTHREADS=1 \
	-sPTHREAD_POOL_SIZE_STRICT=28 \
	-sSHARED_MEMORY=1 
# -sGLOBAL_BASE=8 
# -sPROXY_TO_PTHREAD=1
#-sEXPORTED_RUNTIME_METHODS=ccall,cwrap 
#-sEXPORT_NAME=testJS
# -sMODULARIZE 
# -sEXPORT_NAME=testFun
# -sWASM_WORKERS=1 
# -sAUDIO_WORKLET=1
# -sENVIRONMENT=web,worker 


all : $(OBJS)
	$(CC) $(OBJS) -L../spaghyeti_source_runtime-core.dll $(COMPILER_FLAGS) $(LINKER_FLAGS) -O3 -o dist/index.html --shell-file template.html


#--show-ports

#sample em
#touch test.cpp && em++ -std=c++20 test.cpp -sEXPORT_ALL=1 -sWASM=1 -sLEGACY_GL_EMULATION=0 -sASSERTIONS -sGL_ASSERTIONS=1 -sMAX_WEBGL_VERSION=3 -sMIN_WEBGL_VERSION=0 -sUSE_WEBGL2=1 -sFULL_ES2=1 -pthread -O3 -o opengl-engine-test-emscripten.html

#sample gcc
#g++ -g -std=c++20 ../vendors/glad/src/glad.c sample.cpp -IC:\libs\glad\include\glad -IC:\libs\glfw-3.3.8\include\GLFW -LC:\libs\glad -lmingw32 -lopengl32 -lglfw3 -lgdi32 -luser32 -lkernel32 -w -o test.exe && ./test.exe

#--js-flags=--experimental-wasm=threads --enable-features=WebAssembly,SharedArrayBuffer 

OBJS = \
   $(wildcard ../src/*.cpp) \
   $(wildcard ../src/**/*.cpp) \
   $(wildcard ../resources/scripts/*.cpp)
CC = em++ -std=c++20 --pre-js pre-js.js --preload-file assets --use-preload-plugins
COMPILER_FLAGS = \
   -DDEVELOPMENT=0 \
   -DPRINT_LOGS=0 \
   -D_ISMOBILE=0 \
   -D_JSON=0
LINKER_FLAGS = \
   -sEXPORT_ALL=1 \
   -sWASM=1 \
   -sLEGACY_GL_EMULATION=0 \
   -sASSERTIONS \
   -sGL_ASSERTIONS=1 \
   -sMAX_WEBGL_VERSION=3 \
   -sMIN_WEBGL_VERSION=0 \
   -sUSE_WEBGL2=1 \
   -sFULL_ES3=1 \
   -sUSE_GLFW=3 \
   -sUSE_LIBPNG=1 \
   -sUSE_ZLIB \
   -sASYNCIFY \
   -sUSE_PTHREADS=1 \
   -sPTHREAD_POOL_SIZE_STRICT=28 \
   -sSHARED_MEMORY=1
all : $(OBJS)
       $(CC) $(OBJS) -L../spaghyeti_source_runtime-core.dll $(COMPILER_FLAGS) $(LINKER_FLAGS) -O3 -o dist/index.html --shell-file template.html