#pragma once

#include "../../../shared/renderer.h"
#include "../nodes/node.h"

namespace editor {

    class AssetManager {

        public:

            bool folderSelected;

            static inline const std::string script_dir = "resources/scripts", 
                                            shader_dir = "resources/shaders";

            std::string currentFolder, projectIcon, selectedAsset;

            std::vector<std::pair<std::string, GLuint>> images; 
            std::vector<std::pair<std::string, GLuint>> audio; 
            std::vector<std::pair<std::string, GLuint>> data; 
            std::vector<std::pair<std::string, GLuint>> text; 
            
            std::vector<std::string> assets; 

            std::map<std::string, std::string> loadedAssets;

            AssetManager();
            ~AssetManager() = default;

            static void Reset();
            static void Register(const std::string& key);
            static bool LoadAsset(const std::string& asset);
            
            static const bool SavePrefab(const std::string& nodeId, std::vector<std::shared_ptr<Node>>& arr);
            static const bool LoadPrefab(std::vector<std::shared_ptr<Node>>& nodes = Node::nodes);

            static const std::string GetThumbnail(const std::string& asset);
            static const std::string GetFolder(const std::string& asset);

            static inline AssetManager* Get() {
                return s_self;
            }

        private:

            static inline AssetManager* s_self;
    };

}
