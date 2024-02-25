#pragma once

#include "../../../../build/include/entity.h"

class EventListener {

    public:

        
        static inline std::string currentProject = "";

        bool NewProject(const char* root_path = "C:\\");
        bool SaveProject(bool saveAs = false);
        bool OpenProject();
        
        void GenerateProject();
        void BuildAndRun();
        void OpenFile();
        void Serialize(json &data);
        void Deserialize(std::ifstream &JSON, std::filesystem::path &result);

    protected:
        
        void InsertTo(const std::string &code, const std::string &directory);
};