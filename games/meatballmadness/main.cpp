
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



bool chefMoveLeft = true;
bool chefMoveRight = false;
bool game_over = false;
bool started = false;
bool paused = false;
bool canThrow = true;
bool canRestart = false;

//--------------------------------


static void ThrowMeatball()
{

    auto meatball = Game::CreateSprite("meatball", 960.0f, 530.0f, 0, 2.0f);
 
    meatball->SetData("platter position", System::Utils::intBetween(0, 50));

    meatball->bodies.push_back({ System::Application::game->physics->CreateDynamicBody(meatball->m_position.x, meatball->m_position.y, 1.0f, 1.0f, false, 3, 1 ), { 0.0f, 5.0f } });

    float randX = System::Utils::floatBetween(-10.0f, -1000.0f);

    meatball->bodies[0].first->SetLinearVelocity(b2Vec2(randX, System::Utils::floatBetween(10.0f, -10.0f)));  
    meatball->bodies[0].first->SetFixedRotation(true);

    MeatballMadness::meatballs.push_back(meatball); 

    System::Audio::play("fire_sound");

}


//---------------------------------


static void MoveChef()
{ 

    if (MeatballMadness::chef->m_position.x <= 1145 && canThrow)
    {
        chefMoveLeft = true;
        chefMoveRight = false;
        canThrow = false;

        ThrowMeatball();
    }

    if (MeatballMadness::chef->m_position.x >= 1270)
    {
        chefMoveLeft = false;
        chefMoveRight = true;
        canThrow = true;
    }

    if (chefMoveLeft)
    {
        MeatballMadness::chef->SetVelocityX(1.5f);
        MeatballMadness::chef->SetFlipX(true);
        MeatballMadness::chef->Animate("move with meatball");
    }
    
    if (chefMoveRight)
    {
        MeatballMadness::chef->SetVelocityX(-1.5f);
        MeatballMadness::chef->SetFlipX(false);
        MeatballMadness::chef->Animate("move");
    }

}


//-----------------------------



