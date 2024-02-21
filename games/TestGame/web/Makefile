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