#pragma once

#include <string>
#include "../gui/gui.h" 


namespace editor {

    class Component {

        public:

            std::string 
                    m_ID, 
                    m_name,
                    filename,
                    m_type, 
                    m_nodeType;

            void Make();

            Component(const std::string &id, const std::string &type, const std::string &node_type);
            ~Component();
        

        private:

            static inline int count = 0;
    };
}