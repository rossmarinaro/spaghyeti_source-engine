#include <bitset>

#ifndef __EMSCRIPTEN__
    #include <bits/stdc++.h>
#endif

#include "../../../build/sdk/include/tilemap.h"
#include "../../../build/sdk/include/app.h"

using app = System::Application;


//create layer from csv
void MapManager::CreateLayer (

    const char* data_key,
    const char* texture_key,
    uint32_t mapWidth,
    uint32_t mapHeight,
    uint32_t tileWidth,
    uint32_t tileHeight,
    uint32_t depth
)
{

    const std::vector<std::string> &data = System::Resources::Manager::GetRawTilemapData(data_key);
    
    std::vector<std::string> map;
    std::vector<std::shared_ptr<Sprite>> layer; 

    std::stringstream ss; 
    std::string line; 

    if (!data.size()) {

        #if DEVELOPMENT == 1
            std::cout << "Tilemap: layer data not found.\n";
        #endif

        return;
    }

    //remove commas from array before parsing

    for (int i = 0; i < data.size(); i++) 
        ss << data[i]; 

    while(getline(ss, line, ','))
        map.push_back(line); 

    for (int y = 0; y < mapHeight; ++y)
        for (int x = 0; x < mapWidth; ++x) 
        {

            if (map.begin() + (x + y * mapWidth) == map.end()) {
                #if DEVELOPMENT == 1
                    std::cout << "Tilemap: index overflow, truncating map.\n";
                #endif
                return;
            }

            //convert string to int at index

            int tileType = atoi(map[x + y * mapWidth].c_str());

            //skip if no tile

            if (tileType != -1) 
            {

                bool flipX = false, 
                     flipY = false,
                     diag = false;

                //number larger than total tiles indicates flip or rotation
                //convert to binary to extract bit flags

                if ((tileType < -1) || (tileType > mapWidth * mapHeight)) 
                { 
                    //flags: 1=flipX, 2=flipY, 3=diagonal

                    std::bitset<32> bin(tileType);

                    std::string flags = bin.to_string().substr(0, 3),
                                bin_reset = bin.to_string();

                    bin_reset[0] = '0'; 
                    bin_reset[1] = '0';
                    bin_reset[2] = '0';
                   
                    tileType = System::Utils::BinToDec(atoi(bin_reset.c_str()));       
                                
                    flipX = static_cast<std::string>(flags).substr(0, 1) == "1";
                    flipY = static_cast<std::string>(flags).substr(1, 1) == "1";
                    diag = static_cast<std::string>(flags).substr(2, 1) == "1";

                }

                //create tilesprite entity

                auto tile = System::Game::CreateTileSprite(texture_key, x * tileWidth, y * tileHeight, tileType); 
                
                tile->name = (std::string)data_key;
                tile->SetDepth(depth); 
                tile->SetFlip(flipX, flipY);  

                if (diag)
                    tile->SetRotation(90);  

                //add layer to stack

                layer.push_back(tile);
            }

        }

    app::game->maps->layers.push_back(layer);

    #if DEVELOPMENT == 1
        std::cout << "Tilemap: Initialized layer: " + (std::string)data_key + "\n";
    #endif

}


//------------------------------------- remove layer


void MapManager::RemoveLayer(const std::string& key) 
{

    for (auto it = app::game->maps->layers.begin(); it != app::game->maps->layers.end(); ++it) {

        auto layer = *it;

        layer.erase(
            std::remove_if(layer.begin(), layer.end(), [&](auto t) { 
                return strcmp(t->type, "tile") == 0 && t->name == key; 
            }), layer.end());
    }

    app::game->currentScene->entities.erase(
        std::remove_if(app::game->currentScene->entities.begin(), app::game->currentScene->entities.end(), [&](auto t) { 
            return strcmp(t->type, "tile") == 0 && t->name == key; }), 
                app::game->currentScene->entities.end());

    #if DEVELOPMENT == 1
        std::cout << "Tilemap: layer " + key + " cleared.\n";
    #endif

}


//------------------------------------- clear all layers


void MapManager::ClearMap() 
{

    app::game->maps->layers.clear();

    app::game->currentScene->entities.erase(
        std::remove_if(app::game->currentScene->entities.begin(), app::game->currentScene->entities.end(), [](auto t) { 
            return strcmp(t->type, "tile") == 0; }), 
                app::game->currentScene->entities.end());

    #if DEVELOPMENT == 1
        std::cout << "Tilemap: layers cleared.\n";
    #endif

}


