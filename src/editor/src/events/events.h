#pragma once

#include "../../../../build/sdk/include/entity.h"
#include "../scene.h"

namespace editor {

    class EventListener {

        public:

            bool exitFlag,
                 saveFlag,
                 buildFlag,
                 canSave;

            std::vector<std::pair<std::string, Scene*>> compileQueue;

            EventListener();
            ~EventListener() = default;

            const bool NewScene(const char* root_path = "C:\\");
            const bool NewProject(const char* root_path = "C:\\");
            const bool SaveScene(bool saveAs = false);
            const bool OpenProject();
            
            void BuildAndRun();
            void OpenFile();
            void DecodeFile(const std::string& outPath, const std::filesystem::path& currentPath);
            void EncodeFile(const std::string& path, bool newScene = false);
            
            std::string GetScriptName(const std::string& path);

            static inline std::string s_currentProject = "",
                                      s_currentScene = "";

            /* 
            struct Event { int index; std::function<void()>&& fn; };

            std::vector<Event> events;
            
            void SetState(); */                      

        private:
            
            void Serialize(json& data, bool newScene = false); 
            void Deserialize(std::ifstream& JSON);
            void ParseScene(const std::string& sceneKey, std::ifstream& JSON);

            static inline EventListener* s_self;
    };
}

