#pragma once

#include <map>
#include <vector>

#include "./shader.h"
#include "./texture.h"

namespace System {

    namespace Resources {

        struct BinaryResource { 
            const int type; 
            const unsigned char* array_buffer; 
            const unsigned int byte_length; 
        };

        // load Textures and Shaders. Each loaded texture
        // and/or shader is also stored for future reference by string
        // handles. Assets are defined in their respective containers.
        class Manager
        {

            public:

                enum { IMAGE, AUDIO, TEXT, DATA, ICON, NOT_SUPPORTED };

                //resource storage

                std::map<const std::string, Graphics::Shader> shaders;
                std::map<const std::string, Graphics::Texture2D> textures;

                Manager() = default;
                ~Manager() = default; 

                static const std::string* GetFilePath(const std::string& name);
                static const std::string* GetSpritesheetPath(const std::string& key);
                static const BinaryResource* GetResource(const std::string& key);   
                static const std::map<const std::string, std::pair<int, int>>* GetAnimations(const std::string& key);
                static const std::vector<std::array<int, 6>>* GetRawSpritesheetData(const std::string& key);
                //parse CSV or JSON map data array
                static const std::vector<std::string> ParseMapData(const std::string& key, int index = 0);

                static void RegisterTextures();
                static void Clear(bool all = true);
                static void LoadFile(const std::string& key, const std::string& path);
                static void LoadRaw(const int type, const std::string& key, const unsigned char* arr, const unsigned int bytes);
                static void LoadAnims(const std::string& key, const std::map<const std::string, std::pair<int, int>>& anims);
                //can be used to load frames from a texture during preload, or at runtime to overwrite previous frames
                static void LoadFrames(const std::string& key, const std::vector<std::array<int, 6>>& frames); 
                static void LoadAtlas(const std::string& key, const std::string& path); 

                static void UnLoadFile(const std::string& key);
                static void UnLoadRaw(const int type, const std::string& key);
                static void UnLoadAnims(const std::string& key);
                static void UnLoadFrames(const std::string& key);  
                static void UnLoadAtlas(const std::string& key); 

            private: 

                //asset storage

                std::map<const std::string, std::vector<std::array<int, 6>>> m_atlases;
                std::map<const std::string, std::map<const std::string, std::pair<int, int>>> m_anims;
                std::map<const std::string, const std::string> m_atlas_paths;
                std::map<const std::string, std::pair<int, const std::string>> m_file_assets;
                std::map<const std::string, BinaryResource> m_raw_assets;

        };
    }

}