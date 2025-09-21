#pragma once

#include "./actor.h"


namespace entity_behaviors {
    
    class Enemy : public Actor {

        public:

            std::shared_ptr<Physics::Body> hb;

            Enemy(std::shared_ptr<Entity> entity, const std::string& key, const std::string& name);
            Enemy(std::shared_ptr<Entity> entity, const std::string& key);
            Enemy(std::shared_ptr<Entity> entity);
            ~Enemy() = default;
            
            void Update() override;
 
        protected:
 
            int m_damage;

            bool m_reverse,
                 m_canMoveLeft,
                 m_canMoveRight;

            float m_startPos;

            bool ListenForDamage(const std::string& key);
            bool DoDamage(int amount, const std::string& key = "") override;

        private:

            std::string m_key;

            void Initialize(std::shared_ptr<Entity> entity);
            
    };
}     