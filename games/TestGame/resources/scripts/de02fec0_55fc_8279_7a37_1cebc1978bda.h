#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/include/entity.h"

using app = System::Application;
class Sprite_de02fec0_55fc_8279_7a37_1cebc1978bda : public Sprite {

    public:

        //constructor, called on start

        Sprite_de02fec0_55fc_8279_7a37_1cebc1978bda (const std::string &key, float x, float y):
            Sprite(key, glm::vec2(x, y))
        {

        }

        //update every frame

        void Update(Inputs* inputs) override {
        if (inputs->m_down)
                //  {
                        if (inputs->m_left)
                            this->SetVelocityX(-20);

                    else if (inputs->m_right)
                            this->SetVelocityX(20);
                //  }
                // else 
                    //  this->SetVelocityX(0);//this->bodies[0].first->SetLinearVelocity(b2Vec2(0, 0));
        }

};