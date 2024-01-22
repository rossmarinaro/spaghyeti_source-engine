#pragma once

#include "../../../build/include/game.h"
#include "../../../build/include/inputs.h"


//game instance
class TestGame : public Game {

	public: 
        //static inline ParticleGenerator* playerParticles;
		TestGame() { name = "TESTGAME"; }
		
		void Preload() override;
		void Run(Camera* camera) override;
		void Update(Inputs* inputs, Camera* camera) override;


}; 
	 