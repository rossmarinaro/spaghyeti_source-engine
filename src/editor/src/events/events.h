#pragma once

#include "../../../../build/sdk/include/entity.h"
#include "../scene.h"

namespace editor {

    class EventListener {

        public:

            static inline bool exitFlag = false,
                               saveFlag = false,
                               buildFlag = false,
                               canSave = true;
            
            static inline std::string s_currentProject = "",
                                      s_currentScene = "";

            static inline std::vector<std::pair<std::string, Scene*>> compileQueue;

            bool NewProject(const char* root_path = "C:\\");
            bool SaveScene(bool saveAs = false);
            bool OpenScene();
            
            void GenerateProject();
            void BuildAndRun();
            void OpenFile();
            void DecodeFile(const std::string& outPath, const std::filesystem::path& currentPath);
            
            std::string GetScriptName(const std::string& path);

        private:
            
            void Serialize(json& data); 
            void Deserialize(std::ifstream& JSON);
            void InsertTo(const std::string& code, const std::string& directory);
            void ParseScene(const std::string& sceneKey, std::ifstream& JSON);
    };
}