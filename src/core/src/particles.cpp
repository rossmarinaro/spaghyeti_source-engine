
// #include "./particles.h"

// ParticleGenerator::ParticleGenerator(const char* key, unsigned int amount)
// : 
//   shader(System::Resources::Manager::shader->Get("sprite")), 
//   texture(System::Resources::Manager::texture2D->Get(key)), 
//   amount(amount)
// {
//     for (unsigned int i = 0; i < this->amount; ++i)
//         this->particles.push_back(Particle());
// }

// //--------------------------------------

// void ParticleGenerator::Update(float dt, Sprite &sprite, unsigned int newParticles, glm::vec2 offset)
// {

//     // add new particles 

//     for (unsigned int i = 0; i < newParticles; ++i)
//     {
//         int unusedParticle = this->firstUnusedParticle();
//         this->respawnParticle(this->particles[unusedParticle], sprite, offset);
//     }

//     // update all particles

//     for (unsigned int i = 0; i < this->amount; ++i)
//     {
//         Particle &p = this->particles[i];
//         p.Life -= dt; // reduce life
//         if (p.Life > 0.0f)
//         {	// particle is alive, thus update
//             p.Position -= p.Velocity * dt; 
//             p.Color.a -= dt * 2.5f;
//         }
//     }

//     this->Draw();
// }

// //-------------------------------------

// void ParticleGenerator::Draw()
// {

//     // use additive blending to give it a 'glow' effect

//     glBlendFunc(GL_SRC_ALPHA, GL_ONE);

//     this->shader.Use();

//     for (Particle particle : this->particles)
//     {
//         if (particle.Life > 0.0f)
//         { glm::mat4 model = glm::mat4(1.0f); 
//     this->shader.SetInt("image", 0, true);
//     this->shader.SetInt("repeat", this->texture.Repeat, true);
//     this->shader.SetFloat("alphaVal", 1.0f, true); 
//     this->shader.SetMat4("model", model, true);
//     this->shader.SetVec3f("spriteTint", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), true);
 
//     this->texture.Update(glm::vec2(50.0f), false, false, GL_FILL); 
//             // this->shader.SetVec2f("offset", particle.Position, true);
//             // this->shader.SetVec4f("color", particle.Color, true);
//             // this->texture.Bind();


//         }
//     }
//     // don't forget to reset to default blending mode
//     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
// }


// //stores the index of the last particle used (for quick access to next dead particle)

// unsigned int lastUsedParticle = 0;

// unsigned int ParticleGenerator::firstUnusedParticle()
// {

//     //first search from last used particle, this will usually return almost instantly

//     for (unsigned int i = lastUsedParticle; i < this->amount; ++i)
//     {
//         if (this->particles[i].Life <= 0.0f)
//         {
//             lastUsedParticle = i;
//             return i;
//         }
//     }

//     //otherwise, do a linear search

//     for (unsigned int i = 0; i < lastUsedParticle; ++i)
//     {
//         if (this->particles[i].Life <= 0.0f)
//         {
//             lastUsedParticle = i;
//             return i;
//         }
//     }

//     //all particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)

//     lastUsedParticle = 0;

//     return 0;
// }

// void ParticleGenerator::respawnParticle(Particle &particle, Sprite &sprite, glm::vec2 offset)
// {

//     float random = ((rand() % 100) - 50) / 10.0f;
//     float rColor = 0.5f + ((rand() % 100) / 100.0f);

//     particle.Position = glm::vec2(50.0f);//sprite.position + random + offset;
//     particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
//     particle.Life = 1.0f;
//     particle.Velocity = glm::vec2(0.0f);//sprite.m_velocity * 0.1f;
// }