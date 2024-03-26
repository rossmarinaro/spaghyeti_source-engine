#pragma once

#include <mutex>
#include <algorithm>

#include "../../build/include/scene.h"


//game instance
class MeatballMadness : public Scene {

    public: 

        static inline std::mutex chef_lock;

        MeatballMadness(const Process::Context& context):
		    Scene(context, "MEATBALL MADNESS!") { }

        ~MeatballMadness() = default;

        void Preload() override;
        void Run() override;
        void Update() override;

        void MoveChef();
        void GameOver();
        void Reset();

    private:

        int score, fails;

        b2Body* playerHitBox;
        std::vector<std::shared_ptr<Sprite>> meatballs;

        std::shared_ptr<Text> menuText;  
        std::shared_ptr<Text> scoreText;
        std::shared_ptr<Text> creditsText;
        std::shared_ptr<Text> gameOverText;
        std::shared_ptr<Sprite> chef; 
        std::shared_ptr<Sprite> player;
}; 

