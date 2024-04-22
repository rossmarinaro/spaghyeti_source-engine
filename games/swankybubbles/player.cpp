#include "../../src/core/app/app.h"
#include "./player.h"


void SwankyVelvet::Move(const char* direction)
{

    if (strcmp(direction, "right") == 0)
    { 
        this->SetVelocityX(1.5f * speed);
        this->SetFlipX(false);
    }

    if (strcmp(direction, "left") == 0)
    {
        this->SetVelocityX(-1.5f * speed);
        this->SetFlipX(true);
    }

    if (strcmp(direction, "up") == 0)
        this->SetVelocityY(1.5f * speed);

    if (strcmp(direction, "down") == 0)
        this->SetVelocityY(-1.5f * speed);
}


void SwankyVelvet::Update()
{

    if (System::Application::inputs.RIGHT && this->position.x < 650)   
        this->Move(System::Application::inputs.LEFT ? "left" : "right");
    
    if (System::Application::inputs.LEFT && this->position.x > 0)
        this->Move(System::Application::inputs.RIGHT ? "right" : "left");

    if (System::Application::inputs.DOWN && this->position.y > 0)
        this->Move(System::Application::inputs.UP ? "up" : "down");
    
    if (System::Application::inputs.UP && this->position.y < 380)
        this->Move(System::Application::inputs.DOWN ? "down" : "up");

}