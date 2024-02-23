#pragma once

#include <string>
#include "../gui/gui.h" 

class Component {

    public:

        const char* m_type;

        std::string 
                   m_resourcePath, 
                   m_ID, 
                   m_name,
                   script_name;

        void Make();

        Component(const std::string &id, const char* type);
        ~Component();
    

    private:

        static inline int count = 0;
};