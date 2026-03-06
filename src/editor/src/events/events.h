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
            const bool OpenScene();
            
            void StartSession(std::ifstream& JSON);
            void ApplyState(bool increment);
            void BuildAndRun();
            void OpenFile();
            void DecodeFile(const std::string& outPath, const std::filesystem::path& currentPath);
            void EncodeFile(const std::string& path, bool newScene = false);
            
            std::string GetScriptName(const std::string& path);

            static inline std::string s_currentProject = "",
                                      s_currentScene = "";

            static inline const unsigned int ACTIONS_LIMIT = 5;
            static inline unsigned int actionsCount, eventCount;
            static inline bool actionsInit;

            static void UpdateSession();                      

        private:

            static void ParseScene(const std::string& sceneKey, std::ifstream& JSON);
            static void Serialize(json& data, bool newScene = false); 
            static void Deserialize(std::ifstream& JSON, int index = 0);
            static inline EventListener* s_self;
    };
}

