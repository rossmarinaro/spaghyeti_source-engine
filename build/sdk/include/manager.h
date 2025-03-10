#pragma once

#include <map>
#include <vector>
#include <string>
#include <list>

#include "./shader.h"
#include "./texture.h"

#if USE_JSON == 1

	#include "./vendors/nlohmann/json.hpp"
	using json = nlohmann::json;

#endif

namespace System {

    namespace Resources {

        struct BinaryResource { std::string type; unsigned char* array_buffer; unsigned int byte_length; };

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
                static const char* GetSpritesheetPath(const std::string& key);
                static const BinaryResource GetResource(const std::string& key);   
                static const std::map<std::string, std::pair<int, int>> GetAnimations(const std::string& key);
                static const std::vector<std::array<int, 6>> GetRawSpritesheetData(const std::string& key);
                static const std::vector<std::string> ParseCSV(const std::string& key, int index = 0);

                static void RegisterTextures();
                static void Clear(bool all = true);
                static void LoadFile(const char* key, const char* path);
                static void LoadRaw(const std::string& type, const char* key, unsigned char* arr, unsigned int bytes);
                static void LoadAnims(const std::string& key, const std::map<std::string, std::pair<int, int>>& anims);
                static void LoadFrames(const std::string& key, const std::vector<std::array<int, 6>>& frames); 
                static void LoadAtlas(const std::string& key, const char* path); 

                static void UnLoadFile(const char* key);
                static void UnLoadRaw(const char* type, const char* key);
                static void UnLoadAnims(const std::string& key);
                static void UnLoadFrames(const std::string& key); 
                static void UnLoadAtlas(const std::string& key); 

            private:

                //asset storage

                std::map<std::string, std::vector<std::array<int, 6>>> m_atlases;
                std::map<std::string, std::map<std::string, std::pair<int, int>>> m_anims;
                std::map<std::string, const char*> m_atlas_paths;
                std::map<std::string, std::pair<std::string, std::string>> m_file_assets;
                std::map<std::string, BinaryResource> m_raw_assets;

        };
    }

}