#OLD IMPLEMENTATION

import sys
import os

#---------------- generate project title and directories


game_name = str(sys.argv[1].replace("-", "_")) if len(sys.argv) >= 2 else "NewGame"

if not os.path.exists(f'../games'):
	os.mkdir(f'../games')

os.mkdir(f'../games/{game_name}')
os.mkdir(f'../games/{game_name}/resources')
os.mkdir(f'../games/{game_name}/resources/assets')
os.mkdir(f'../games/{game_name}/resources/assets/images')
os.mkdir(f'../games/{game_name}/resources/assets/audio')
os.mkdir(f'../games/{game_name}/resources/icon')


#---------------- apply current game layer

os.mkdir(f'../games/{game_name}/web')
os.mkdir(f'../games/{game_name}/web/assets')
os.mkdir(f'../games/{game_name}/web/dist')

with open(f'../games/currentGame.txt', "w+") as file:
	file.write(f'''
{game_name}
	''')


with open(f'../games/currentGame.h', "w+") as file:
	file.write(f'''

#pragma once

//set correct paths to this file with respective symbol type to compile current game

#ifndef CURRENT_GAME

#include "./{game_name}/main.h"

typedef {game_name.capitalize()} Layer;

#define CURRENT_GAME

#endif

	''') 

#----------------- generate web boilerplate

with open(f'../games/{game_name}/web/Makefile', "w+") as file:
	file.write(f'''
#EMSCRIPTEN BUILD

OBJS = \n

	$(wildcard ../../../src/core/src/*.cpp) \\
	$(wildcard ../../../src/core/src/**/*.cpp) \\
	$(wildcard ../../../src/core/src/**/**/*.cpp) \\
	$(wildcard ../../../src/core/src/**/**/**/*.cpp) \\
	$(wildcard ../../../src/core/src/**/**/**/**/*.cpp) \\
	$(wildcard ../../../src/window/*.cpp) \\
	$(wildcard ../../../vendors/*.cpp) \\
	$(wildcard ../../../vendors/**/*.cpp) \\
	$(wildcard ../../../vendors/**/***/*.cpp) \\
	$(wildcard ../../../vendors/**/**/**/*.cpp) \\
	$(wildcard ../*.cpp) \\
	$(wildcard ../**/*.cpp) \\
	$(wildcard ../**/**/*.cpp) \\
	$(wildcard ../**/**/**/*.cpp) \\
	$(wildcard ../**/**/**/**/*.cpp)

CC = em++ -std=c++20 --pre-js pre-js.js --preload-file assets --use-preload-plugins 

COMPILER_FLAGS = \\
	-DDEVELOPMENT=0 \\
	-DPRINT_LOGS=0 \\
	-D_ISMOBILE=0 \\
	-D_JSON=0
 
LINKER_FLAGS = \\
	-sEXPORT_ALL=1 \\
	-sWASM=1 \\
	-sLEGACY_GL_EMULATION=0 \\
	-sASSERTIONS \\
	-sGL_ASSERTIONS=1 \\
	-sMAX_WEBGL_VERSION=3 \\
	-sMIN_WEBGL_VERSION=0 \\
	-sUSE_WEBGL2=1 \\
	-sFULL_ES3=1 \\
	-sUSE_GLFW=3 \\
	-sUSE_LIBPNG=1 \\
	-sUSE_ZLIB \\
	-sASYNCIFY \\
	-sUSE_PTHREADS=1 \\
	-sPTHREAD_POOL_SIZE_STRICT=28 \\
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
	''') 

with open(f'../games/{game_name}/web/pre-js.js', "w+") as file:
	file.write('''

Module['window'];
Module['document'];
Module['logReadFiles'] = function(files) { console.log('using files: ', files)} 
	''') 

with open(f'../games/{game_name}/web/template.html', "w+") as file:
	file.write(f'''

<!DOCTYPE html>
<html>

<head>
    <meta charset="utf-8">
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <link rel="stylesheet" type="text/css" href="./style.css" />
	<link rel="icon" type="image/png" sizes="192x192" href="./icon.png" />
    <title>{game_name}</title>
</head>

<body>

    <!-- Create the canvas that the C++ code will draw into -->
    <canvas id="canvas" oncontextmenu="event.preventDefault()"></canvas>


    <!-- MANUAL RESIZE OPTIONS -->

    <!-- <span id=controls>
        <span>
            <input 
                type=checkbox 
                id=resize>Resize canvas
        </span>
        <span>
            <input 
                type=checkbox 
                id=pointerLock 
                checked>Lock/hide mouse pointer    
        </span>
        <span>
            <input 
                type=button 
                onclick='Module.requestFullscreen(document.getElementById("pointerLock").checked,document.getElementById("resize").checked)' 
                value=Fullscreen>
        </span>
    </span> -->

    <!-- Allow the C++ to access the canvas element --> 
    <script type='text/javascript'>
        var Module = {{
            window: (function() {{ return window; }})(),
            document: (function() {{ return document; }})(),
            canvas: (function() {{ return document.getElementById('canvas'); }})()
        }};

        /* PREVENTS TOUCH EVENT FROM CLICKING ON INPUT AFTER 300MS */
 
        document.addEventListener('touchstart', () => setTimeout(() => document.querySelector('canvas').style.pointerEvents = 'none', 250));
        document.addEventListener('touchend', () => document.querySelector('canvas').style.pointerEvents = 'auto');
        document.addEventListener('touchmove', () => document.querySelector('canvas').style.pointerEvents = 'auto');
        document.addEventListener('touchcancel', () => document.querySelector('canvas').style.pointerEvents = 'auto');

        /* INIT FULLSCREEN RESIZE ON CLICK */
        
        //let isFullScreen = false;

        // document.addEventListener('click', () => {{
            
        //     if (isFullScreen)
        //         return;

        //     isFullScreen = true;

        //     Module.requestFullscreen(false, true);
        // }});

    </script>
    
    <!-- Add the javascript glue code (base.js) as generated by Emscripten -->
    <script src="base.js"></script>

    {{ SCRIPT }}

    
</body>

</html>
	''') 


#---------------- generate icon files

with open(f'../games/{game_name}/resources/icon/icon.rc', "w+") as file:
	file.write('1 ICON "./icon.ico"') 

with open(f'../games/{game_name}/resources/icon/Makefile', "w+") as file:
	file.write('''
icon.o: icon.rc	
	windres icon.rc icon.o
	''') 


#---------------- generate make file


with open(f'../games/{game_name}/Makefile', "w+") as file:
	file.write(f'''
	
OBJS = \\
	$(wildcard ./src/*.cpp) \\
	$(wildcard ./src/**/*.cpp) \\
	main.cpp \\
	spaghyeti_source_runtime-core.dll 

all : $(OBJS)
	g++ -g -std=c++17 $(OBJS) -w ./resources/icon/icon.o -o {game_name}.exe 

	''') 


#---------------- generate game header file


with open(f'../games/{game_name}/main.h', "w+") as file:
	file.write(f'''
	
#pragma once

#include "../../src/core/src/game/game.h"


//game instance
class {game_name.capitalize()} : public Game {{

	public: 

		{game_name.capitalize()}() {{ name = "{game_name.upper()}"; }}

		void Preload() override;
		void Run(Camera* camera) override;
		void Update(Inputs* inputs, Camera* camera) override;
		
}}; 
	''') 






