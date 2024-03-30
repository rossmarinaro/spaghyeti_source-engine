#pragma once

#include "../../../../build/include/entity.h"
#include "../scene.h"

namespace editor {

    class EventListener {

        public:

            static inline bool exitFlag = false,
                               saveFlag = false,
                               shouldSave = false;
            
            static inline std::string currentProject = "",
                                      currentScene = "";

            static inline std::map<std::string, Scene> compileQueue;

            bool NewProject(const char* root_path = "C:\\");
            bool SaveProject(bool saveAs = false);
            bool OpenProject();
            
            void GenerateProject();
            void BuildAndRun();
            void OpenFile();

        private:
            
            void Serialize(json& data); 
            void Deserialize(std::ifstream& JSON);
            void DecodeFile(const std::string& outPath, const std::filesystem::path& currentPath);
            void InsertTo(const std::string& code, const std::string& directory);
            void ParseScene(const std::string& sceneKey, std::ifstream& JSON);
    };
}