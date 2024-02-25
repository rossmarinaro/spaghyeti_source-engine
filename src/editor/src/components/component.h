#pragma once

#include <string>
#include "../gui/gui.h" 

class Component {

    public:

        const char* m_type;

        std::string 
                   m_ID, 
                   m_name,
                   filename;

        void Make();

        Component(const std::string &id, const char* type);
        ~Component();
    

    private:

        static inline int count = 0;
};