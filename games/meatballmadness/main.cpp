
#include "../../build/include/app.h"
#include "./main.h"
#include "./player.h"

#ifdef _WIN32
	#include <windows.h>
#endif

#ifndef __EMSCRIPTEN__

    #include "./resources/assets/raw/audio/ring.h"

    #include "./resources/assets/raw/audio/error.h"
    #include "./resources/assets/raw/audio/fire_sound.h"
    #include "./resources/assets/raw/audio/barf.h"
    #include "./resources/assets/raw/audio/music.h"

    #include "./resources/assets/raw/image/background.h"
    #include "./resources/assets/raw/image/meatball.h"
    #include "./resources/assets/raw/image/patron.h"
    #include "./resources/assets/raw/image/waiter.h"
    #include "./resources/assets/raw/image/chef.h"  
    #include "./resources/assets/raw/image/logo.h" 

#endif

#include "./resources/assets/raw/anims/waiter.h"
#include "./resources/assets/raw/anims/chef.h"
#include "./resources/assets/raw/anims/patron.h"

#include "./resources/assets/atlas/waiter.h"
#include "./resources/assets/atlas/meatball.h"
#include "./resources/assets/atlas/chef.h"
#include "./resources/assets/atlas/patron.h"

static bool chefMoveLeft = true;
static bool chefMoveRight = false;
static bool game_over = false;
static bool started = false;
static bool paused = false;
static bool canThrow = true;
static bool canRestart = false;

//--------------------------------


void MeatballMadness::ThrowMeatball()
{

    auto meatball = System::Game::CreateSprite("meatball", 960.0f, 530.0f, 0, 2.0f);

    meatball->SetData("platter position", System::Utils::intBetween(0, 50));

    meatball->bodies.push_back({ Physics::CreateDynamicBody("box", meatball->m_position.x, meatball->m_position.y, 1.0f, 1.0f, false, 3, System::Utils::floatBetween(1.0f, 10.0f) ), { 0.0f, 5.0f } });
    meatball->bodies[0].first->SetFixedRotation(true);

    const float speedX = System::Utils::intBetween(0, 10) > 5 ? 
                        System::Utils::floatBetween(-0.01f, -0.25) : 
                        System::Utils::floatBetween(-0.1f, -0.2),

                speedY = System::Utils::intBetween(0, 10) > 5 ? 
                        System::Utils::floatBetween(-0.2f, 0.32f) : 
                        System::Utils::floatBetween(0.0f, 0.3f);

    meatball->SetImpulse(speedX, speedY);

    this->meatballs.push_back(meatball); 

    System::Audio::play("fire_sound");

}


//---------------------------------


void MeatballMadness::MoveChef()
{ 

    if (this->chef->m_position.x <= 1145 && canThrow)
    {
        chefMoveLeft = true;
        chefMoveRight = false;
        canThrow = false;

        this->ThrowMeatball();
    }

    if (this->chef->m_position.x >= 1270)
    {
        chefMoveLeft = false;
        chefMoveRight = true;
        canThrow = true;
    }

    if (chefMoveLeft)
    {
        this->chef->SetVelocityX(1.5f);
        this->chef->SetFlipX(true);
        this->chef->Animate("move with meatball", false, 3);
    }
    
    if (chefMoveRight)
    {
        this->chef->SetVelocityX(-1.5f);
        this->chef->SetFlipX(false);
        this->chef->Animate("move", false, 3);
    }

}


//-----------------------------



