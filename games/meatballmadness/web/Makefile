#EMSCRIPTEN BUILD

OBJS = \
	$(wildcard ../../../src/core/src/*.cpp) \
	$(wildcard ../../../src/core/src/**/*.cpp) \
	$(wildcard ../../../src/core/src/**/**/*.cpp) \
	$(wildcard ../../../src/core/src/**/**/**/*.cpp) \
	$(wildcard ../../../src/core/src/**/**/**/**/*.cpp) \
	$(wildcard ../../../vendors/*.cpp) \
	$(wildcard ../../../vendors/**/*.cpp) \
	$(wildcard ../../../vendors/**/***/*.cpp) \
	$(wildcard ../../../vendors/**/**/**/*.cpp) \
	$(wildcard ../../../src/window/*.cpp) \
	$(wildcard ../*.cpp) \
	$(wildcard ../**/*.cpp) \
	$(wildcard ../**/**/*.cpp) \
	$(wildcard ../**/**/**/*.cpp) \
	$(wildcard ../**/**/**/**/*.cpp)

CC = em++ -std=c++20 --pre-js pre-js.js --preload-file assets --use-preload-plugins 

COMPILER_FLAGS = \
	-DSPAGHYETI_CORE \
	-DDEVELOPMENT=0 \
	-DSTANDALONE=1 \
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

LIBRARY_FLAGS = -lGL -lglfw3 -openal 

OBJ_NAME = -O3 -o dist/index.html --shell-file template.html


all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) $(OBJ_NAME) 

#--js-flags=--experimental-wasm=threads --enable-features=WebAssembly,SharedArrayBuffer 

