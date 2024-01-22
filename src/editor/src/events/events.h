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
        void Deserialize(std::ifstream &JSON, const std::string &directory, std::filesystem::path &result);

    private:
 
        static inline std::vector<std::string> contentCode;
        static inline std::vector<std::string> assetCode; 
        static inline std::vector<std::string> entityCode;

        static inline bool m_initialized = false;
        
        void InsertTo(const std::string &code, const std::string &directory);
};