void MeatballMadness::Update()
{ 
    //set control states

    if (System::Application::isMobile)
        for (int i = 0; i < System::Game::virtual_buttons.size(); i++)
        {
            System::Game::virtual_buttons[i]->SetScale(4.0f);
            
            switch (i) {
                case 0: this->context.inputs->m_left = System::Game::UIListenForInput(0); break;
                case 1: this->context.inputs->m_right = System::Game::UIListenForInput(1); break;
                case 2: this->context.inputs->m_up = System::Game::UIListenForInput(2); break;
            }
        }

    //platter hitbox

    if (this->playerHitBox)
        this->playerHitBox->SetTransform(b2Vec2(this->player->m_flipX ? this->player->m_position.x : this->player->m_position.x + 60, this->player->m_position.y), 0); 

    //game over

    if (this->fails >= 3 && !game_over) 
        this->GameOver();
    
    else if (paused && (this->context.inputs->isDown && canRestart)) 
    {
            
        this->gameOverText->SetAlpha(0.0f);

        game_over = false;
        paused = false;
        canRestart = false;
        
        entity_behaviors::Behavior::GetBehavior<entity_behaviors::Waiter>("Waiter", this->behaviors)->canMove = true;
        
    }

    else if (!started)
    {

        if (this->context.inputs->isDown)
        {

            #ifdef __EMSCRIPTEN__
                if (!System::Audio::musicPlaying)
                    System::Audio::play("music", true);
            #endif

            entity_behaviors::Behavior::GetBehavior<entity_behaviors::Waiter>("Waiter", this->behaviors)->canMove = true;
            
            started = true;
        }

        return;
    }

    else if (!game_over)
    {

        this->menuText->SetAlpha(0.0f);
        this->creditsText->SetAlpha(0.0f);

        if (this->meatballs.size())
        {
            
            auto it = meatballs.begin();
            
            for (; it != meatballs.end(); ++it)
            {

                auto meatball = *it;

                if (!meatball.get())
                    continue;

                //score point

                else if (meatball->m_alive && meatball->m_position.x <= 290.0f) {

                    meatball->m_alive = false;

                    System::Game::DestroyEntity(meatball);

                    score += 1;

                    System::Audio::play("barf");
                    System::Application::game->time->delayedCall(250, [](){ System::Audio::play("ring"); });

                }

                //fail

                else if (meatball->m_position.y >= 830) {

                    meatball->SetFrame(1);
                    meatball->SetRotation(0);

                    if (meatball->m_active) {

                        meatball->bodies[0].first->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
                        meatball->m_active = false;
                        meatball->RemoveBodies();   

                        this->fails += 1; 
                    }

                }

            //overlap hitbox

                else if (
                    meatball->m_active &&
                    b2TestOverlap(meatball->bodies[0].first->GetFixtureList()->GetAABB(0), this->playerHitBox->GetFixtureList()->GetAABB(0))
                )
                {
                    meatball->SetRotation(0);
                    meatball->SetFrame(1);
                    meatball->RemoveBodies();
                    meatball->m_active = false;
                }

                else if (!meatball->m_active) {

                    //meatball on platter

                    const int meatballPosition = meatball->GetData<int>("platter position");
                    
                    meatball->SetPosition(
                        this->player->m_flipX ? 
                            this->playerHitBox->GetTransform().p.x - meatballPosition : 
                            this->playerHitBox->GetTransform().p.x + meatballPosition,
                        this->playerHitBox->GetTransform().p.y - 25
                    );
                }

                 else   
                    meatball->m_rotation -= System::Utils::floatBetween(5.0f, 15.0f);
                
            }
        }

        this->MoveChef();

        //set depth  

        for (const auto& entity : System::Application::game->entities) {
            
            if (this->scoreText != nullptr) {
                this->scoreText->content = "SCORE: " + std::to_string(this->score);
                this->scoreText->SetDepth(entity->m_depth + 1);
            }
            
            for (const auto& button : System::Game::virtual_buttons)
                if (button)
                    button->SetDepth(entity->m_depth + 1);

            this->gameOverText->SetDepth(entity->m_depth + 1);
        }
    }

       
}



//-----------------------------


