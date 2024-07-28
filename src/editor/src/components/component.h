#pragma once

#include <string>
#include "../gui/gui.h" 


namespace editor {

    class Component {

        public:

            std::string ID, 
                        name,
                        filename,
                        type, 
                        nodeType;

            void Make();

            Component(const std::string& id, const std::string& type, const std::string& node_type, bool init = true);

            ~Component();
        

        private:

            bool m_init;
            static inline int s_count = 0;
    };
}