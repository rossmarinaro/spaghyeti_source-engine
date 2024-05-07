
#include "./main.h"
#include "../../src/core/app/app.h"
#include "./player.h"



std::function<void(const char* key, bool loop)> SwankyBubbles::destroyElfCallbackSound = System::Application::audio.play;


//---------------------------- powerup



void SwankyBubbles::PopBubble(int type)
{

    System::Application::audio.play("bubble_pop"); 

    switch (type)
    {

        case 0: return;
        case 1: 
            static_cast<SwankyVelvet*>(player)->speed = 2;
            System::Application::time.delayedCall(10000, [&]() { static_cast<SwankyVelvet*>(player)->speed = 1; }); 
        break;
        case 2: SwankyBubbles::ScorePoint(1); break;
        case 3: 
            static_cast<SwankyVelvet*>(player)->canHit = false;
            System::Application::time.delayedCall(10000, [&]() { static_cast<SwankyVelvet*>(player)->canHit = true; }); 
        break;
        case 4: static_cast<SwankyVelvet*>(player)->canHit && health--; break;
        case 5: health++; break;
        case 6: 
            static_cast<SwankyVelvet*>(player)->canFire = true; 
            System::Application::time.delayedCall(10000, [&]() { static_cast<SwankyVelvet*>(player)->canFire = false; });
        break;
        case 7: SwankyBubbles::ScorePoint(3); break;
    }

}


//------------------------------ score pont


void SwankyBubbles::ScorePoint(int amount)
{

    if (currentScore > 999)
        return;

    currentScore += amount;

    System::Application::time.delayedCall(250, [&]() { System::Application::audio.play("ring"); });

    //update score digits

    if (currentScore >= 100)
    {
        if (currentScore == 100)
        {

            digit_1 = 1;
            digit_2 = 0;
            digit_3 = 0;
        }

        else
        {
            if (digit_1 < 9 && currentScore % 100 == 0)
                digit_1++;

            if (digit_2 < 9 && currentScore % 10 == 0) {
                digit_2++;
                digit_3 = 0;
            }
            else
            {
        
                if (currentScore % 10 == 0)
                {
                    if (digit_2 < 9) 
                        digit_2++;
                         
                   else if (currentScore % 100 == 0)
                       digit_2 = 0;

                    digit_3 = 0;
                }
                
                else
                    digit_3++; 

            }
        }
    }
    
    else 
    {

        if (currentScore % 10 == 0) {

            digit_1++;
            digit_2 = 0;
        }
        
        else
            digit_2++; 
    }
 

}


//---------------------------------- render and depth sorting 


