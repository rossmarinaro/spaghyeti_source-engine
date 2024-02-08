#pragma once

#include <mutex>
#include "../../src/core/game/game.h"

//game instance
class SwankyBubbles : public Game {

    public: 

        static inline int currentScore = 0;

        static inline int digit_1 = 0;
        static inline int digit_2 = 0;
        static inline int digit_3 = 0;

        static inline float dt = 0.0f;

        static inline bool game_over = false;
        static inline bool canRestart = false;
        static inline bool restart = false;
        static inline bool canFire = true;
        static inline bool started = false;
        static inline bool damageBuffer = true;
        static inline bool gruntBuffer = true;

        static inline int health = 3;

        static std::function<void(const char* key, bool loop)> destroyElfCallbackSound;

        static inline std::mutex bubble_lock;
        static inline std::mutex elf_lock;

        static inline Graphics::Quad* attackBox;
        static inline Graphics::Quad* hitBox;

        static inline std::vector<Sprite*> bubbles; 
        static inline std::vector<Sprite*> elves; 
        static inline std::vector<Sprite*> fireballs; 

        static inline std::vector<Sprite*> scoreText; 
        static inline std::vector<Sprite*> hearts;

        static inline Sprite* menu;
        static inline Sprite* background;
        static inline Sprite* gameOverText;

        SwankyBubbles() { name = "SWANKY BUBBLES!"; };

        static void ScorePoint(int amount);
        static void PopBubble(int type);

}; 
  