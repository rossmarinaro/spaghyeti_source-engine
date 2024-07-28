// #pragma once

// #include <vector>

// #include "../entities/entity.h"


// class Particle : public Primitive {
    
//     public:

//         Particle(): Primitive()
//         {
//             Position = glm::vec2(50.0f); 
//             Velocity = glm::vec2(0.0f); 
//             Color = glm::vec4(1.0f);
//             Life = 0.0f;
//         };


//         glm::vec2 
//                 Position, 
//                 Velocity;

//         glm::vec4 Color;

//         float Life;
   
// };

// //-------------------------------


// class ParticleGenerator
// {

//     public:


//         ParticleGenerator(const char* key, unsigned int amount);

//         void Update(float dt, Sprite &object, unsigned int newParticles, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
//         void Draw();

//     private:


//         std::vector<Particle> particles;
//         unsigned int amount;

//         Shader& m_shader; 
//         Texture2D& m_texture;
//         unsigned int VAO;

//         // returns the first Particle index that's currently unused e.g. Life <= 0.0f or 0 if no particle is currently inactive

//         unsigned int firstUnusedParticle();

//         // respawns particle

//         void respawnParticle(Particle &particle, Sprite &object, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
// };