void Game::Update()
{       

    //update score

    for (auto &text : SwankyBubbles::scoreText)
    {

        if (text->active)
        {

            if (text == SwankyBubbles::scoreText[6])
                text->SetFrame(SwankyBubbles::currentScore < 10 ? SwankyBubbles::currentScore : SwankyBubbles::digit_1);

            if (text == SwankyBubbles::scoreText[7])
            {

                if (SwankyBubbles::currentScore >= 10)
                {
                    text->SetFrame(SwankyBubbles::digit_2);
                    text->SetAlpha(1.0f);
                }

            else    
                text->SetAlpha(0.0f);
            }

            if (text == SwankyBubbles::scoreText[8])
            {

                if (SwankyBubbles::currentScore >= 100)
                {
                    text->SetFrame(SwankyBubbles::digit_3);
                    text->SetAlpha(1.0f);
                }
                else    
                    text->SetAlpha(0.0f);
            }
            
            text->SetFlipX(true);
        }
    }

//render menu

    if (!SwankyBubbles::started)
    {

        System::Game::GetScene()->ToggleVirtualButtonVisibility(false);

        for (auto &heart : SwankyBubbles::hearts)
            if (heart)
                heart->renderable = false;

        if (SwankyBubbles::menu->active)
            SwankyBubbles::menu->renderable = true;

        if (
            System::Game::GetScene()->GetContext().inputs.SHIFT || 
            System::Game::GetScene()->GetContext().inputs.SPACE ||
            System::Game::GetScene()->GetContext().inputs.LEFT_CLICK    
        )
        {

            SwankyBubbles::started = true;
            
            System::Game::GetScene()->GetContext().inputs.SHIFT = false;
            System::Game::GetScene()->GetContext().inputs.SPACE = false;
            System::Game::GetScene()->GetContext().inputs.LEFT_CLICK = false;
            player->renderable = true;

           //wasm build requires user guesture to play sound
           
            #ifdef __EMSCRIPTEN__
                if (!System::Application::audio.musicPlaying)
                    System::Application::audio.play("cave_music", true); 
            #endif
    
        }

        return;
    }

    else if (SwankyBubbles::restart)
    {
        Run();
        return;
    }

    System::Game::GetScene()->ToggleVirtualButtonVisibility(true);

//gameplay update

    if (!SwankyBubbles::game_over)
    {   
        

        if (SwankyBubbles::menu->active)
            SwankyBubbles::menu->renderable = false;

    //update background

        if (SwankyBubbles::background->active)
           SwankyBubbles:: background->renderable = true;

    //update health / hearts

        for (auto &heart : SwankyBubbles::hearts)
            if (heart)
                heart->renderable = true;

        switch (SwankyBubbles::health)
        {
            case 3:
                SwankyBubbles::hearts[0]->SetTint(glm::vec3(1.0f)); 
                SwankyBubbles::hearts[1]->SetTint(glm::vec3(1.0f));
                SwankyBubbles::hearts[2]->SetTint(glm::vec3(1.0f));
            break;
            case 2: 
                SwankyBubbles::hearts[0]->SetTint(glm::vec3(0.0f)); 
                SwankyBubbles::hearts[1]->SetTint(glm::vec3(1.0f));
                SwankyBubbles::hearts[2]->SetTint(glm::vec3(1.0f));
            break;
            case 1:
                SwankyBubbles::hearts[0]->SetTint(glm::vec3(0.0f));
                SwankyBubbles::hearts[1]->SetTint(glm::vec3(0.0f));
                SwankyBubbles::hearts[2]->SetTint(glm::vec3(1.0f));
            break;
            case 0:
                SwankyBubbles::hearts[0]->SetTint(glm::vec3(0.0f));
                SwankyBubbles::hearts[1]->SetTint(glm::vec3(0.0f));
                SwankyBubbles::hearts[2]->SetTint(glm::vec3(0.0f));

                SwankyBubbles::game_over = true; 
            break;
        }


    //set hit / attack boxes position to player

        if (SwankyBubbles::hitBox != nullptr && SwankyBubbles::attackBox != nullptr)
        {

            SwankyBubbles::hitBox->SetPosition(
                player->flipX ? player->position.x + 50 : player->position.x + 10,
                player->position.y + 10
            );

            SwankyBubbles::attackBox->SetPosition(
                player->flipX ? player->position.x : player->position.x + 90,
                player->position.y + 40 
            );

            #ifndef __EMSCRIPTEN__
                if (System::Resources::Manager::enableDebug)
                {
                    SwankyBubbles::hitBox->m_debug = true;
                    SwankyBubbles::attackBox->m_debug = true; 
                }
            #endif
        }

        //player tint

        if (player && !static_cast<SwankyVelvet*>(player)->canHit)
            player->SetTint(glm::vec3(1.75f, 1.75f, 1.75f));
        else
            player->SetTint(glm::vec3(1.0f, 1.0f, 1.0f));


    //update stack allocated entities

    //bubbles

        for (auto &bubble : SwankyBubbles::bubbles)
        {

            if (!bubble)
                continue;

            if (bubble && bubble->active)
            {

                bubble->SetVelocityY(bubble->m_velocity.y);

            //destroy bubbles on contact with player's cane
            
                if (physics.collisions.CheckCollisions(bubble, SwankyBubbles::attackBox))
                {
                    SwankyBubbles::PopBubble(bubble->currentFrame); 
                    RemoveFromVector(SwankyBubbles::bubbles, bubble);
                } 

            //or bottom of viewport

                else if (bubble->position.y >= 500)
                    RemoveFromVector(SwankyBubbles::bubbles, bubble);
            }
        }

    //elves
                
        for (auto &elf : SwankyBubbles::elves)
        {

            if (!elf)
                continue;

            if (elf && elf->active)
            {

                elf->SetVelocityX(elf->m_velocity.x); 
                elf->Animate("fly");

                
                if (player && static_cast<SwankyVelvet*>(player)->canHit)
                    player->SetTint(physics.collisions.CheckCollisions(elf, SwankyBubbles::hitBox) ? 
                        glm::vec3(1.0f, 0.0f, 0.0f) : 
                        glm::vec3(1.0f, 1.0f, 1.0f)
                    );

            //game over
            
                if (
                    physics.collisions.CheckCollisions(elf, SwankyBubbles::hitBox) && 
                    SwankyBubbles::damageBuffer &&
                    static_cast<SwankyVelvet*>(player)->canHit
                )
                {
                    SwankyBubbles::damageBuffer = false;
    
                    if (SwankyBubbles::gruntBuffer && SwankyBubbles::health > 0)
                    {
                        SwankyBubbles::gruntBuffer = false;
                        System::Application::audio.play("barf");
                    }

                    System::Application::time.delayedCall(500, [&](){ 
                        SwankyBubbles::damageBuffer = true; 
                        SwankyBubbles::gruntBuffer = true;
                    }); 

                    SwankyBubbles::health--;

                }
                    
            //destroy when out of viewport

            else if (elf->position.x >= 750)
                RemoveFromVector(SwankyBubbles::elves, elf);
            }
            
        }

    //destroy fireball

        for (auto &fireball : SwankyBubbles::fireballs)
        {

            if (fireball != nullptr && fireball->active)
            {

                fireball->SetVelocityX(fireball->m_velocity.x); 

                //destroy elf with FIRE!

                for (auto &elf : SwankyBubbles::elves)
                {

                    if (!elf)
                        continue;

                    if (elf == nullptr && !elf->active && fireball == nullptr && !fireball->active)
                        return;
                        
                    if (elf->active && fireball->active && physics.collisions.CheckCollisions(fireball, elf))
                    {

                        RemoveFromVector(SwankyBubbles::fireballs, fireball); 

                        elf->SetTint(glm::vec3(1.0f, 0.0f, 0.0f));
                            
                        System::Application::time.delayedCall(250, [&](){ 

                            elf->SetTint(glm::vec3(0.0f, 0.0f, 0.0f));

                            System::Application::time.delayedCall(250, [&](){ 

                                if (elf->active)
                                {
                                    RemoveFromVector(SwankyBubbles::elves, elf);
                                    SwankyBubbles::destroyElfCallbackSound("fire_sound", false);
                                } 
                            });
                        });  

                    }
                }

                if (fireball && fireball->active && fireball->position.x <= 0 || fireball->position.x >= 750)
                    RemoveFromVector(SwankyBubbles::fireballs, fireball);
            }
                                

        //pop bubbles with FIRE!

            for (auto &bubble : SwankyBubbles::bubbles)
            {
                if (!bubble)
                    continue;
                
                if (bubble->active && physics.collisions.CheckCollisions(fireball, bubble))
                {

                    SwankyBubbles::PopBubble(bubble->currentFrame);    
                    RemoveFromVector(SwankyBubbles::bubbles, bubble);

                    if (fireball != nullptr && fireball->active)
                        RemoveFromVector(SwankyBubbles::fireballs, fireball); 
                
                }
            }

        }
        
    }

    //----------game is over, restart

    else 
    {

        for (auto &bubble : SwankyBubbles::bubbles)
        {
            if (!bubble && bubble->alive)
                continue;
            
            bubble->SetAlpha(0.0f);
        }

        for (auto &elf : SwankyBubbles::elves)
        {
            if (!elf && elf->alive)
                continue;

            elf->SetAlpha(0.0f);
        }

        for (auto &fb : SwankyBubbles::fireballs)
        {
            if (!fb && fb->alive)
                continue;

            fb->SetAlpha(0.0f);
        }

        if (player)
            player->SetAlpha(0.0f);
           
        //game over text

        if (SwankyBubbles::gameOverText->active)
        {

            SwankyBubbles::gameOverText->SetAlpha(1.0f);
            SwankyBubbles::gameOverText->renderable = true;

            player->renderable = false;

            if (!SwankyBubbles::canRestart)
                System::Application::time.delayedCall(2000, [&](){ SwankyBubbles::canRestart = true; }); 
        }

        if (SwankyBubbles::canRestart)
        {

            System::Application::audio.play("error"); 

            if (     
                GetScene()->GetContext().inputs.SHIFT || 
                GetScene()->GetContext().inputs.SPACE ||
                GetScene()->GetContext().inputs.LEFT_CLICK 
            )
            {
                ShutDown(); 
                SwankyBubbles::restart = true; 
            }
            
        }

    }

}


