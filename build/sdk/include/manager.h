#pragma once

#include <map>
#include <vector>
#include <string>
#include <list>

#include "./shader.h"
#include "./texture.h"

namespace System {

    namespace Resources {

        struct BinaryResource { int type; unsigned char* array_buffer; unsigned int byte_length; };

        // load Textures and Shaders. Each loaded texture
        // and/or shader is also stored for future reference by string
        // handles. Assets are defined in their respective containers.
        class Manager
        {

            public:

                enum { IMAGE, AUDIO, TEXT, DATA, ICON, NOT_SUPPORTED };

                //resource storage

                std::map<std::string, Shader> shaders;
                std::map<std::string, Graphics::Texture2D> textures;

                Manager() = default;
                ~Manager() = default;

                static const std::string GetFilePath(const std::string& name);
                static const std::string GetSpritesheetPath(const std::string& key);
                static const BinaryResource GetResource(const std::string& key);   
                static const std::map<std::string, std::pair<int, int>> GetAnimations(const std::string& key);
                static const std::vector<std::array<int, 6>> GetRawSpritesheetData(const std::string& key);
                static const std::vector<std::string> ParseCSV(const std::string& key, int index = 0);

                static void RegisterTextures();
                static void Clear(bool all = true);
                static void LoadFile(const std::string& key, const std::string& path);
                static void LoadRaw(int type, const std::string& key, unsigned char* arr, unsigned int bytes);
                static void LoadAnims(const std::string& key, const std::map<std::string, std::pair<int, int>>& anims);
                static void LoadFrames(const std::string& key, const std::vector<std::array<int, 6>>& frames); 
                static void LoadAtlas(const std::string& key, const std::string& path); 

                static void UnLoadFile(const std::string& key);
                static void UnLoadRaw(int type, const std::string& key);
                static void UnLoadAnims(const std::string& key);
                static void UnLoadFrames(const std::string& key);  
                static void UnLoadAtlas(const std::string& key); 

            private:

                //asset storage

                std::map<std::string, std::vector<std::array<int, 6>>> m_atlases;
                std::map<std::string, std::map<std::string, std::pair<int, int>>> m_anims;
                std::map<std::string, std::string> m_atlas_paths;
                std::map<std::string, std::pair<int, std::string>> m_file_assets;
                std::map<std::string, BinaryResource> m_raw_assets;

        };
    }

}