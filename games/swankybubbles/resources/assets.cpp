#include <vector>
#include <string>
#include <cstring>
#include <map>

#include "../../../src/core/resources/assets/assets.h"

//load raw char arrays if not webgl build

#ifndef __EMSCRIPTEN__
    #include "./assets/raw/audio/ring.h"
    #include "./assets/raw/audio/bubble_pop.h"
    #include "./assets/raw/audio/error.h"
    #include "./assets/raw/audio/fire_sound.h"
    #include "./assets/raw/audio/cave_music.h"
    #include "./assets/raw/audio/barf.h"

    #include "./assets/raw/image/cave.h"
    #include "./assets/raw/image/swanky_bubble.h"
    #include "./assets/raw/image/elf.h"
    #include "./assets/raw/image/fire.h"
    #include "./assets/raw/image/bubbles.h"
    #include "./assets/raw/image/heart.h"
    #include "./assets/raw/image/text.h"
    #include "./assets/raw/image/game_over.h"
    #include "./assets/raw/image/menu.h"
#endif

#include "./assets/raw/anims/elf.h"

#include "./assets/atlas/bubbles.h"
#include "./assets/atlas/elf.h"
#include "./assets/atlas/text.h"


using namespace Assets;

void Manager::Init()
{

    FLAC = {     

        #ifdef __EMSCRIPTEN__
            { "barf", "assets/barf.flac" },
            { "bubble_pop", "assets/bubble_pop.flac" },
            { "error", "assets/error.flac" },
            { "fire_sound", "assets/fire_sound.flac" },
            { "cave_music", "assets/cave_music.flac" },
            { "ring", "assets/ring.flac" } 
        #endif
    };

    RAW_AUDIO = {

        #ifndef __EMSCRIPTEN__
            { "ring", Audio::ring },
            { "bubble_pop", Audio::bubble_pop }, 
            { "error", Audio::error },
            { "fire_sound", Audio::fire_sound },
            { "cave_music", Audio::cave_music},
            { "barf", Audio::barf}
        #endif
    };

    PNG = {
    
        #ifdef __EMSCRIPTEN__
            { "cave", "assets/cave.png" },
            { "menu", "assets/menu.png" },
            { "swanky_bubble", "assets/swanky_bubble.png" },
            { "elf", "assets/elf.png" },
            { "fire", "assets/fire.png" },
            { "bubbles", "assets/bubbles.png" },
            { "heart", "assets/heart.png" },
            { "pixel_text", "assets/pixel_text.png" },
            { "game_over", "assets/game_over.png" },
            { "circle_button", "assets/action_button.png" },
            { "arrow_button", "assets/direction_button.png" }
        #endif
    }; 

    RAW_IMAGE = {

        #ifndef __EMSCRIPTEN__
            { "cave", Images::cave },
            { "menu", Images::menu },
            { "swanky_bubble", Images::swanky_bubble },
            { "elf", Images::elf }, 
            { "fire", Images::fire }, 
            { "bubbles", Images::bubbles },
            { "heart", Images::heart },
            { "pixel_text", Images::pixel_text },
            { "game_over", Images::game_over }
        #endif
    };

    ATLASES = {
        
        { "pixel_text", Spritesheets::pixel_text },
        { "elf", Spritesheets::elf },
        { "bubbles", Spritesheets::bubbles }
    };

    
    ANIMS = {

        { "elf", Anims::elf }
    };

    ATLAS_PATH = {
   
        { "pixel_text", "raw atlas" },
        { "elf", "raw atlas" },
        { "bubbles", "raw atlas" },

        { "some json atlas", "/path_to_spritesheet.json" },
        { "another json atlas", "/path_to_spritesheet.json" }
    };

    IMAGE_DIMENSIONS_AND_CHANNELS = {
        
        { "menu", { 500, 334, 3 } },
        { "cave", { 600, 400, 3 } },
        { "swanky_bubble", { 220, 151, 4 } },
        { "elf", { 220, 453, 4 } },
        { "fire", { 10, 9, 4 } },
        { "bubbles", { 480, 60, 4 } },
        { "heart", { 60, 60, 4 } },
        { "pixel_text", { 384, 144, 4 } },
        { "game_over", { 45, 25, 4 } },
        { "circle_button", { 25, 25, 4 } },
        { "arrow_button", { 25, 25, 4 } }

    };

    AUDIO_SIZE = {

        { "fire_sound", 13332 },
        { "bubble_pop", 17089 },
        { "error", 32533 },
        { "ring", 28443 },
        { "cave_music", 802481 },
        { "barf", 51329 }
    };
}





