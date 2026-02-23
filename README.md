🍝 <::SPAGHYETI SOUЯCE ЭNGINE::> 🍝 

![image info](./images/spaghyeti-text.png)

----------------------------------------------------------

Features:

Triple buffer batch rendering (partial support) with additional framebuffer.

Rendering system / content creation tool with audio generation, texturing, and logging. 

Supports loading image (png), shaders (glsl text), json, csv, and raw byte encoded binary and compressed files.

Build assets can be either embedded or referenced.

Scene heirarchy supports sprites, CSV tilemaps (supports flipped tiles), audio, text, spawner, and empty nodes.

Supports native C++ scripting for nodes.

3 rendering layers.

Dynamic entity spawning to mitigate large memory usage.

Standalone runtime embedded into application builds, use as core only, or as DLL runtime with editor front end.

Project generation creates base boilerplate and proxy reference to current application, decoupled from core engine source.

Project configurations are saved as a custom ".SPAGHYETI" file which are decrypted into JSON prior to parsing.

Editor created in-scene game objects can be saved as prefabs for repeated use.

Manual per-Scene asset and shader loading to reduce unnecessary file loads.

Shader platform compatible versions are automatically updated per build type.

For Emscripten WebGL builds, download the latest SDK release, run emsdk install tot, navigate to emscripten install directory, run emcmdprompt.bat and cd to the "web" directory in this tree. compile with that directory's MakeFile.

Requires install of Python 3 to run build scripts.

Editor:
![image info](./images/screenshot.png)
![image info](./images/screenshot2.png)
![image info](./images/screenshot3.png)

Test Game:
![image info](./images/testgame.png)

---------------------------------------------------------- 

Written by Ross Marinaro - Pastaboss Enterprise 🍝👌

![image info](./src/editor/res/icon.ico)  

----------------------------------------------------------

Libraries used: GLFW / GLAD, GLM, GLText (embedded default text), FreeType (in-app fonts) STB_IMAGE, MiniAudio, DearIMGUI, Box2D, nlohmann JSON

GLText has some modifications to support OpenGLES as well as outline fragment shader color uniform support

Uses Unix tool xxd for byte encoding assets

Requires install of MingW (gcc) for Windows, although the goal is to be compiler agnostic

![image info](./images/stack.jpg)

----------------------------------------------------------

Notes:

When targeting specific entity from script use `System::Game::GetScene()->GetEntity()`, if targeting a method from another script use `System::Game::GetBehavior()`.

----------------------------------------------------------

To Do:

state management: undo/redo functionality (state snapshot memento pattern saving entire app state as json after each action)

improve editor entity selection/mouse picking/raycasting

tilemap editor

sprite texture packer

fix big .o files (audio.o manager.o game.o entity.o tilemap.o texture.o shader.o)

add CMakeLists.txt build option and build script with: `cmake -S . -B build -G "Your Generator" -DCMAKE_C_COMPILER=gcc -DCMAKE_POLICY_VERSION_MINIMUM=3.5` 
note: "Your Generator" ie: "MinGW Makefiles", "CodeBlocks", etc.