//-----------------------------------


void Game::Init()
{

    //health

    SwankyBubbles::hearts = {

        CreateUI("heart", 500.0f, 20.0f),
        CreateUI("heart", 550.0f, 20.0f),
        CreateUI("heart", 600.0f, 20.0f)
    };

    
    //score
    
    SwankyBubbles::scoreText = {

        CreateUI("pixel_text", 20.0f, 10.0f, 10), //S
        CreateUI("pixel_text", 30.0f, 10.0f, 11), //C
        CreateUI("pixel_text", 40.0f, 10.0f, 12), //O
        CreateUI("pixel_text", 50.0f, 9.0f, 13),  //R
        CreateUI("pixel_text", 60.0f, 10.0f, 14), //E
        CreateUI("pixel_text", 70.0f, 10.0f, 15), //:
        CreateUI("pixel_text", 80.0f, 10.0f, 0), //digit 1, default 0
        CreateUI("pixel_text", 90.0f, 10.0f, 0), //digit 2, default invisible
        CreateUI("pixel_text", 100.0f, 10.0f, 0) //digit 3, default invisible

    };

    //virtual controls - mobile only

    if (System::Application::isMobile)
    {
        
        GetScene()->GetContext().inputs.virtual_buttons = {

            CreateUI("arrow_button", 30.0f, 200.0f), //left
            CreateUI("arrow_button", 125.0f, 200.0f), //right
            CreateUI("arrow_button", 80.0f, 260.0f), //up
            CreateUI("arrow_button", 80.0f, 140.0f), //down
            CreateUI("circle_button", 720.0f, 200.0f) //action

        }; 
            
        GetScene()->GetContext().inputs.virtual_buttons[1]->SetRotation(180);
        GetScene()->GetContext().inputs.virtual_buttons[2]->SetRotation(90);
        GetScene()->GetContext().inputs.virtual_buttons[3]->SetRotation(270);

        for (auto &button : GetScene()->GetContext().inputs.virtual_buttons)
            if (button)
                button->SetScale(4.0f);
    }

}


