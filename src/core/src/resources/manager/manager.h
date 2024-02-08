#pragma once

#include <map>
#include <vector>
#include <string>
#include <list>

#include "../shader/shader.h"
#include "../primitive.h"

#include "../../../../../vendors/box2d/include/box2d/box2d.h"

namespace System {

    namespace Resources {


        // A static singleton ResourceManager class that hosts several
        // functions to load Textures and Shaders. Each loaded texture
        // and/or shader is also stored for future reference by string
        // handles. Assets are defined in their respective containers.
        class Manager
        {

            public:

                std::map<std::string, std::vector<std::string>> TILEMAP;
                std::map<std::string, unsigned int> AUDIO_SIZE;
                std::map<std::string, std::vector<std::array<int, 4>>> ATLASES;
                std::map<std::string, std::map<std::string, std::pair<int, int>>> ANIMS;
                std::map<std::string, const char*> ATLAS_PATH;
                std::map<std::string, std::array<int, 3>> IMAGE_DIMENSIONS_AND_CHANNELS;
                                                        

                Manager() = default;
                ~Manager() = default;
                
                static inline Shader* shader;
                static inline Graphics::Texture2D* texture2D;
                static inline Graphics::Primitive* primitive;

                // resource storage

                static inline std::map<unsigned int, Graphics::Primitive> primitives;
                static inline std::map<std::string, Shader> shaders;
                static inline std::map<std::string, Graphics::Texture2D> textures;

                static const char* GetFilePath(const std::string &name);
 
                static const char* GetRawData(const std::string &key);  

                static const char* GetSpritesheetPath(const std::string &key);

                static const unsigned int GetSizeOfRawAudio(const std::string &key); 

                static const std::map<std::string, std::pair<int, int>> GetAnimations(const std::string &key);

                static const std::array<int, 3> GetRawDimensionsAndChannels(const std::string &key);

                static std::vector<std::array<int, 4>> GetRawSpritesheetData(const std::string &key);
                
                static const std::vector<std::string> GetRawTilemapData(const std::string &key);

                static void RegisterAssets(); 
                static void LoadFile(const char* key, const char* path);
                static void LoadRawImage(const char* key, const char* arr, int width, int height, int channel);
                static void LoadRawAudio(const char* key, const char* arr, unsigned int bytes);
                static void LoadAnims(const std::string &key, const std::map<std::string, std::pair<int, int>> &anims);
                static void LoadFrames(const std::string &key, const std::vector<std::array<int, 4>> &frames); 
                static void LoadAtlas(const std::string &key, const char* path); 
                static void LoadTilemap(const std::string &key, const std::vector<std::string> &data);

                static void UnLoadFile(const char* key);
                static void UnLoadRawImage(const char* key);
                static void UnLoadRawAudio(const char* key);
                static void UnLoadAnims(const std::string &key);
                static void UnLoadFrames(const std::string &key); 
                static void UnLoadAtlas(const std::string &key); 
                static void UnLoadTilemap(const std::string &key);

                static std::vector<std::string> ParseCSV(const std::string &key);

                // properly de-allocates all loaded resources

                static void Clear();

            private:

                //file assets

                std::map<std::string, std::string> file_image_assets;
                std::map<std::string, std::string> file_audio_assets;
                std::map<std::string, std::string> file_text_assets;

                //raw assets

                std::map<std::string, const char*> raw_image_assets;
                std::map<std::string, const char*> raw_audio_assets;

                //all assets

                std::map<std::string, std::string> file_assets;
                std::map<std::string, const char*> raw_assets;

        };
    }

}