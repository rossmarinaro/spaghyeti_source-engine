#pragma once

#include <mutex>
#include <algorithm>

#include "../../src/core/src/game/game.h"


//game instance
class MeatballMadness : public Game {

    public: 

        int score, fails;

        static inline std::mutex chef_lock;

        static inline std::shared_ptr<Text> menuText;  
        static inline std::shared_ptr<Text> scoreText;
        static inline std::shared_ptr<Text> creditsText;
        static inline std::shared_ptr<Text> gameOverText;
        static inline std::shared_ptr<Sprite> chef; 
        static inline std::shared_ptr<Player> player;

        static inline b2Body* playerHitBox;
        static inline std::vector<std::shared_ptr<Sprite>> meatballs;
    
        MeatballMadness() { name = "MEATBALL MADNESS!"; }

        void GameOver();
        void Reset();

        void Preload() override;
		void Run(Camera* camera) override;
		void Update(Inputs* inputs, Camera* camera) override;

    private:

        
         
}; 

