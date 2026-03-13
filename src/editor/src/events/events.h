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

            std::vector<std::shared_ptr<Node>>* selected_nodes;
            std::vector<std::pair<std::string, Scene*>> compileQueue;

            EventListener();
            ~EventListener() = default;

            const bool NewScene(const char* root_path = "C:\\");
            const bool NewProject(const char* root_path = "C:\\");
            const bool SaveScene(bool saveAs = false);
            const bool OpenScene();
            
            void StartSession(std::stringstream& stream);
            void ApplyState(bool increment);
            void BuildAndRun();
            void OpenFile();
            std::stringstream DecodeFile(const std::filesystem::path& path);
            void EncodeFile(const std::string& path, bool newScene = false);
            
            std::string GetScriptName(const std::string& path);

            static inline std::string s_currentProject = "",
                                      s_currentScene = "";
                                      
            static inline std::stringstream sessionData;

            static inline const unsigned int ACTIONS_LIMIT = 5;
            static inline unsigned int actionsCount, eventCount;
            static inline bool actionsInitUndo, actionsInitRedo;

            static void UpdateSession();                      

        private:

            static inline EventListener* s_self;

            static json ParseJSONStream(std::stringstream& stream, int index = 0); 
            static void ParseScene(const std::string& sceneKey, std::stringstream& stream);
            static void Serialize(json& data, bool newScene = false); 
            static void Deserialize(json& data, bool isSession = false);
    };
}

