#pragma once

#include "../../../../build/sdk/include/renderer.h"


namespace editor {

    class AssetManager {

        public:

            static inline const std::string script_dir = "resources\\scripts";
            static inline const std::string shader_dir = "resources\\shaders";

            static inline std::vector<std::pair<std::string, GLuint>> images; 
            static inline std::map<std::string, std::string> loadedAssets,
                                                             productionAssets;

            static void SetIcon(const std::string& key);
            static void LoadAsset(const std::string& asset, const std::string& path);
            static void SavePrefab(void* node);

            static std::string GetFolder(const std::string& asset);
            static std::string GetThumbnail(const std::string& asset);

    };

}