void MeatballMadness::Preload()
{

    System::Resources::Manager::LoadFrames("waiter", Assets::Spritesheets::waiter);
    System::Resources::Manager::LoadFrames("meatball", Assets::Spritesheets::meatball);
    System::Resources::Manager::LoadFrames("chef", Assets::Spritesheets::chef);
    System::Resources::Manager::LoadFrames("patron", Assets::Spritesheets::patron);

    #ifdef __EMSCRIPTEN__

        System::Resources::Manager::LoadFile("ring", "assets/ring.flac");
        System::Resources::Manager::LoadFile("error", "assets/error.flac" );
        System::Resources::Manager::LoadFile("barf", "assets/barf.flac");
        System::Resources::Manager::LoadFile("fire_sound", "assets/fire_sound.flac");
        System::Resources::Manager::LoadFile("music", "assets/music.flac");

        System::Resources::Manager::LoadFile("background", "assets/background.png");
        System::Resources::Manager::LoadFile("waiter", "assets/waiter.png");
        System::Resources::Manager::LoadFile("patron", "assets/patron.png");
        System::Resources::Manager::LoadFile("chef", "assets/chef.png");
        System::Resources::Manager::LoadFile("meatball", "assets/meatball.png");
        System::Resources::Manager::LoadFile("circle_button", "assets/action_button.png");
        System::Resources::Manager::LoadFile("arrow_button", "assets/direction_button.png");

    #else

        System::Resources::Manager::LoadRawImage("logo", Assets::Images::logo, 70, 70, 4);
        System::Resources::Manager::LoadRawImage("background", Assets::Images::background, 600, 400, 3);
        System::Resources::Manager::LoadRawImage("meatball", Assets::Images::meatball, 30, 16, 4);
        System::Resources::Manager::LoadRawImage("patron", Assets::Images::patron, 315, 101, 4);
        System::Resources::Manager::LoadRawImage("waiter", Assets::Images::waiter, 210, 70, 4);
        System::Resources::Manager::LoadRawImage("chef", Assets::Images::chef, 62, 756, 4);

        System::Resources::Manager::LoadRawAudio("ring", Assets::Audio::ring, 28443);
        System::Resources::Manager::LoadRawAudio("error", Assets::Audio::error, 32533);
        System::Resources::Manager::LoadRawAudio("fire_sound", Assets::Audio::fire_sound, 13332);
        System::Resources::Manager::LoadRawAudio("music", Assets::Audio::music, 3796818);
        System::Resources::Manager::LoadRawAudio("barf", Assets::Audio::barf, 51329);


    #endif


    System::Resources::Manager::LoadAnims("waiter", Assets::Anims::waiter); 
    System::Resources::Manager::LoadAnims("chef", Assets::Anims::chef);
    System::Resources::Manager::LoadAnims("patron", Assets::Anims::patron);

}


//-----------------------------


void MeatballMadness::Run() 
{

    //splash image

    #ifndef __EMSCRIPTEN__

        GLFWimage image; 
        
        image.width = 70;
        image.height = 70;
        image.pixels = reinterpret_cast<unsigned char*>(const_cast<char*>(System::Resources::Manager::GetRawData("logo")));
    
        glfwSetWindowIcon(System::Window::s_instance, 1, &image);

    #endif

    this->Reset();

    //environment bounds

    Physics::CreateStaticBody(0.0f, 0.0f, 1500.0f, 0.0f, 3);
    Physics::CreateStaticBody(0.0f, 850.0f, 1500.0f, 10.0f, 3);
    Physics::CreateStaticBody(250.0f, 850.0f, 10.0f, 850.0f, 3);
    Physics::CreateStaticBody(1020.0f, 0.0f, 10.0f, 850.0f, 3);

    //sprites

    auto background = System::Game::CreateSprite("background", 450.0f, 280.0f);
    background->SetScale(2.5f, 2.42f);
    background->SetDepth(0);

    auto patron = System::Game::CreateSprite("patron", 165.0f, 640.0f);               
    patron->SetScale(3.0f);
    patron->SetAnimation("idle");
    patron->SetDepth(1);

    this->chef = System::Game::CreateSprite("chef", 1250.0f, 640.0f);
    this->chef->SetScale(2.0f);
    this->chef->SetFrame(2);
    this->chef->SetDepth(1);

    //player

    this->player = System::Game::CreateSprite("waiter", 450.0f, 760.0f, 1, 2.5);
    this->player->SetDepth(1);

    this->player->bodies.push_back({ Physics::CreateDynamicBody("box", 450.0f, 760.0f, 10.0f, 35.0f, false, 3, 3.5), { 30.0f, 70.0f } });
    this->player->bodies[0].first->SetFixedRotation(true);

    System::Game::CreateBehavior<entity_behaviors::Waiter>(this->player, this);
    this->playerHitBox = Physics::CreateDynamicBody("box", 0.0f, 0.0f, 40.0f, 10.0f, true, 1);  

    //UI

    this->scoreText = System::Game::CreateText("", 30.0f, -10.0f);
    this->scoreText->SetScale(3.0f);
    this->scoreText->SetTint({1.0f, 1.0f, 0.0f});

    this->gameOverText = System::Game::CreateText("GAME OVER", 500.0f, 300.0f);
    this->gameOverText->SetScale(5.0f);
    this->gameOverText->SetTint({1.0f, 0.0f, 0.0f});
    this->gameOverText->SetAlpha(0.0f);

    this->menuText = System::Game::CreateText("MEATBALL MADNESS", 290.0f, 300.0f);
    this->menuText->SetScale(7.0f);
    this->menuText->SetTint({1.0f, 0.5f, 0.0f});

    this->creditsText = System::Game::CreateText("DEVELOPED BY PASTABOSS ENTERPRISE", 360.0f, 495.0f);
    this->creditsText->SetScale(3.0f);
    this->creditsText->SetTint({1.0f, 0.5f, 0.0f});

    //virtual on screen inputs (mobile only)

    if (System::Application::isMobile)
    {

        System::Game::virtual_buttons = {

            System::Game::CreateUI("arrow_button", 130.0f, 500.0f), //left
            System::Game::CreateUI("arrow_button", 250.0f, 500.0f), //right
            System::Game::CreateUI("circle_button", 1400.0f, 500.0f) //action

        };
            
        System::Game::virtual_buttons[1]->SetRotation(180);

    }

    #ifndef __EMSCRIPTEN__
        if (!System::Audio::musicPlaying)
           System::Audio::play("music", true);
    #endif

}

