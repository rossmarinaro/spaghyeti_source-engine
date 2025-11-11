#pragma once

#include <string>
#include "../gui/gui.h" 


namespace editor {

    class Component {

        public:

            enum { NONE, SHADER, SCRIPT, ANIMATOR, PHYSICS };

            int type;

            std::string ID, 
                        name,
                        filename;

            Component(const std::string& id, int type, int node_type, bool init = true);
            ~Component();

            void Make();

            static const std::string Get(int type);
            static void ApplyAnimations(bool init = false);
        

        private:

            int m_nodeType;
            bool m_init;
            static inline int s_count = 0;
    };
}