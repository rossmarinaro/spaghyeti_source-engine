#pragma once

#include <string>

class Component {

    public:

        const char* m_type;
        std::string m_resourcePath, m_ID, m_name;

        void Make(const std::string &name);

        Component(const std::string &id, const char* type, bool init = true);
        ~Component();
    

    private:

        static inline int count = 0;
        bool m_initialized;
};