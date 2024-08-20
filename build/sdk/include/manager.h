#pragma once

#include <map>
#include <vector>
#include <string>
#include <list>

#include "./shader.h"
#include "./texture.h"

namespace System {

    namespace Resources {


        // load Textures and Shaders. Each loaded texture
        // and/or shader is also stored for future reference by string
        // handles. Assets are defined in their respective containers.
        class Manager
        {

            public:

                //resource storage

                std::map<std::string, Shader> shaders;
                std::map<std::string, Graphics::Texture2D> textures;

                Manager() = default;
                ~Manager() = default;

                static const char* GetFilePath(const std::string& name);
                static const char* GetRawData(const std::string& key);  
                static const char* GetSpritesheetPath(const std::string& key);
                static const unsigned int GetSizeOfRawAudio(const std::string& key); 
                static const std::map<std::string, std::pair<int, int>> GetAnimations(const std::string& key);
                static const std::array<int, 3> GetRawDimensionsAndChannels(const std::string& key);
                static std::vector<std::array<int, 6>> GetRawSpritesheetData(const std::string& key);

                static void RegisterTextures();
                static void Clear(bool all = true);
                static void LoadFile(const char* key, const char* path);
                static void LoadRawImage(const char* key, const char* arr, int width, int height, int channel);
                static void LoadRawAudio(const char* key, const char* arr, unsigned int bytes);
                static void LoadAnims(const std::string& key, const std::map<std::string, std::pair<int, int>>& anims);
                static void LoadFrames(const std::string& key, const std::vector<std::array<int, 6>>& frames); 
                static void LoadAtlas(const std::string& key, const char* path); 

                static void UnLoadFile(const char* key);
                static void UnLoadRawImage(const char* key);
                static void UnLoadRawAudio(const char* key);
                static void UnLoadAnims(const std::string& key);
                static void UnLoadFrames(const std::string& key); 
                static void UnLoadAtlas(const std::string& key); 
                static std::vector<std::string> ParseCSV(const std::string& key, int index = 0);

            private:

                //asset storage

                std::map<std::string, unsigned int> audio_size;
                std::map<std::string, std::vector<std::array<int, 6>>> atlases;
                std::map<std::string, std::map<std::string, std::pair<int, int>>> anims;
                std::map<std::string, const char*> atlas_paths;
                std::map<std::string, std::array<int, 3>> image_dimensions_and_channels;

                //file assets
 
                std::map<std::string, std::string> m_file_image_assets;
                std::map<std::string, std::string> m_file_audio_assets;
                std::map<std::string, std::string> m_file_text_assets;
                std::map<std::string, std::string> m_file_assets;

                //raw assets

                std::map<std::string, const char*> m_raw_image_assets;
                std::map<std::string, const char*> m_raw_audio_assets;
                std::map<std::string, const char*> m_raw_assets;

        };
    }

}