void MeatballMadness::Update(Inputs* inputs, Physics* physics)
{   

    //platter hitbox

    if (playerHitBox)
        playerHitBox->SetTransform(b2Vec2(player->m_flipX ? player->m_position.x : player->m_position.x + 60, player->m_position.y), 0); 

    //game over

    if (fails >= 3 && !game_over) 
        this->GameOver();
    
    else if (paused)
    {

        if (inputs->isDown && canRestart) {
            
            gameOverText->SetAlpha(0.0f);
    
            game_over = false;
            paused = false;
            canRestart = false;
        }
    }

    else if (!started)
    {

        if (inputs->m_SPACE || inputs->m_left_click)
        {

            #ifdef __EMSCRIPTEN__
                if (!System::Audio::musicPlaying)
                    System::Audio::play("music", true);
            #endif
            player->SetData("can move", true);
            started = true;
        }

        return;
    }

    else if (!game_over)
    {

        menuText->SetAlpha(0.0f);
        creditsText->SetAlpha(0.0f);

        if (scoreText != nullptr)
            scoreText->content = "SCORE: " + std::to_string(score);

        if (meatballs.size())
            for (auto &meatball : meatballs)
            {

                if (!meatball.get())
                    return;

                //score point

                else if (meatball->m_alive && meatball->m_position.x <= 290.0f) {

                    meatball->m_alive = false;

                    RemoveFromVector(meatballs, meatball);

                    score += 1;

                    System::Audio::play("barf");
                    System::Application::game->time->delayedCall(250, [](){ System::Audio::play("ring"); });

                }

                //fail

                else if (meatball->m_position.y >= 830) {

                    meatball->SetFrame(1);

                    meatball->SetRotation(0);

                    if(meatball->bodies[0].first) {

                        meatball->bodies[0].first->SetLinearVelocity(b2Vec2(0.0f, 0.0f));

                        meatball->bodies[0].first->SetEnabled(false); 
                        
                        physics->DestroyBody(meatball->bodies[0].first);  
                    }

                    if (meatball->m_active) {

                        meatball->m_active = false;

                        fails += 1; 
                    }
                }

            //overlap hitbox

                else if (
                    meatball->bodies[0].first && 
                    b2TestOverlap(meatball->bodies[0].first->GetFixtureList()->GetAABB(0), playerHitBox->GetFixtureList()->GetAABB(0))
                )
                {

                    meatball->SetRotation(0);
                    meatball->SetFrame(1);

                    if (meatball->bodies[0].first) {     
                        meatball->bodies[0].first->SetEnabled(false); 
                        physics->DestroyBody(meatball->bodies[0].first);
                    }

                    meatball->m_active = false;

                }

            //meatball on platter

                else if (!meatball->m_active)
                {

                    const int meatballPosition = meatball->GetData<int>("platter position");
                    
                    meatball->SetPosition(
                        player->m_flipX ? 
                            playerHitBox->GetTransform().p.x - meatballPosition : 
                            playerHitBox->GetTransform().p.x + meatballPosition,
                        playerHitBox->GetTransform().p.y - 25
                    );
                }

                else
                    meatball->m_rotation -= System::Utils::floatBetween(0.0f, -10.0f);

            }

        MoveChef();

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


void MeatballMadness::Run(Inputs* inputs, Camera* camera, Physics* physics) 
{

    #ifndef __EMSCRIPTEN__

        GLFWimage image; 
        
        image.width = 70;
        image.height = 70;
        image.pixels = reinterpret_cast<unsigned char*>(const_cast<char*>(System::Resources::Manager::GetRawData("logo")));
    
        glfwSetWindowIcon(System::Window::s_instance, 1, &image);

    #endif

    this->Reset();

    //environment bounds

    physics->CreateStaticBody(0.0f, 0.0f, 1500.0f, 0.0f);
    physics->CreateStaticBody(0.0f, 850.0f, 1500.0f, 10.0f);
    physics->CreateStaticBody(250.0f, 850.0f, 10.0f, 850.0f);
    physics->CreateStaticBody(1020.0f, 0.0f, 10.0f, 850.0f);

    auto background = CreateSprite("background", 450.0f, 280.0f);
    background->SetScale(2.5f, 2.42f);

    auto patron = CreateSprite("patron", 165.0f, 640.0f);               
    patron->SetScale(3.0f);
    patron->SetAnimation("idle");

    chef = CreateSprite("chef", 1250.0f, 640.0f);
    chef->SetScale(2.0f);
    chef->SetFrame(2);

    player = CreateSprite("waiter", 450.0f, 760.0f, 0, 2.5);

    player->bodies.push_back({ physics->CreateDynamicBody(450.0f, 760.0f, 10.0f, 35.0f, false, 3, 1.5), { 20.0f, 45.0f } });
    player->bodies[0].first->SetFixedRotation(true);
    player->SetData("can move", false);
    CreateBehavior<Waiter>(player, this);
    playerHitBox = physics->CreateDynamicBody(0.0f, 0.0f, 40.0f, 10.0f, true, 1);

    //UI

    scoreText = CreateText("", 30.0f, -10.0f);
    scoreText->SetScale(3.0f);
    scoreText->SetTint(glm::vec3(1.0f, 1.0f, 0.0f));

    gameOverText = CreateText("GAME OVER", 500.0f, 300.0f);
    gameOverText->SetScale(5.0f);
    gameOverText->SetTint(glm::vec3(1.0f, 0.0f, 0.0f));
    gameOverText->SetAlpha(0.0f);

    menuText = CreateText("MEATBALL MADNESS", 290.0f, 300.0f);
    menuText->SetScale(7.0f);
    menuText->SetTint(glm::vec3(1.0f, 0.5f, 0.0f));

    creditsText = CreateText("DEVELOPED BY PASTABOSS ENTERPRISE", 360.0f, 495.0f);
    creditsText->SetScale(3.0f);
    creditsText->SetTint(glm::vec3(1.0f, 0.5f, 0.0f));

    if (System::Application::isMobile)
    {

        virtual_buttons = {

            CreateUI("arrow_button", 130.0f, 500.0f), //left
            CreateUI("arrow_button", 250.0f, 500.0f), //right
            CreateUI("circle_button", 1400.0f, 500.0f) //action

        };
            
        virtual_buttons[1]->SetRotation(180);

        for (auto &button : virtual_buttons)
            if (button)
                button->SetScale(4.0f);

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

    gameOverText->SetAlpha(1.0f);  

    chef->SetFlipX(false);
    chef->SetFrame(2);

    System::Audio::play("error");
    this->Reset();
    
}


//------------------------------


void MeatballMadness::Reset()
{

    for (auto &meatball : meatballs)
        DestroyEntity(meatball);

    meatballs.clear(); 
    
    score = 0;
    fails = 0;
    
    chefMoveLeft = true;
    chefMoveRight = false;

    System::Application::game->time->delayedCall(2000, [&]() { canRestart = true; });

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


#undef main

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

		MeatballMadness game;
        System::Application app { &game }; 

		#ifdef __EMSCRIPTEN__
			emscripten_exit_with_live_runtime();
		#endif

		return 0; 
	}


    //Boof RedGibbon