//---------------------------------


void MeatballMadness::GameOver()
{

    game_over = true;
    paused = true;

    entity_behaviors::Behavior::GetBehavior<entity_behaviors::Waiter>("Waiter", this->behaviors)->canMove = false;
    this->player->StopAnimation();
    this->player->SetFrame(1);
    this->player->SetVelocityX(0);

    this->gameOverText->SetAlpha(1.0f);  

    this->chef->SetFlipX(false);
    this->chef->StopAnimation();
    this->chef->SetFrame(2);

    System::Audio::play("error");
    this->Reset();
    
}


//------------------------------


void MeatballMadness::Reset()
{

    for (auto& meatball : this->meatballs)
        System::Game::DestroyEntity(meatball);

    this->meatballs.clear(); 
    
    this->score = 0;
    this->fails = 0;
    
    chefMoveLeft = true;
    chefMoveRight = false;

    Time::delayedCall(2000, [&]() { canRestart = true; });

}


//--------------------- main entry


#ifdef __EMSCRIPTEN__
 
	//check screen dimensions

	EM_JS(float, getScreenWidth, (), { return window.screen.width; });
	EM_JS(float, getScreenHeight, (), { return window.screen.height; });

	//check if mobile device

	EM_JS(bool, checkMobile, (), { 

		let check = false;

		if (window.screen.width <= 800)
			check = true;

		return check;
	});

	//fetch remote data

	EM_JS(void, fetchData, (), { 

		//TODO: implement fetch data

		console.log('fetching data...');
	}); 


#endif


int main(int argc, char* args[])
{  

    #ifdef _WIN32
        ShowWindow(GetConsoleWindow(), SW_HIDE);
        SetUnhandledExceptionFilter(UnhandledExceptionFilter);
    #endif

    #ifdef __EMSCRIPTEN__  

        System::Application::isMobile = checkMobile();
        fetchData();
        
    #elif _ISMOBILE == 1
        System::Application::isMobile = true;
    #endif

    //main application process

    System::Game game;

    System::Application::name = "MEATBALL MADNESS!";

    System::Game::LoadScene<MeatballMadness>(&game);

    System::Application app { &game }; 

    #ifdef __EMSCRIPTEN__
        emscripten_exit_with_live_runtime();
    #endif

    return 0; 
}


    //Boof RedGibbon