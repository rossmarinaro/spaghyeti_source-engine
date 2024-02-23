#pragma once

#include "../../../../build/include/renderer.h"

class AssetManager {

    public:

    static inline std::string script_dir = "resources\\scripts";

    static inline std::vector<std::pair<std::string, GLuint>> images; 
    static inline std::map<std::string, std::string> loadedAssets;

    static std::string GetFolder(const std::string &asset);
    static std::string GetThumbnail(const std::string &asset);

};