//----------------------------------


void Game::Run()
{

    System::Application::time.exitFlag = false; 

    SwankyBubbles::started = false;
    SwankyBubbles::restart = false;
    SwankyBubbles::health = 3;
    SwankyBubbles::game_over = false;   

//background

    SwankyBubbles::background = CreateSprite("cave", 300.0f, 145.0f);
    SwankyBubbles::background->SetScale(3.45f, 2.45f);

//game over text

    SwankyBubbles::gameOverText = CreateSprite("game_over", 360.0f, 200.0f);
    SwankyBubbles::gameOverText->SetScale(5.0f);
    SwankyBubbles::gameOverText->SetAlpha(0.0f);


//spawn entities on interval


    //---------- bubble

    System::Application::time.setInterval(1200, [&](){ 

        if (
            !SwankyBubbles::started || 
            SwankyBubbles::game_over ||
            SwankyBubbles::bubbles.size() > 10
        )
            return;

        Sprite* bubble = CreateSprite("bubbles", System::Utils::intBetween(0, 700), 0.0f, System::Utils::intBetween(0, 9)); 

        bubble->SetVelocityY(System::Utils::intBetween(1.0f, 5.0f));   

        SwankyBubbles::bubbles.push_back(bubble);

    }, SwankyBubbles::bubble_lock);


    //--------- elves

    System::Application::time.setInterval(2000, [&](){ 

        if (
            !SwankyBubbles::started || 
            SwankyBubbles::game_over ||
            SwankyBubbles::elves.size() > 3
        )
            return;

        Sprite* elf = CreateSprite("elf", 0.0f, System::Utils::intBetween(0.0f, 350.0f));

        elf->SetVelocityX(System::Utils::intBetween(2, 6));   

        SwankyBubbles::elves.push_back(elf);
            
    }, SwankyBubbles::elf_lock);


    //-------- shoot fire balls

    std::thread([&](){
 
        while(!SwankyBubbles::game_over)
        {

            if (
                GetScene()->GetContext().inputs.SPACE && 
                SwankyBubbles::canFire &&
                static_cast<SwankyVelvet*>(player)->canFire
            )
            {

                SwankyBubbles::canFire = false;
                System::Application::time.delayedCall(250, [&](){ 

                    SwankyBubbles::canFire = true; 

                    Sprite* fb = CreateSprite("fire", player->flipX ? 
                        player->position.x - 20 : 
                        player->position.x + 110, 
                        player->position.y + 35
                    ); 

                    fb->SetScale(2.0f);  

                    fb->m_velocity.x = player->flipX ? -5.0f : 5.0f;

                    SwankyBubbles::fireballs.push_back(fb);
                });

            }
        }
    }).detach();

//background splash menu

    if (!SwankyBubbles::started)
    {

        SwankyBubbles::menu = CreateSprite("menu", 250.0f, 150.0f);

        SwankyBubbles::menu->SetScale(3.0f);

        //autoplay on native build
         
        #ifndef __EMSCRIPTEN__
            if (!System::Application::audio.musicPlaying)
                System::Application::audio.play("cave_music", true);
        #endif

    } 

    //player 

    player = CreatePlayer<SwankyVelvet>("swanky_bubble", 250.0f, 100.0f); 
    player->renderable = false;

    SwankyBubbles::attackBox = CreateRect(0.0f, 0.0f, 50.0f, 10.0f);  
    SwankyBubbles::hitBox = CreateRect(0.0f, 0.0f, 100.0f, 100.0f); 

}


//-----------------------------------


void Game::ShutDown()
{

    System::Application::time.exitFlag = System::Application::time.exitFlag.exchange(1);

    SwankyBubbles::health = 3;
    SwankyBubbles::game_over = false;

    SwankyBubbles::started = false;
    SwankyBubbles::canRestart = false;
    SwankyBubbles::restart = false;

    SwankyBubbles::currentScore = 0;
    SwankyBubbles::digit_1 = 0;
    SwankyBubbles::digit_2 = 0;
    SwankyBubbles::digit_3 = 0;

    SwankyBubbles::bubbles.clear();
    SwankyBubbles::elves.clear();
    SwankyBubbles::fireballs.clear();
    SwankyBubbles::scoreText.clear();

    System::Log::logMulti("Game over. Score is: ", SwankyBubbles::currentScore);

}






