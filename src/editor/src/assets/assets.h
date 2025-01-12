#pragma once

#include "../../../../build/sdk/include/renderer.h"
#include "../nodes/node.h"

namespace editor {

    class AssetManager {

        public:

            static inline const std::string script_dir = "resources\\scripts",
                                            shader_dir = "resources\\shaders";

            static inline std::string currentFolder, projectIcon, selectedAsset;
            static inline bool folderSelected = false;

            static inline std::vector<std::pair<std::string, GLuint>> images; 
            static inline std::vector<std::pair<std::string, GLuint>> audio; 
            static inline std::vector<std::pair<std::string, GLuint>> data; 

            static inline std::vector<std::string> assets; 

            static inline std::map<std::string, std::string> loadedAssets;

            static void Reset();
            static void SetIcon(const std::string& key);
            static void Register(const std::string& key);
            static void LoadAsset(const std::string& asset);
            static bool SavePrefab(const std::string& nodeId);
            static bool LoadPrefab(std::vector<std::shared_ptr<Node>>& nodes = Node::nodes);

            static std::string GetFolder(const std::string& asset);
            static std::string GetThumbnail(const std::string& asset);

    };

}
